#!/bin/bash

set -e

CI_ROOT_PATH=$(dirname $(realpath ${BASH_SOURCE[0]}))
ROOT_PATH=$CI_ROOT_PATH/..

CONTAINER_RT=podman
OS_IMAGE="ubuntu_22_04"

usage() {
  cat <<EOF
usage: $0 [OPTIONS...]
  -c|--container-rt CONTAINER_RT  Container runtime to use (default: docker)
  -i|--os-image OS_IMAGE          OS image name to use (default: ubuntu_22_04)
EOF
}

error() {
  echo "error: $@"
}

info() {
  echo "info: $@"
}

newline() {
  echo
}

#-----------------------------------------------------------------------------#

# Parse options
while [[ $# -gt 0 ]]; do
  case $1 in
    -c|--container-rt)
      CONTAINER_RT="$2"
      shift
      shift
      ;;
    -i|--os-image)
      OS_IMAGE="$2"
      shift
      shift
      ;;
    *)
      error "unrecognized option '$1'"
      newline
      usage
      exit 1
      ;;
  esac
done

# Sanity check
if ! which $CONTAINER_RT >/dev/null 2>/dev/null; then
  error "container runtine '$CONTAINER_RT' uninstalled"
  exit 1
fi

if [[ ! -f $CI_ROOT_PATH/dockerfile/Dockerfile.$OS_IMAGE ]]; then
  error "os image '$OS_IMAGE' unsupported"
  exit 1
fi

OS_IMAGE_NAME=libsarus-build:$OS_IMAGE

# Create image if needed.
if [ -z "$($CONTAINER_RT images -q $OS_IMAGE_NAME 2>/dev/null)" ]; then
  pushd $CI_ROOT_PATH/dockerfile
  $CONTAINER_RT build -f Dockerfile.$OS_IMAGE -t $OS_IMAGE_NAME
  popd
fi

$CONTAINER_RT run --rm -it --privileged \
  --mount type=bind,source=$ROOT_PATH,target=/libsarus \
  --workdir /libsarus $OS_IMAGE_NAME
