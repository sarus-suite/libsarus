#!/bin/bash
# Script: install_dep.sh
#
# Install all dependencies needed to build libsarus using Spack.
# CAVEAT: dependencies are locally installed in the Spack directory.

LIBSARUS_ROOT_PATH=$(dirname $(realpath "${BASH_SOURCE[0]}"))
LOCAL_SPACK_PATH=$LIBSARUS_ROOT_PATH/ci/spack/bin

if [[ $($LOCAL_SPACK_PATH/spack env status) != *"No active env"* ]]; then 
  echo "error: Spack environment already activated."
  echo "error: if you're in a build container, no need to do $0."
  exit
fi

export PATH=$LIBSARUS_ROOT_PATH/ci/spack/bin:$PATH
. $LIBSARUS_ROOT_PATH/ci/spack/share/spack/setup-env.sh
cp $LIBSARUS_ROOT_PATH/ci/spack.yaml.base $LIBSARUS_ROOT_PATH/ci/spack.yaml

spack mirror add v0.22.1 https://binaries.spack.io/v0.22.1
spack buildcache keys --install --trust
spack env activate ci 
spack install
