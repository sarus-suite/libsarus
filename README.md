# libsarus

Essential code collection for custom container runtime development.

**Runtime Requirements**
- Linux kernel 3.0+
- Kernel modules (`loop`, `squashfs`, `overlay`)


## Build

Use devcontainers for development.

### Requirements

 - Runtime requirements
 - Docker (rootful)

Run `ci/check_devhost.sh` to check build requirements.

### Procedure

 1. Clone the repository. Make sure to clone every submodule (`clone --recursive`).
 2. Enter the devcontainer via VSCode. Alternatively, you can enter a devcontainer on CLI using `ci/enter_buildenv.sh`.
 3. Build `libsarus` using `./build.sh`. You can find `libsarus.a` under `./build/src`.

### Notes

 - To build `libsarus` with a release mode setting, modify `BUILD_TYPE` in `./build.sh` to `Release`. (default: `Debug`)
 - To disable unit tests, add `-DENABLE_UNIT_TESTS=FALSE` to CMake options in `./build.sh`. (default: `TRUE`)
 - To build `libsarus` as a shared library, add `-DBUILD_SHARED_LIBS=TRUE` to CMake options in `./build.sh`. (default: `FALSE`) **Caveat: this will create another runtime dependency to Boost 1.85 (`filesystem` and `regex`).**

## Test

`libsarus` has some basic unit tests to verify the functionality. To run unit tests,

 1. Build `libsarus` first (with unit tests enabled).
 2. In the same devcontainer, run `ctest` in the `./build` directory. Alternatively, selectively run unit tests by permission as follows.

```
$ ctest -E AsRoot                         # Do unprivileged tests
$ sudo env "PATH=$PATH" ctest -R AsRoot   # Do privileged tests
```

The CI pipeline runs the same unit tests upon each push.

## Contribution

All contributions should pass all CI pipeline stages. If you don't have write permission to this repository, please submit your contribution via PR.

### Formatting

Failing to satisfy the code formatting standard causes the pipeline stage `check.format` to fail. To avoid this, run `./ci/format_code.sh` to fix the formatting issues automatically before pushing commits.
