# libsarus

Essential code collection for custom container runtime development.


## Requirements

### Build

- TODO

### Runtime

- Linux kernel 3.0+ (with `cgroup-v1`)
- Kernel modules (`loop`, `squashfs`, `overlay`)
- libboost 1.85.0+

Runtime requirements can be checked with `ci/check_host.sh`.


## Build and Test

### On the host

<details>

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

3. **Run `ctest` with `sudo`.**

```
$ cd build; sudo ctest #OPTIONAL: --output-on-failure
```

</details>

### In a container

<details>

#### Entering a container

You can enter a container environment where all dependencies were already installed as follows.

```
$ ci/enter_buildenv.sh
```

This will create a container image if necessary and bring you inside the container. The `libsarus` root directory will be mounted to `~/libsarus`, where you can build and test `libsarus` as usual. 

**Caveat: your host environment (i.e., outside the container) still needs to pass `ci/check_host.sh` to properly run tests inside the container.**

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

3. **Run `ctest` with `sudo`.** NOTE: notice `env "PATH=$PATH"`. This is required to carry over the path of Spack-installed dependencies to `sudo`.

```
$ cd build; sudo env "PATH=$PATH" ctest #OPTIONAL: --output-on-failure
```

</details>
