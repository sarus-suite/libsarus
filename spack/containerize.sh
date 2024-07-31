#!/bin/bash

LIBSARUS_ROOT_PATH=$(dirname $(realpath "${BASH_SOURCE[0]}"))/..

export PATH=$LIBSARUS_ROOT_PATH/spack/spack/bin:$PATH
. $LIBSARUS_ROOT_PATH/spack/spack/share/spack/setup-env.sh

SUPPORTED_OS=

add_supported_os() {
  SUPPORTED_OS="$1 $SUPPORTED_OS"
}

is_supported_os() {
  [[ -n $1 ]] && (echo $SUPPORTED_OS | grep -q $TARGET_OS)
}

#------------------------------------------------------------------------------#

add_supported_os "ubuntu:22.04"
add_supported_os "opensuseleap:15"
add_supported_os "rocky:9"

TARGET_OS="$1"
TARGET_DOCKERFILE_NAME=Dockerfile.gen.$TARGET_OS

if ! is_supported_os $TARGET_OS; then
  echo "usage: $0 <target_os>"
  exit
fi

cp Dockerfile.base $TARGET_DOCKERFILE_NAME

case $TARGET_OS in
  "ubuntu:22.04")
    BUILD_OS_IMAGE="spack/ubuntu-jammy:develop"
    FINAL_OS_IMAGE="ubuntu:22.04"
    ;;
  "opensuseleap:15")
    BUILD_OS_IMAGE="spack/leap15:develop"
    FINAL_OS_IMAGE="opensuse/leap:15"
    ;;
  "rocky:9")
    BUILD_OS_IMAGE="spack/rockylinux9:develop"
    FINAL_OS_IMAGE="rockylinux:9"
    ;;
  *)
    echo "error: shouldn't reach here."
    exit
esac

sed -i "s,@@BUILD_OS_IMAGE@@,$BUILD_OS_IMAGE,g" $TARGET_DOCKERFILE_NAME
sed -i "s,@@FINAL_OS_IMAGE@@,$FINAL_OS_IMAGE,g" $TARGET_DOCKERFILE_NAME

TMP_SPACK_YAML=/tmp/.spack.yaml
cp spack.yaml $TMP_SPACK_YAML
sed -i '/^$/d' $TMP_SPACK_YAML
sed -i 's,\(.*\),\&\&   echo "\1" \\,g' $TMP_SPACK_YAML
cat <<EOF >>$TMP_SPACK_YAML
&&   echo "  concretizer:" \\
&&   echo "    unify: true" \\
&&   echo "  config:" \\
&&   echo "    install_tree: /opt/software" \\
&&   echo "  view: /opt/views/view" \\
EOF

sed -i "/@@SPACK_YAML_FILE@@/r $TMP_SPACK_YAML" $TARGET_DOCKERFILE_NAME
sed -i "/@@SPACK_YAML_FILE@@/d" $TARGET_DOCKERFILE_NAME
rm $TMP_SPACK_YAML
