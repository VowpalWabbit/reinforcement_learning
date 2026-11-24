#!/usr/bin/env bash
set -euo pipefail

: "${PYTHON:?Set PYTHON to the legacy interpreter path to provision}"
case "${1:?Set the Python minor version}" in
  3.6)
    version=3.6.15
    sha256=6e28d7cdd6dd513dd190e49bca3972e20fcf455090ccf2ef3f1a227614135d91
    ;;
  3.7)
    version=3.7.17
    sha256=7911051ed0422fd54b8f59ffc030f7cf2ae30e0f61bda191800bb040dce4f9d2
    ;;
  3.8)
    version=3.8.20
    sha256=6fb89a7124201c61125c0ab4cf7f6894df339a40c02833bfd28ab4d7691fafb4
    ;;
  *)
    echo "Unsupported legacy Python version: $1" >&2
    exit 1
    ;;
esac

prefix="$(dirname "$(dirname "$PYTHON")")"
source_dir="$PWD/build/legacy-python-source"
mkdir -p "$source_dir"
cd "$source_dir"
curl --fail --location --retry 3 \
  "https://www.python.org/ftp/python/$version/Python-$version.tar.xz" \
  --output "Python-$version.tar.xz"
echo "$sha256  Python-$version.tar.xz" | sha256sum --check
tar -xf "Python-$version.tar.xz"
cd "Python-$version"
# AlmaLinux 8's GCC 8 and OpenSSL 1.1 build unmodified legacy CPython;
# the image's newer GCC remains selected for the native wheel itself.
CC=/usr/bin/gcc CXX=/usr/bin/g++ CPPFLAGS=-I/usr/include LDFLAGS=-L/usr/lib64 \
  PKG_CONFIG_PATH=/usr/lib64/pkgconfig \
  ./configure --prefix="$prefix" --with-ensurepip=install
make -j2
make install
ln -sf python3 "$PYTHON"
"$PYTHON" -c 'import ssl, ctypes, bz2, lzma, sys; print(sys.version)'
