# libsarus

Essential code collection for custom container runtime development.


## Runtime Requirements

- Linux kernel 3.0+
- Kernel modules (`loop`, `squashfs`, `overlay`)
- libboost 1.85.0+

Use `ci/check_host.sh` to check runtime requirements.


## Build and Test

### On the host

You can develop, build, and test `libsarus` directly on your host machine.

#### Requirements

- Linux kernel 3.0+
- Kernel modules (`loop`, `squashfs`, `overlay`)
- Spack dependencies (See [this link](https://spack.readthedocs.io/en/latest/getting_started.html))

Use `ci/check_host.sh --no-dep` to check build requirements (except for Spack dependencies).

<details>
  <summary>Click here for procedure</summary>

#### Clone

Make sure to clone every submodule when cloning.

```
$ git clone --recursive <repo_url>
```

#### Build

1. **Install third-party dependencies.** `install_dep.sh` will install all necessary dependencies in the local directory using Spack.

```
$ ./install_dep.sh
```

2. **Build `libsarus`**. `build.sh` will build `libsarus` using the dependencies installed by Spack.

```
$ ./build.sh
```

#### Test

1. **Check if your system can run tests.** The dependency check is unnecessary as tests are pre-linked to dependencies at build time.

```
$ ./ci/check_host.sh --no-dep
```

2. **Build `libsarus`.**

```
$ ./build.sh
```

3. **Run `ctest` in `build`.**

```
$ ctest -E AsRoot        # Do unprivileged tests
$ sudo ctest -R AsRoot   # Do privileged tests
```

</details>

### In a container

Alternatively, you can build and test `libsarus` in a container image while doing development on your host machine. A provided script will enter a container image with all dependencies pre-installed and mount the `libsarus` root directory to `~/libsarus`.

#### Requirements

- Linux kernel 3.0+
- Kernel modules (`loop`, `squashfs`, `overlay`)
- Docker

Use `ci/check_host.sh --no-dep` to check build requirements (except for Docker).

<details>
  <summary>Click here for procedure</summary>

#### Clone

Make sure to clone every submodule when cloning.

```
$ git clone --recursive <repo_url>
```

#### Entering a container

Enter a container environment as follows.

```
$ ci/enter_buildenv.sh
```

#### Build

1. **Build `libsarus`**. `build.sh` will build `libsarus` using the dependencies installed by Spack.

```
$ ./build.sh
```

#### Test

1. **Check if your system can run tests.** The dependency check is unnecessary as tests are pre-linked to dependencies at build time.

```
$ ./ci/check_host.sh --no-dep
```

2. **Build `libsarus`.**

```
$ ./build.sh
```

3. **Run `ctest` in `build`.** `env "PATH=$PATH"` forwards the path of Spack-installed dependencies to `sudo`.

```
$ ctest -E AsRoot                         # Do unprivileged tests
$ sudo env "PATH=$PATH" ctest -R AsRoot   # Do privileged tests
```

</details>

## Contribution

All contributions should pass all pipeline stages. If you don't have write permission to this repository, please submit your contribution via PR.

### Formatting

Code formatting will be automatically checked in the pipeline upon PR; if some code doesn't match the standard, the pipeline stage `check.format` will generate a warning. To avoid such a formatting warning, consider running `ci/format_code.sh` to automatically format all code.
