#!/bin/bash

LIBSARUS_ROOT_PATH=$(dirname $(realpath "${BASH_SOURCE[0]}"))

export PATH=$LIBSARUS_ROOT_PATH/ci/spack/bin:$PATH
. $LIBSARUS_ROOT_PATH/ci/spack/share/spack/setup-env.sh

spack mirror add v0.22.1 https://binaries.spack.io/v0.22.1
spack buildcache keys --install --trust
spack env activate spack
spack install
