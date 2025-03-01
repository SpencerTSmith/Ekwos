#!/bin/env bash
set -euo pipefail

PROJECT_NAME="ekwos"

SRC_DIR="src"
LIBS_DIR="libs"
SHADER_DIR="${SRC_DIR}/shaders"

BIN_DIR="bin"
OUTPUT_SHADER_DIR="${BIN_DIR}/shaders"
OBJ_DIR="${BIN_DIR}/o"

C_SOURCES=$(find "${SRC_DIR}" -name "*.c")
LIB_SOURCES=$(find "${LIBS_DIR}" -name "*.c")
SHADER_SRCS=$(find "${SHADER_DIR}" -name "*.vert" -o -name "*.frag")

CFLAGS=" -g -Wall -Wextra -Wshadow -Wpedantic -DDEBUG=1 -DOS_LINUX=1 -std=gnu17"
LDFLAGS="-lglfw -lvulkan -lm"

OBJ_FILES=()

mkdir -p "${BIN_DIR}"
mkdir -p "${OUTPUT_SHADER_DIR}"
mkdir -p "${OBJ_DIR}"

needs_rebuild() {
	local source_file=$1
	local object_file=$2

	# Object doesn't exist yet, rebuild
	if [[ ! -f "${object_file}" ]]; then
		return 0
	fi

	# Source is newer, rebuild
	if [[ "${source_file}" -nt "${object_file}" ]]; then
		return 0
	fi

	# No need to rebuild
	return 1
}

for SHADER in ${SHADER_SRCS}; do
	SHADER_NAME=$(basename ${SHADER})
	OUT_SHADER="${OUTPUT_SHADER_DIR}/${SHADER_NAME}.spv"

	if needs_rebuild "${SHADER}" "${OUT_SHADER}"; then
		echo "Compiling ${SHADER} ..."
		glslc "${SHADER}" -o "${OUT_SHADER}"
	else
		echo "${SHADER} up to date"
	fi
done

for SRC in ${C_SOURCES} ${LIB_SOURCES}; do
	OBJ_FILE="${OBJ_DIR}/$(basename ${SRC} .c).o"
	OBJ_FILES+=("${OBJ_FILE}")

	if needs_rebuild "${SRC}" "${OBJ_FILE}"; then
		echo "Compiling ${SRC} ..."
		gcc ${CFLAGS} -I${SRC_DIR} -I${LIBS_DIR} -c "${SRC}" -o "${OBJ_FILE}"
	else
		echo "${SRC} up to date"
	fi
done

if [[ ${#OBJ_FILES[@]} -gt 0 ]]; then
	echo "Linking..."
	gcc ${CFLAGS} "${OBJ_FILES[@]}" ${LDFLAGS} -o "${BIN_DIR}/${PROJECT_NAME}"
else
	echo "Project up to date"
fi

echo "Build Complete... (${BIN_DIR}/${PROJECT_NAME})"
