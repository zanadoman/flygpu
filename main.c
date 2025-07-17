#include <SDL3/SDL.h>

#define PI   3.14159f
#define FOV  (60.0f / 180.0f * PI)
#define NEAR 0.1f
#define FAR  100.0f

typedef struct {
    float x;
    float y;
    float z;
    float angle;
    float width;
    float height;
} Quad;

typedef struct Renderer Renderer;

static Renderer * create_renderer(SDL_Window *window);
static bool       render_quads(Renderer *renderer, const Quad *begin, const Quad *end);
static void       destroy_renderer(Renderer *renderer);

Sint32 main(void)
{
    SDL_Window *window   = NULL;
    Renderer   *renderer = NULL;
    Quad        quads[2];
    Uint64      tick     = 0;
    bool        running  = true;
    SDL_Event   event;

    SDL_memset( quads, 0, sizeof(quads));
    SDL_memset(&event, 0, sizeof(event));

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("SDL_GPU", 800, 600, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return 1;
    }

    renderer = create_renderer(window);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s\n", SDL_GetError());
        return 1;
    }

    quads[0].z      = -1.0f;
    quads[0].width  =  0.1f;
    quads[0].height =  0.1f;

    quads[1].x      =  0.5f;
    quads[1].y      =  0.5f;
    quads[1].z      = -5.0f;
    quads[1].angle  = -45.0f / 180.0f * PI;
    quads[1].width  =  1.2f;
    quads[1].height =  1.0f;

    tick = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if ((event.type) == SDL_EVENT_QUIT) running = false;
        }

        if (!render_quads(renderer, quads, quads + sizeof(quads) / sizeof(*quads))) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
            return 1;
        }

        SDL_Log("Frame time: %lu ms\n", SDL_GetTicks() - tick);
        tick = SDL_GetTicks();
    }

    destroy_renderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

struct Renderer {
    SDL_Window              *window;
    SDL_GPUDevice           *device;
    SDL_GPUGraphicsPipeline *quad_pipeline;
};

static SDL_GPUShader           * load_shader(SDL_GPUDevice *device, const char *path, SDL_GPUShaderStage stage, Uint32 ubos);
static SDL_GPUGraphicsPipeline * create_quad_pipeline(SDL_GPUDevice *device, SDL_Window *window);

typedef float Matrix4[4 * 4];

static bool set_projection(SDL_Window *window, Matrix4 projection);
static void set_transformation(const Quad* quad, Matrix4 transformation);
static void multiply_matrices(Matrix4 a, Matrix4 b, Matrix4 c);

Renderer * create_renderer(SDL_Window *window)
{
    Renderer *renderer = SDL_calloc(1, sizeof(*renderer));

    if (!renderer) return NULL;

    renderer->window = window;

    renderer->device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, "vulkan");
    if (!renderer->device) return NULL;

    if (!SDL_ClaimWindowForGPUDevice(renderer->device, renderer->window)) {
        SDL_DestroyGPUDevice(renderer->device);
        return NULL;
    }

    if (!SDL_SetGPUSwapchainParameters(renderer->device, renderer->window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_IMMEDIATE)) {
        SDL_ReleaseWindowFromGPUDevice(renderer->device, renderer->window);
        SDL_DestroyGPUDevice(renderer->device);
        return NULL;
    }

    renderer->quad_pipeline = create_quad_pipeline(renderer->device, renderer->window);
    if (!renderer->quad_pipeline) {
        SDL_ReleaseWindowFromGPUDevice(renderer->device, renderer->window);
        SDL_DestroyGPUDevice(renderer->device);
        return NULL;
    }

    return renderer;
}

bool render_quads(Renderer *renderer, const Quad *begin, const Quad *end)
{
    Matrix4                 projection;
    SDL_GPUCommandBuffer   *command_buffer = NULL;
    SDL_GPUColorTargetInfo  target;
    SDL_GPURenderPass      *render_pass    = NULL;
    const Quad             *quad           = NULL;
    Matrix4                 transformation;
    Matrix4                 mvp;

    if (!set_projection(renderer->window, projection)) return false;
    SDL_memset(&target, 0, sizeof(target));

    command_buffer = SDL_AcquireGPUCommandBuffer(renderer->device);
    if (!command_buffer) return false;

    target.clear_color.r = 0.25f;
    target.clear_color.g = 0.25f;
    target.clear_color.b = 0.25f;
    target.load_op       = SDL_GPU_LOADOP_CLEAR;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, renderer->window, &target.texture, NULL, NULL)) {
        SDL_CancelGPUCommandBuffer(command_buffer);
        return false;
    }

    if (target.texture) {
        render_pass = SDL_BeginGPURenderPass(command_buffer, &target, 1, NULL);
        SDL_BindGPUGraphicsPipeline(render_pass, renderer->quad_pipeline);
        for (quad = begin; quad != end; ++quad) {
            set_transformation(quad, transformation);
            multiply_matrices(projection, transformation, mvp),
            SDL_PushGPUVertexUniformData(command_buffer, 0, &mvp, sizeof(mvp));
            SDL_DrawGPUPrimitives(render_pass, 6, 1, 0, 0);
        }
        SDL_EndGPURenderPass(render_pass);
    }

    return SDL_SubmitGPUCommandBuffer(command_buffer);
}

