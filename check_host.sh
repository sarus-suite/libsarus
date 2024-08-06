#!/bin/bash
# Script: check_host.sh
#
# Check whether the runtime host environment is compatible with libsarus.
# CAVEAT: not to be confused with the *build* environment.

exec 3>&1
#exec 2>/dev/null
#exec >/dev/null

# In reference to: https://apple.stackexchange.com/questions/83939/compare-multi-digit-version-numbers-in-bash/123408#123408
ver_to_num() {
  echo "$@" | awk -F. '{ printf("%d%03d%03d\n", $1,$2,$3); }';
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
if [[ ! -f /dev/loop* ]]; then
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

if ! lsmod | grep overlay; then
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

# Check: libboost
REQUIRED_COMPONENTS="filesystem regex"
MINIMUM_BOOST_VER="1.85.0"
for _REQ_BOOST in $REQUIRED_COMPONENTS; do
  ldconfig -v
  OBTAINED_BOOST_VER=$(ldconfig -v | grep libboost_$_REQ_BOOST | awk -F"[. ]" '{ printf "%s.%s.%s", $3, $4, $5 }')
  if [ $(ver_to_num $OBTAINED_BOOST_VER) -lt $(ver_to_num $MINIMUM_BOOST_VER) ]; then
    if [ -z $OBTAINED_BOOST_VER ]; then
      _DIAG="Library '$_REQ_BOOST' not installed"
    else
      _DIAG="System version ($OBTAINED_BOOST_VER) smaller than minimum ($MINIMUM_BOOST_VER)"
    fi
    fail "libboost ($_REQ_BOOST)" "$_DIAG"
  else
    pass "libboost ($_REQ_BOOST)"
  fi
done

# Finalize
if [ -n "$HAS_ERROR" ]; then
  info "Check $(reddify failed)"
  exit 1
else
  info "Check successful"
  exit 0
fi
