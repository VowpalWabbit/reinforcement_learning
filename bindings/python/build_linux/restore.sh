#!/bin/bash

set -e

CPPREST_PATH='https://github.com/Microsoft/cpprestsdk.git'
CPPREST_FOLDER='cpprestsdk'

BOOST_PATH='https://sourceforge.net/projects/boost/files/boost/1.58.0/boost_1_58_0.tar.gz/download'
BOOST_FOLDER='boost_1_58_0'

LIBSSL_PATH='https://www.openssl.org/source/old/1.0.2/openssl-1.0.2g.tar.gz'
LIBSSL_FOLDER='openssl-1.0.2g'

ZLIB_PATH='https://zlib.net/fossils/zlib-1.2.8.tar.gz'
ZLIB_FOLDER='zlib-1.2.8'

: ${RL_PYTHON_EXT_DEPS?"RL_PYTHON_EXT_DEPS environment variable must be set with the path to save dependencies to"}
BUILD_DIR=$RL_PYTHON_EXT_DEPS

mkdir -p $BUILD_DIR
BUILD_DIR=$(readlink -f $BUILD_DIR)

SRC_DIR=$BUILD_DIR/src
mkdir -p $SRC_DIR

function install_cpprest {
  cd $SRC_DIR
  git clone https://github.com/Microsoft/cpprestsdk.git $CPPREST_FOLDER
  cd $CPPREST_FOLDER/Release
  git checkout e8dda215426172cd348e4d6d455141f40768bf47
  mkdir build
  cd build
  cmake .. -DBUILD_SHARED_LIBS=0 -DCMAKE_CXX_FLAGS=-fPIC -DBUILD_SAMPLES=Off -DBUILD_TESTS=Off
  make -j8

  cp Binaries/libcpprest.a $BUILD_DIR/
}

function install_boost {
  cd $SRC_DIR

  wget -O boost.tar.gz $BOOST_PATH
  tar -xvzf boost.tar.gz
  cd $BOOST_FOLDER
  ./bootstrap.sh --prefix=boost_output --with-libraries=program_options,system > /dev/null
  ./bjam cxxflags=-fPIC cflags=-fPIC -a install > /dev/null

  cp boost_output/lib/libboost_system.a boost_output/lib/libboost_program_options.a $BUILD_DIR
}

function install_libssl {
  cd $SRC_DIR

  wget -O openssl.tar.gz $LIBSSL_PATH
  tar -xvzf openssl.tar.gz
  cd $LIBSSL_FOLDER
  ./config --prefix=ssl_output --openssldir=ssl_output no-shared -fPIC
  make depend
  make

  cp libssl.a libcrypto.a $BUILD_DIR
}

function install_zlib {
  cd $SRC_DIR

  wget -O zlib.tar.gz $ZLIB_PATH
  tar xvzf zlib.tar.gz
  cd $ZLIB_FOLDER
  ./configure --prefix=zlib --static --archs=-fPIC
  make

  cp libz.a $BUILD_DIR
}

#Building CppRest
if [ ! -f $BUILD_DIR/libcpprest.a ]; then
  install_cpprest >> $BUILD_DIR/cpprest.log 2>&1 &
fi

#Building Boost
if [ ! -f $BUILD_DIR/libboost_system.a ] || [ ! -f $BUILD_DIR/libboost_program_options.a ]; then
  install_boost >> $BUILD_DIR/boost.log 2>&1 &
fi

#Building libssl
if [ ! -f $BUILD_DIR/libssl.a ] || [ ! -f $BUILD_DIR/libcrypto.a ]; then
  install_libssl >> $BUILD_DIR/libssl.log 2>&1 &
fi

#Building zlib
if [ ! -f $BUILD_DIR/libz.a ]; then
  install_zlib >> $BUILD_DIR/zlib.log 2>&1 &
fi

echo "Installing... Check logs in $BUILD_DIR for details"
# Wait for all of the install processes to complete
wait

echo "Done"
