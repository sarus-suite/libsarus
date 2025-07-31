#!/bin/bash

LIBSARUS_ROOT_PATH=$(dirname $(realpath ${BASH_SOURCE[0]}))/..

OPT_FILE=""
if [[ "$1" == "--changed" ]]; then
  OPT_FILES="--files $(git status --short | awk '{ print $2 }')"
elif [[ -z "$1" ]]; then
  OPT_FILES="--all-files"
else
  echo "usage: $0 [--changed]"
  echo "    --changed   Format changed source code (default: all)"
  exit 1
fi

if ! which pre-commit; then
  echo "error: pre-commit not installed."
  echo "error: pre-commit may be installed with pip."
  exit 1
fi

if ! which clang-format; then
  echo "error: clang-format not installed."
  exit 1
fi

pre-commit run $OPT_FILES
