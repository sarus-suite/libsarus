#!/bin/bash
# Script: check_host.sh
#
# Check whether the runtime host environment is compatible with libsarus.
# Written to be usable both on a bare-metal and inside a container.
# CAVEAT: not to be confused with the *build* environment.

exec 3>&1
exec 2>/dev/null
exec >/dev/null

# In reference to: https://apple.stackexchange.com/questions/83939/compare-multi-digit-version-numbers-in-bash/123408#123408
ver_to_num() {
  echo "$@" | awk -F. '{ printf("%d%03d%03d\n", $1,$2,$3); }';
}

yellowify() {
  echo "\033[0;33m$1\033[0m"
}

reddify() {
  echo "\033[0;31m$1\033[0m"
}

pass() {
  echo -e "-- Check for $1 - okay" >&3
  shift
  for MSG in "$@"; do
    echo ">> $MSG" >&3
  done
}

skip() {
  echo -e "-- Check for $1 - $(yellowify skipped)" >&3
  shift
  for MSG in "$@"; do
    echo ">> $MSG" >&3
  done
}

fail() {
  echo -e "-- Check for $1 - $(reddify failed)" >&3
  shift
  for MSG in "$@"; do
    echo ">> $MSG" >&3
  done
  HAS_ERROR=1
}

info() {
  echo -e "-- $1" >&3
}

#-----------------------------------------------------------------------------#

if [[ "$1" == "--ci" ]]; then
  export INSIDE_CI_PIPELINE=1
fi

# Check: Linux kernel version
MINIMUM_KERNEL_VER="3.0"
OBTAINED_KERNEL_VER=$(cat /proc/version | grep -Eo '[0-9]\.[0-9]+\.[0-9]+' | head -n1)
if [ $(ver_to_num $OBTAINED_KERNEL_VER) -lt $(ver_to_num $MINIMUM_KERNEL_VER) ]; then
  fail "kernel version" \
    "System version ($OBTAINED_KERNEL_VER) smaller than minimum ($MINIMUM_KERNEL_VER)"
else
  pass "kernel version"
fi

# Check: kernel modules
if [[ ! -e /dev/loop0 ]]; then
  fail "kernel module (loop)" \
    "Kernel module 'loop' not loaded" \
    "Consider executing '# modprobe loop'"
else
  pass "kernel module (loop)"
fi

if ! cat /proc/filesystems | grep "squashfs"; then
  fail "kernel module (squashfs)" \
    "Kernel module 'squashfs' not loaded" \
    "Consider executing '# modprobe squashfs'"
else
  pass "kernel module (squashfs)"
fi

if ! cat /proc/filesystems | grep "overlay"; then
  fail "kernel module (overlay)" \
    "Kernel module 'overlay' not loaded" \
    "Consider executing '# modprobe overlay'"
else
  pass "kernel module (overlay)"
fi

# Check: mount-utils
MINIMUM_MOUNT_VER="2.20.0" # to get autoclear flag automatically be enabled
OBTAINED_MOUNT_VER=$(mount --version | grep -Eo '[0-9]\.[0-9]+\.[0-9]+' | head -n1)
if [ $(ver_to_num $OBTAINED_MOUNT_VER) -lt $(ver_to_num $MINIMUM_MOUNT_VER) ]; then
  fail "mount-utils" \
    "System version ($OBTAINED_KERNEL_VER) smaller than minimum ($MINIMUM_KERNEL_VER)"
else
  pass "mount-utils"
fi

# Check: rootful Docker
if [[ ! -v INSIDE_CI_PIPELINE ]]; then
  if docker info -f "{{println .SecurityOptions}}" | grep rootless >/dev/null 2>&1; then
    fail "Docker is rootless. Rootful Docker is required."
  else
    pass "rootful Docker"
  fi
fi

# Finalize
if [ -n "$HAS_ERROR" ]; then
  info "Check $(reddify failed)"
  exit 1
else
  info "Check successful"
  exit 0
fi
