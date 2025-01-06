#!/bin/zsh

# Directory for shaders
SHADER_DIR="./src/shaders"
BUILD_DIR="./build/shaders"

# Create the output directory if it doesn't exist
mkdir -p "$BUILD_DIR"

# Loop over all .glsl files in the shaders directory
for shader in "$SHADER_DIR"/*.{vert,frag}; do
    # Extract shader filename without extension
    shader_name=$(basename "$shader")
    
    # Compile shader using glslc
    glslc "$shader" -o "$BUILD_DIR/$shader_name.spv"
    
    echo "Compiled $shader_name.spv"
done
