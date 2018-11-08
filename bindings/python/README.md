# Python Bindings

## Build + Install

1. `sudo add-apt-repository ppa:deadsnakes/ppa`
2. `sudo apt-get update`
3. `sudo apt-get install python3.6`
4. [Install pip for python3.6](https://askubuntu.com/questions/889535/how-to-install-pip-for-python-3-6-on-ubuntu-16-10)
5. `sudo python3.6 -m pip install -r bindings/python/dev-requirements.txt`
6. `mkdir build`
7. `cd build`
8. `cmake .. -DBUILD_PYTHON=On -DRL_PY_VERSION=3.6`
9. `sudo python3.6 -m pip install ../bindings/python/dist/<output-file>.whl`

## Usage
To use the client Python bindings please use [these instructions](https://microsoft.github.io/vowpal_wabbit/reinforcement_learning/doc/python/html/index.html).
