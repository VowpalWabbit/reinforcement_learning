#!/usr/bin/env bash
set -euo pipefail

: "${PYTHON:?Set PYTHON to the Python interpreter used to build the wheel}"

"$PYTHON" -m pip install setuptools==84.2.1 wheel==0.48.0 auditwheel==6.8.2
"$PYTHON" setup.py \
  --cmake-options="-DCMAKE_TOOLCHAIN_FILE=$PWD/ext_libs/vcpkg/scripts/buildsystems/vcpkg.cmake;-DVCPKG_MANIFEST_MODE=ON;-DRAPIDJSON_SYS_DEP=ON;-DFMT_SYS_DEP=ON;-DSPDLOG_SYS_DEP=ON;-DVW_ZLIB_SYS_DEP=ON;-DVW_BOOST_MATH_SYS_DEP=ON;-DVW_GTEST_SYS_DEP=ON;-DCMAKE_POSITION_INDEPENDENT_CODE=ON" \
  bdist_wheel --dist-dir wheel_output
"$PYTHON" -m auditwheel repair wheel_output/*.whl --wheel-dir audit_output --plat manylinux_2_28_x86_64