void destroy_renderer(Renderer *renderer)
{
    SDL_ReleaseGPUGraphicsPipeline(renderer->device, renderer->quad_pipeline);
    SDL_ReleaseWindowFromGPUDevice(renderer->device, renderer->window);
    SDL_DestroyGPUDevice(renderer->device);
    SDL_free(renderer);
}

SDL_GPUShader * load_shader(SDL_GPUDevice *device, const char *path, SDL_GPUShaderStage stage, Uint32 ubos)
{
    SDL_GPUShaderCreateInfo  info;
    void                    *code   = NULL;
    SDL_GPUShader           *shader = NULL;

    SDL_memset(&info, 0, sizeof(info));

    code = SDL_LoadFile(path, &info.code_size);
    if (!code) return NULL;

    info.code                = code;
    info.entrypoint          = "main";
    info.format              = SDL_GPU_SHADERFORMAT_SPIRV;
    info.stage               = stage;
    info.num_uniform_buffers = ubos;

    shader = SDL_CreateGPUShader(device, &info);

    SDL_free(code);
    return shader;
}

SDL_GPUGraphicsPipeline * create_quad_pipeline(SDL_GPUDevice *device, SDL_Window *window)
{
    SDL_GPUColorTargetDescription      target;
    SDL_GPUGraphicsPipelineCreateInfo  info;
    SDL_GPUGraphicsPipeline           *pipeline = NULL;

    SDL_memset(&target, 0, sizeof(target));
    SDL_memset(&info,   0, sizeof(info));

    target.format = SDL_GetGPUSwapchainTextureFormat(device, window);

    info.vertex_shader = load_shader(device, "./quad.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 1);
    if (!info.vertex_shader) return NULL;

    info.fragment_shader = load_shader(device, "./quad.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 0);
    if (!info.fragment_shader) return NULL;

    info.target_info.color_target_descriptions = &target;
    info.target_info.num_color_targets         = 1;

    pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);

    SDL_ReleaseGPUShader(device, info.fragment_shader);
    SDL_ReleaseGPUShader(device, info.vertex_shader);
    return pipeline;
}

bool set_projection(SDL_Window *window, Matrix4 projection)
{
    Sint32 width  = 0;
    Sint32 height = 0;
    float  focal  = 1.0f / SDL_tanf(FOV / 2.0f);
    if (!SDL_GetWindowSize(window, &width, &height)) return false;
    projection[0]  =  focal / ((float)width / (float)height);
    projection[1]  =  0.0f;
    projection[2]  =  0.0f;
    projection[3]  =  0.0f;
    projection[4]  =  0.0f;
    projection[5]  =  focal;
    projection[6]  =  0.0f;
    projection[7]  =  0.0f;
    projection[8]  =  0.0f;
    projection[9]  =  0.0f;
    projection[10] =  (FAR + NEAR) / (NEAR - FAR);
    projection[11] = -1.0f;
    projection[12] =  0.0f;
    projection[13] =  0.0f;
    projection[14] =  2.0f * FAR * NEAR / (NEAR - FAR);
    projection[15] =  0.0f;
    return true;
}

void set_transformation(const Quad *quad, Matrix4 transformation)
{
    float cosine       =  SDL_cosf(quad->angle);
    float sine         =  SDL_sinf(quad->angle);
    transformation[0]  =  cosine * quad->width;
    transformation[1]  =  sine * quad->width;
    transformation[2]  =  0.0f;
    transformation[3]  =  0.0f;
    transformation[4]  = -sine * quad->height;
    transformation[5]  =  cosine * quad->height;
    transformation[6]  =  0.0f;
    transformation[7]  =  0.0f;
    transformation[8]  =  0.0f;
    transformation[9]  =  0.0f;
    transformation[10] =  1.0f;
    transformation[11] =  0.0f;
    transformation[12] =  quad->x;
    transformation[13] =  quad->y;
    transformation[14] =  quad->z;
    transformation[15] =  1.0f;
}

void multiply_matrices(Matrix4 a, Matrix4 b, Matrix4 c)
{
    Sint32 i, j, k;
    for (i = 0; i != 4; ++i) {
        for (j = 0; j != 4; ++j) {
            c[j * 4 + i] = 0.0f;
            for (k = 0; k != 4; ++k) {
                c[j * 4 + i] += a[k * 4 + i] * b[j * 4 + k];
            }
        }
    }
}
