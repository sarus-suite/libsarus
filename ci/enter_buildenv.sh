#!/bin/bash

set -e

ROOT_PATH=$(dirname $(realpath ${BASH_SOURCE[0]}))/..

CONTAINER_RT="podman"
OS_IMAGE="ubuntu_22_04"

usage() {
  cat <<EOF
usage: $0 [OPTIONS...]
  -c|--container-rt CONTAINER_RT  Container runtime to use (default: podman)
  -i|--os-image OS_IMAGE          OS image name to use (default: ubuntu_22_04)
EOF
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
      echo "error: unrecognized option '$1'"
      echo
      usage
      exit 1
      ;;
  esac
done

# Sanity check
if ! which $CONTAINER_RT >/dev/null 2>/dev/null; then
  echo "error: container runtine '$CONTAINER_RT' uninstalled"
  exit 1
fi

if [[ ! -d $ROOT_PATH/.devcontainer/$OS_IMAGE ]]; then
  echo "error: os image '$OS_IMAGE' unsupported"
  exit 1
fi

if [ "$EUID" -ne 0 ]; then
  echo "warning: entering as a non-root. some unit tests may fail."
fi

OS_IMAGE_NAME=libsarus-build:$OS_IMAGE

# Create image if needed.
if [ -z "$($CONTAINER_RT images -q $OS_IMAGE_NAME 2>/dev/null)" ]; then
  pushd $ROOT_PATH/.devcontainer/$OS_IMAGE
  $CONTAINER_RT build -f Containerfile -t $OS_IMAGE_NAME
  popd
fi

$CONTAINER_RT run --rm -it --privileged \
  --mount type=bind,src=$ROOT_PATH,dst=/libsarus,relabel=private \
  --workdir /libsarus $OS_IMAGE_NAME
