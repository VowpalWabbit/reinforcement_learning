# Python Bindings

## Build + Install

1. `sudo apt-get install swig`
2. `sudo add-apt-repository ppa:deadsnakes/ppa`
3. `sudo apt-get update`
4. `sudo apt-get install python3.6`
5. [Install pip for python3.6](https://askubuntu.com/questions/889535/how-to-install-pip-for-python-3-6-on-ubuntu-16-10)
6. `sudo python3.6 -m pip install -r bindings/python/dev-requirements.txt`
7. `mkdir ext_deps`
8. ``export RL_PYTHON_EXT_DEPS=`realpath ./ext_deps` ``
9. `bash ./bindings/python/build_linux/restore.sh`
10. `mkdir build`
11. `cd build`
12. `cmake .. -DBUILD_PYTHON=On -DRL_PY_VERSION=3.6`
13. `make`
14. `sudo python3.6 -m pip install ../bindings/python/dist/<output-file>.whl`

## Usage
To use the client Python bindings please use [these instructions](https://microsoft.github.io/vowpal_wabbit/reinforcement_learning/doc/python/html/index.html).
