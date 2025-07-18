.PHONY : debug clean

flygpu : main.s quad.vert.spv quad.frag.spv
	@clang $< -lSDL3 -o $@

main.s : main.c
	@clang $< @compile_flags.txt -S

quad.vert.spv : quad.vert
	@glslang -V $< -o $@

quad.frag.spv : quad.frag
	@glslang -V $< -o $@

debug : flygpu
	@./flygpu

clean :
	@rm flygpu main.s quad.vert.spv quad.frag.spv
