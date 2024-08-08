#!/bin/bash
# Script: build.sh
#
# Build libsarus with the dependencies installed by Spack.

LIBSARUS_ROOT_PATH=$(dirname $(realpath "${BASH_SOURCE[0]}"))
LOCAL_SPACK_PATH=$LIBSARUS_ROOT_PATH/ci/spack/bin

BUILD_DIR=$PWD/build
INSTALL_DIR=$PWD/build/install
TOOLCHAIN_FILE=gcc.cmake
BUILD_TYPE=Debug

for _ARG in "$@"; do
  if [[ $_ARG == "-D*" ]]; then
    _KEY=${_ARG#-D}; KEY=${_KEY%=*}
    VALUE=${_ARG#*=}
    declare $KEY=$VALUE
  fi
done 

# WARNING: the local testing environment and GitLab CI/CD use Spack differently;
# for local testing, dependencies are installed in
# $LIBSARUS_ROOT_PATH/ci/spack but for GitLab CI/CD, dependencies are
# installed in /opt/spack-environment. This subtle difference can make a
# discrepancy that something works locally but not in CI/CD.
if [[ $($LOCAL_SPACK_PATH/spack env status) == *"No active env"* ]]; then 
  export PATH=$LOCAL_SPACK_PATH:$PATH
  . $LIBSARUS_ROOT_PATH/ci/spack/share/spack/setup-env.sh
  if [ ! -f $LIBSARUS_ROOT_PATH/ci/spack.yaml ]; then
    cp $LIBSARUS_ROOT_PATH/ci/spack.yaml.base $LIBSARUS_ROOT_PATH/ci/spack.yaml
  fi
  spack env activate ci 
fi

cmake -DCMAKE_TOOLCHAIN_FILE=$BUILD_DIR/../cmake/toolchain_files/$TOOLCHAIN_FILE \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
      -B $BUILD_DIR -S .
make -C $BUILD_DIR install
