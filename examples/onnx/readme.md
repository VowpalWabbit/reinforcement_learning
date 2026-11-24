# ONNX example

This is example runs RLClientLib as BYOM using an ONNX model for MNIST and logs the interactions and observations to files.

## Generate `mnist_test_data.txt`

```sh
# Install mnist python lib
python3 -m pip install python-mnist

# Run installed script to pull mnist data to ./data
mnist_get_data.sh

python3 ../../unit_test/extensions/onnx/mnist_data/data_generator.py
```

## Build example

```sh
mkdir build
cd build
cmake .. -Drlclientlib_BUILD_ONNXRUNTIME_EXTENSION=On
make onnx_example -j $(nproc)
```

## Run example

```sh
./examples/onnx/onnx_example ../examples/onnx/mnist_test_data.txt ./examples/onnx/mnist_data/mnist_model.onnx
```

This produces two files in the current directory `observation.fb.data` and `interaction.fb.data`.
