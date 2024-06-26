#!/bin/bash

_GLSLC="$VULKAN_SDK/bin/glslc --target-env=vulkan1.3 -g"

_COMPILED_DIR="./Compiled"

mkdir -p $_COMPILED_DIR

_FILENAME="$(basename -- "$1")"
_SHADERNAME=${_FILENAME%.*}

_SHADER_VERT="$_SHADERNAME".vert
_SHADER_VERT_OUT="$_SHADERNAME"_vert.spv
_SHADER_FRAG="$_SHADERNAME".frag
_SHADER_FRAG_OUT="$_SHADERNAME"_frag.spv

if [ -f "$_SHADER_VERT" ]; then
    echo "Compiling vert shader $_SHADER_VERT"
    $_GLSLC $_SHADER_VERT -o $_COMPILED_DIR/$_SHADER_VERT_OUT
fi

if [ -f "$_SHADER_VERT" ]; then
    echo "Compiling frag shader $_SHADER_FRAG"
    $_GLSLC $_SHADER_FRAG -o $_COMPILED_DIR/$_SHADER_FRAG_OUT
fi