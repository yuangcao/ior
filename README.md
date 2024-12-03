# HPC IO Benchmark Repository [![Build Status](https://travis-ci.org/hpc/ior.svg?branch=main)](https://travis-ci.org/hpc/ior)

This repository contains the IOR and mdtest parallel I/O benchmarks.  The
[official IOR/mdtest documentation][] can be found in the `docs/` subdirectory
or on Read the Docs.

## Building

1. If `configure` is missing from the top level directory, you probably
   retrieved this code directly from the repository.  Run `./bootstrap`
   to generate the configure script.  Alternatively, download an
   [official IOR release][] which includes the configure script.

1. Run `./configure`.  For a full list of configuration options, use
   `./configure --help`.

2. Run `make`

3. Optionally, run `make install`.  The installation prefix
   can be changed via `./configure --prefix=...`.

## Testing

* Run `make check` to invoke the unit tests.
* More comprehensive functionality tests are included in `testing/`.  These
  scripts will launch IOR and mdtest via MPI.
* Docker scripts are also provided in `testing/docker/` to test various
  distributions at once.  

## Use mdtest CubeFS backend

### Install dependencies

Two dependencies: `libcfs.so, libcjson.so`.

`libcfs.so`: Run `make libsdk` in CubeFS.

`libcjson.so`：

```bash
wget https://github.com/DaveGamble/cJSON/archive/refs/tags/v1.7.18.tar.gz
tar zxvf v1.7.18.tar.gz
mkdir build
cd build
cmake ..
make
```

### Compile mdtest

The path of the two dependencies are hard-coded in the code. You should change the paths containing `cya` to the paths where your own libraries are located.

```
git clone git@github.com:yuangcao/ior.git
# change path to libs
./bootstrap
./configure --with-cubefs # To use the CubeFS backend, you should add the parameter --with-cubefs
make
```

### Usage

```
./src/mdtest -a=CUBEFS --cubefs.conf=/path/to/conf/file -F -C -n 10
```

conf file example:

```
{
  "masterAddr": "192.168.1.103:16010,192.168.1.117:16010,192.168.1.141:16010",
  "volName": "cyasdktest",
  "logDir": "/home/kvgroup/cya/cubefs-oppo/data-dist/client-cyasdktest/log",
  "logLevel": "debug",
  "enableAudit": "true",
  "accessKey": "TvrE6nMCYxuvp9AQ",
  "secretKey": "vnA4uz8MMQQW5gwBC3ES5zmfKeusHfTk"
}
```

[official IOR release]: https://github.com/hpc/ior/releases
[official IOR/mdtest documentation]: http://ior.readthedocs.org/
