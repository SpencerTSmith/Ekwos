#!/bin/env bash
set -euo pipefail

PROJECT_NAME="ekwos"

SRC_DIR="src"
LIBS_DIR="libs"
SHADER_DIR="src/shaders"

BIN_DIR="bin"
OUTPUT_SHADER_DIR="${BIN_DIR}/shaders"

mkdir -p "${BIN_DIR}"
mkdir -p "${OUTPUT_SHADER_DIR}"

SHADER_SRCS=$(find "${SHADER_DIR}" -name "*.vert" -o -name "*.frag")

for SHADER in ${SHADER_SRCS}; do
	SHADER_NAME=$(basename ${SHADER})
	OUT_SHADER="${OUTPUT_SHADER_DIR}/${SHADER_NAME}.spv"

	echo "Compiling shader ${SHADER}"
	glslc "${SHADER}" -o "${OUT_SHADER}"
done

C_SOURCES=$(find "${SRC_DIR}" -name "*.c")
LIB_SOURCES=$(find "${LIBS_DIR}" -name "*.c")

CFLAGS=" -g -Wall -Wextra -Wshadow -Wpedantic -DDEBUG=1 -DOS_LINUX=1 -std=gnu17"
LDFLAGS="-lglfw -lvulkan -lm"

echo "Compiling C sources..."
gcc ${CFLAGS} -I${SRC_DIR} -I${LIBS_DIR} ${C_SOURCES} ${LDFLAGS} -o "${BIN_DIR}/${PROJECT_NAME}"

echo "Build Complete... ${BIN_DIR}/${PROJECT_NAME}"
