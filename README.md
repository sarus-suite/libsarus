# libsarus

Essential code collection for custom container runtime development.


## Build

1. **Install third-party dependencies.** `install_dep.sh` will install all necessary dependencies in the local directory using Spack.

```
$ ./install_dep.sh
```

2. **Build `libsarus`**. `build.sh` will build `libsarus` using the dependencies installed by Spack.

```
$ ./build.sh
```


## Test

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


## Using container environment

You can enter a container environment where all dependencies were already installed with the following command.

```
$ ci/enter_buildenv.sh
```

You can build and test `libsarus` inside the container as usual. Notice that **your host environment (i.e., outside the container) still need to pass `ci/check_host.sh` to properly run tests.**