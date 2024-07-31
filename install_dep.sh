#!/bin/bash

LIBSARUS_ROOT_PATH=$(dirname $(realpath "${BASH_SOURCE[0]}"))

export PATH=$LIBSARUS_ROOT_PATH/spack/spack/bin:$PATH
. $LIBSARUS_ROOT_PATH/spack/spack/share/spack/setup-env.sh

spack buildcache keys --install --trust
spack bootstrap trust
spack env activate spack
spack install

# TODO: install sudo if needed.
