#!/bin/bash

set -e

CI_ROOT_PATH=$(dirname $(realpath ${BASH_SOURCE[0]}))

CONTAINER_RT=docker
OS_IMAGE="ubuntu:22.04"

usage() {
  cat <<EOF
usage: $0 [OPTIONS...]
  -c|--container-rt CONTAINER_RT  Container runtime to use (default:docker)
  -i|--os-image OS_IMAGE          OS image name to use (default: ubuntu:22.04)
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

if [[ -z $($CI_ROOT_PATH/dockerfile/containerize.sh -p | grep $OS_IMAGE) ]]; then
  error "os image '$OS_IMAGE' unsupported"
  exit 1
fi

OS_IMAGE_NAME=libsarus/build/$OS_IMAGE

# Create image if needed.
# TODO: support other CONTAINER_RTs
if [ -z "$(docker images -q $OS_IMAGE_NAME 2>/dev/null)" ]; then
  pushd $CI_ROOT_PATH/dockerfile
  ./containerize.sh $OS_IMAGE
  $CONTAINER_RT build -f Dockerfile.gen.$OS_IMAGE -t $OS_IMAGE_NAME .
  popd
fi

$CONTAINER_RT run --rm -it --privileged \
  --mount type=bind,source=$CI_ROOT_PATH/..,target=/home/docker/libsarus \
  --workdir /home/docker/libsarus $OS_IMAGE_NAME
