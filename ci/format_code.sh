#!/bin/bash

# TODO: add '--all' option

ORG_CLANG_FORMAT=$(which clang-format)
PATH=".:$PATH" pre-commit run --all-files
