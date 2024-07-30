#!/bin/bash

LIBSARUS_ROOT_PATH=$(dirname $(realpath "${BASH_SOURCE[0]}"))

BUILD_DIR=$PWD/build
TOOLCHAIN_FILE=gcc.cmake
BUILD_TYPE=Debug
INSTALL_DIR=$PWD/build/install

export PATH=$LIBSARUS_ROOT_PATH/spack/bin:$PATH
. $LIBSARUS_ROOT_PATH/spack/share/spack/setup-env.sh
spack env activate .

cmake -DCMAKE_TOOLCHAIN_FILE=$BUILD_DIR/../cmake/toolchain_files/$TOOLCHAIN_FILE \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
      -B $BUILD_DIR -S .
