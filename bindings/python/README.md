# Python Bindings

## Build + Install

Commands are relative to repo root.
```bash
python setup.py install

# Or, if vcpkg used for deps
python setup.py --cmake-options="-DCMAKE_TOOLCHAIN_FILE=/path_to_vcpkg_root/scripts/buildsystems/vcpkg.cmake" install
```

- For Ubuntu 20.04, Python 3.8 a recommended vcpkg version is: `Release 2020.06, commit 6185aa7`

## Usage

After successful installation, an example is in [`examples/python/basic_usage.py`](../../examples/python/basic_usage.py).
