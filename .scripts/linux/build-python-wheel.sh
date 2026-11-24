#!/usr/bin/env bash
set -euo pipefail

: "${PYTHON:?Set PYTHON to the Python interpreter used to build the wheel}"
AUDITWHEEL_PYTHON="${AUDITWHEEL_PYTHON:-/opt/python/cp311-cp311/bin/python}"

python_version=$("$PYTHON" -c 'import sys; print(sys.version_info.major * 100 + sys.version_info.minor)')
if (( python_version < 310 )); then
  "$PYTHON" -m pip install pip==21.3.1 setuptools==59.6.0 wheel==0.37.1
else
  "$PYTHON" -m pip install setuptools==84.2.1 wheel==0.48.0
fi
"$AUDITWHEEL_PYTHON" -m pip install auditwheel==6.8.2
"$PYTHON" setup.py \
  --cmake-options="-DCMAKE_TOOLCHAIN_FILE=$PWD/ext_libs/vcpkg/scripts/buildsystems/vcpkg.cmake;-DVCPKG_MANIFEST_MODE=ON;-DRAPIDJSON_SYS_DEP=ON;-DFMT_SYS_DEP=ON;-DSPDLOG_SYS_DEP=ON;-DVW_ZLIB_SYS_DEP=ON;-DVW_BOOST_MATH_SYS_DEP=ON;-DVW_GTEST_SYS_DEP=ON;-DCMAKE_POSITION_INDEPENDENT_CODE=ON" \
  bdist_wheel --dist-dir wheel_output
"$AUDITWHEEL_PYTHON" -m auditwheel repair wheel_output/*.whl --wheel-dir audit_output --plat manylinux_2_28_x86_64
