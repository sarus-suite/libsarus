#!/bin/bash
# Script: build.sh
#
# Build libsarus with the dependencies.

LIBSARUS_ROOT_PATH=$(dirname $(realpath "${BASH_SOURCE[0]}"))

BUILD_DIR=$LIBSARUS_ROOT_PATH/build
INSTALL_DIR=$LIBSARUS_ROOT_PATH/build/install
TOOLCHAIN_FILE=gcc.cmake
BUILD_TYPE=Debug

cmake -DCMAKE_TOOLCHAIN_FILE=$LIBSARUS_ROOT_PATH/cmake/toolchain_files/$TOOLCHAIN_FILE \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
      -B $BUILD_DIR -S .
make -C $BUILD_DIR -j
