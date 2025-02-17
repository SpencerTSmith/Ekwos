#!/bin/bash

set -e
set -o pipefail

CC=cc
CFLAGS="-g -Wall -Wextra -Wshadow -Wpedantic -DDEBUG=1"
LDFLAGS="-lglfw -lvulkan -m"

SRC_DIR="src"
BUILD_DIR="bin"
OUT="ekwos"

# All c files
SRC_FILES=($(find "$SRC_DIR" -type f -name "*.c"))

OBJ_FILES=("${SRC_FILES[@]/%.c/.o}")
OBJ_FILES=("${OBJ_FILES[@]/$SRC_DIR/$BUILD_DIR}")

echo "Compiling..."
for i in "${!SRC_FILES[@]}"; do
	echo "	${SRC_FILES[$i]} -> ${OBJ_FILES[$i]}"
	$CC $CFLAGS -c "${SRC_FILES[$i]}" -o "${OBJ_FILES[$i]}"
done

echo "Linking..."
$CC $OBJ_FILES $LDFLAGS -o "$BUILD_DIR/$OUT"

echo "Build complete: $BUILD_DIR/$OUT"
