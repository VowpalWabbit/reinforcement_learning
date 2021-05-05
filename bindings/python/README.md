# Python Bindings

## Build + Install

To build the python binding on Ubuntu 20.04 for python 3.8, a recommended way is to first install all the dependencies using vcpkg (Release 2020.06, commit 6185aa7), and then execute the following commands:

~~~bash
sudo apt install swig
export VCPKG_ROOT=<path to vcpkg directory>
export RL_PYTHON_EXT_DEPS=`realpath ./ext_deps`
./bindings/python/build_linux/restore.sh
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DBUILD_PYTHON=On -DPY_VERSION=3.8
make -j`nproc` _rl_client
pip install ../bindings/python/dist/rl_client-<VERSION>.whl
~~~

## Usage

After successful installation, an example is in `examples/python/basic_usage.py`.