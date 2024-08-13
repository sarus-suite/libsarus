#!/bin/bash

LIBSARUS_ROOT_PATH=$(dirname $(realpath ${BASH_SOURCE[0]}))/..

OPT_ALL_FILES=""
if [[ "$1" == "--all" ]]; then
  OPT_ALL_FILES="--all-files"
elif [[ -n "$1" ]]; then
  echo "usage: $0 [--all]"
  echo "    --all     Format all source code (default: only Git-staged code)"
  exit 1
fi

PRE_COMMIT_INSTALLED=0
if [[ -f $LIBSARUS_ROOT_PATH/.git/hooks/pre-commit ]]; then
  PRE_COMMIT_INSTALLED=1
fi

if [[ $PRE_COMMIT_INSTALLED == 0 ]]; then
  pre-commit install
fi
pre-commit run $OPT_ALL_FILES
if [[ $PRE_COMMIT_INSTALLED == 0 ]]; then
  pre-commit uninstall
fi
