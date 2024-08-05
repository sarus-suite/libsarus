#!/bin/bash
# Script: check_host.sh
#
# Check whether the runtime host environment is compatible with libsarus.
# CAVEAT: not to be confused with the *build* environment.

exec 3>&1
exec 2>/dev/null
exec >/dev/null

# In reference to: https://apple.stackexchange.com/questions/83939/compare-multi-digit-version-numbers-in-bash/123408#123408
ver_to_num() {
  echo "$@" | awk -F. '{ printf("%d%03d%03d\n", $1,$2,$3); }';
}

pass() {
  echo "[+] check: $1: okay." >&3
  shift
  for MSG in "$@"; do
    echo "[+] - $MSG" >&3
  done
}

fail() {
  echo "[-] check: $1: failed." >&3
  shift
  for MSG in "$@"; do
    echo "[-]  - $MSG" >&3
  done
  HAS_ERROR=1
}

info() {
  echo "[*] $1" >&3
}

#-----------------------------------------------------------------------------#

# Check: Linux kernel version
MINIMUM_KERNEL_VER="3.0"
OBTAINED_KERNEL_VER=$(cat /proc/version | grep -Eo '[0-9]\.[0-9]+\.[0-9]+' | head -n1)
if [ $(ver_to_num $OBTAINED_KERNEL_VER) -lt $(ver_to_num $MINIMUM_KERNEL_VER) ]; then
  fail "kernel version" \
    "system version ($OBTAINED_KERNEL_VER) smaller than minimum ($MINIMUM_KERNEL_VER)."
else
  pass "kernel version"
fi

# Check: kernel modules
REQUIRED_MODULES="loop squashfs overlay"
for _REQ_MOD in $REQUIRED_MODULES; do
  if ! grep "\/$_REQ_MOD.ko" /lib/modules/`uname -r`/modules.*; then
    fail "kernel module ($_REQ_MOD)" \
      "'$_REQ_MOD' not loaded." \
      "consider executing '# modprobe $_REQ_MOD'"
  else
    pass "kernel module ($_REQ_MOD)"
  fi
done

# Check: mount-utils
MINIMUM_MOUNT_VER="2.20.0" # to get autoclear flag automatically be enabled
OBTAINED_MOUNT_VER=$(mount --version | grep -Eo '[0-9]\.[0-9]+\.[0-9]+' | head -n1)
if [ $(ver_to_num $OBTAINED_MOUNT_VER) -lt $(ver_to_num $MINIMUM_MOUNT_VER) ]; then
  fail "mount-utils" \
    "system version ($OBTAINED_KERNEL_VER) smaller than minimum ($MINIMUM_KERNEL_VER)."
else
  pass "mount-utils"
fi

# Check: libboost
REQUIRED_COMPONENTS="filesystem regex"
MINIMUM_BOOST_VER="1.85.0"
for _REQ_BOOST in $REQUIRED_COMPONENTS; do
  OBTAINED_BOOST_VER=$(ldconfig -v | grep libboost_$_REQ_BOOST | awk -F"[. ]" '{ printf "%s.%s.%s", $3, $4, $5 }')
  if [ $(ver_to_num $OBTAINED_BOOST_VER) -lt $(ver_to_num $MINIMUM_BOOST_VER) ]; then
    if [ -z $OBTAINED_BOOST_VER ]; then
      _DIAG="'$_REQ_BOOST' not installed."
    else
      _DIAG="system version ($OBTAINED_BOOST_VER) smaller than minimum ($MINIMUM_BOOST_VER)."
    fi
    fail "libboost ($_REQ_BOOST)" "$_DIAG"
  else
    pass "libboost ($_REQ_BOOST)"
  fi
done

# Finalize
if [ -n "$HAS_ERROR" ]; then
  info "check failed"
  exit 1
else
  info "check successful"
  exit 0
fi
