#!/bin/bash

glslang -V ./quad.vert -o ./quad.vert.spv && glslang -V ./quad.frag -o ./quad.frag.spv && clang ./main.c @compile_flags.txt
