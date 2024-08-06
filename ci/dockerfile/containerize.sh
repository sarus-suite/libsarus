#!/bin/bash

LIBSARUS_ROOT_PATH=$(dirname $(realpath "${BASH_SOURCE[0]}"))/..

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

add_inline_placeholder "FINAL_OS_IMAGE"
add_inline_placeholder "OS_PKG_MANAGER"
add_inline_placeholder "SPACK_DEP_PKGS"

case $TARGET_OS in
  "ubuntu:22.04")
    FINAL_OS_IMAGE="ubuntu:22.04"
    OS_PKG_MANAGER="DEBIAN_FRONTEND=noninteractive apt"
    SPACK_DEP_PKGS="build-essential ca-certificates coreutils curl \
      environment-modules gfortran git gpg lsb-release \
      python3 python3-distutils unzip zip"
    ;;
  "opensuseleap:15")
    FINAL_OS_IMAGE="opensuse/leap:15"
    OS_PKG_MANAGER="zypper"
    SPACK_DEP_PKGS="binutils bzip2 clingo cpp cpp7 dbus-1 file gcc gcc-c++ \
      gcc-fortran gcc7 gcc7-c++ gcc7-fortran gio-branding-SLE git git-core \
      glib2-tools glibc-devel gzip less libasan4 libatomic1 libbz2-devel \
      libcilkrts5 libclingo4 libctf-nobfd0 libctf0 libdbus-1-3 libduktape206 \
      libexpat1 libgdbm4 libgfortran4 libgio-2_0-0 libgmodule-2_0-0 \
      libgobject-2_0-0 libgomp1 libisl15 libitm1 libjitterentropy3 liblsan0 \
      libmpc3 libmpfr6 libmpx2 libmpxwrappers2 libopenssl1_1 \
      libpolkit-agent-1-0 libpolkit-gobject-1-0 libpython3_6m1_0 \
      libquadmath0 libsha1detectcoll1 libstdc++6-devel-gcc7 libtsan0 \
      libubsan0 libxcrypt-devel linux-glibc-devel make patch perl perl-Error \
      perl-Git pkg-config polkit python3-base python3-cffi python3-clingo \
      python3-pycparser shared-mime-info spack spack-recipes sudo tar unzip \
      which xz"
    ;;
  "rocky:9")
    FINAL_OS_IMAGE="rockylinux:9"
    OS_PKG_MANAGER="dnf"
    SPACK_DEP_PKGS="epel-release \
      autoconf automake binutils bison flex gcc gcc-c++ gdb \
      glibc-devel libtool make pkgconf pkgconf-m4 pkgconf-pkg-config \
      redhat-rpm-config rpm-build rpm-sign strace \
      asciidoc byacc diffstat git intltool jna ltrace patchutils \
      perl-Fedora-VSP perl-generators pesign source-highlight systemtap \
      valgrind valgrind-devel \
      findutils gcc-gfortran gnupg2 hostname iproute \
      python3 python3-pip python3-setuptools unzip python3-botocore"
      # curl redhat-lsb-core
    ;;
  *)
    echo "error: shouldn't reach here."
    exit
esac

add_multiline_placeholder "SPACK_YAML_FILE"

ORG_SPACK_YAML_FILE=../spack.yaml
SPACK_YAML_FILE=/tmp/.spack.yaml
cp $ORG_SPACK_YAML_FILE $SPACK_YAML_FILE
sed -i '/^$/d' $SPACK_YAML_FILE
sed -i 's,\(.*\),\&\&   echo "\1" \\,g' $SPACK_YAML_FILE
cat <<EOF >>$SPACK_YAML_FILE
&&   echo "  concretizer:" \\
&&   echo "    unify: true" \\
&&   echo "  config:" \\
&&   echo "    build_jobs: 8" \\
&&   echo "    install_tree: ~/software" \\
&&   echo "  view: ~/views/view" \\
EOF

replace_placeholders

finish_containerize
