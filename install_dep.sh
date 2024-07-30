#!/bin/bash

LIBSARUS_ROOT_PATH=$(dirname $(realpath "${BASH_SOURCE[0]}"))

export PATH=$LIBSARUS_ROOT_PATH/spack/bin:$PATH
. $LIBSARUS_ROOT_PATH/spack/share/spack/setup-env.sh

spack buildcache keys --install --trust

spack env activate .
spack install
