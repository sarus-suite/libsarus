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

TARGET_DOCKERFILE_NAME=
INTERMEDIATE_FILE=/tmp/.Dockerfile.gen

prepare_containerize() {
  TARGET_DOCKERFILE_NAME=Dockerfile.gen."$1"

  if ! is_supported_os "$1"; then
    echo "usage: $0 <target_os>"
    exit
  fi

  cp Dockerfile.base $INTERMEDIATE_FILE
}

finish_containerize() {
  mv $INTERMEDIATE_FILE $TARGET_DOCKERFILE_NAME
}

#------------------------------------------------------------------------------#

PLACEHOLDERS_INLINE=
PLACEHOLDERS_MULTILINE=

add_inline_placeholder() {
  PLACEHOLDERS_INLINE="$1 $PLACEHOLDERS_INLINE"
}

add_multiline_placeholder() {
  PLACEHOLDERS_MULTILINE="$1 $PLACEHOLDERS_MULTILINE"
}

replace_inline_placeholders() {
  for _PLACEHOLDER in $PLACEHOLDERS_INLINE; do
    sed -i "s,@@${_PLACEHOLDER}@@,${!_PLACEHOLDER},g" $INTERMEDIATE_FILE
  done
}

replace_multiline_placeholders() {
  # NOTE: for multiline placeholders, the variable is a path of the file that
  # contains the real content of the placeholder.
  for _PLACEHOLDER in $PLACEHOLDERS_MULTILINE; do
    sed -i "/@@$_PLACEHOLDER@@/r ${!_PLACEHOLDER}" $INTERMEDIATE_FILE
    sed -i "/@@$_PLACEHOLDER@@/d" $INTERMEDIATE_FILE
  done
}

replace_placeholders() {
  replace_inline_placeholders
  replace_multiline_placeholders
}

#------------------------------------------------------------------------------#

add_supported_os "ubuntu:22.04"
add_supported_os "opensuseleap:15"
add_supported_os "rocky:9"

TARGET_OS="$1"
prepare_containerize "$TARGET_OS" 

add_inline_placeholder "BUILD_OS_IMAGE"
add_inline_placeholder "FINAL_OS_IMAGE"
add_inline_placeholder "OS_PKG_MANAGER"

case $TARGET_OS in
  "ubuntu:22.04")
    BUILD_OS_IMAGE="spack/ubuntu-jammy:develop"
    FINAL_OS_IMAGE="ubuntu:22.04"
    OS_PKG_MANAGER="DEBIAN_FRONTEND=noninteractive apt"
    ;;
  "opensuseleap:15")
    BUILD_OS_IMAGE="spack/leap15:develop"
    FINAL_OS_IMAGE="opensuse/leap:15"
    OS_PKG_MANAGER="zypper -y"
    ;;
  "rocky:9")
    BUILD_OS_IMAGE="spack/rockylinux9:develop"
    FINAL_OS_IMAGE="rockylinux:9"
    OS_PKG_MANAGER="yum"
    ;;
  *)
    echo "error: shouldn't reach here."
    exit
esac

add_multiline_placeholder "SPACK_YAML_FILE"

SPACK_YAML_FILE=/tmp/.spack.yaml
cp spack.yaml $SPACK_YAML_FILE
sed -i '/^$/d' $SPACK_YAML_FILE
sed -i 's,\(.*\),\&\&   echo "\1" \\,g' $SPACK_YAML_FILE
cat <<EOF >>$SPACK_YAML_FILE
&&   echo "  concretizer:" \\
&&   echo "    unify: true" \\
&&   echo "  config:" \\
&&   echo "    install_tree: ~/software" \\
&&   echo "  view: ~/views/view" \\
EOF

replace_placeholders

finish_containerize
