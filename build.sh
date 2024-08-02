#!/bin/bash

LIBSARUS_ROOT_PATH=$(dirname $(realpath "${BASH_SOURCE[0]}"))
LOCAL_SPACK_PATH=$LIBSARUS_ROOT_PATH/ci/spack/bin

BUILD_DIR=$PWD/build
TOOLCHAIN_FILE=gcc.cmake
BUILD_TYPE=Debug

# WARNING: the local testing environment and GitLab CI/CD use Spack differently;
# for local testing, dependencies are installed in
# $LIBSARUS_ROOT_PATH/ci/spack but for GitLab CI/CD, dependencies are
# installed in /opt/spack-environment. This subtle difference can make a
# discrepancy that something works locally but not in CI/CD.
if [[ $($LOCAL_SPACK_PATH/spack env status) == *"No active env"* ]]; then 
  export PATH=$LOCAL_SPACK_PATH:$PATH
  . $LIBSARUS_ROOT_PATH/ci/spack/share/spack/setup-env.sh
  spack env activate spack
fi

cmake -DCMAKE_TOOLCHAIN_FILE=$BUILD_DIR/../cmake/toolchain_files/$TOOLCHAIN_FILE \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -B $BUILD_DIR -S .
make -C $BUILD_DIR
