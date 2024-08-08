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

1. **Check if your system can run tests.**

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

### In a container

You can enter a container environment where all dependencies were already installed as follows.

```
$ ci/enter_buildenv.sh
```

This will create a container image if necessary and bring you inside the container. The `libsarus` root directory will be mounted to `~/libsarus`, where you can build and test `libsarus` as usual. 

**Caveat: your host environment (i.e., outside the container) still needs to pass `ci/check_host.sh` to properly run tests inside the container.**