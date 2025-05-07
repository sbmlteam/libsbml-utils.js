#!/bin/bash

# change into this directory
pushd "$(dirname "$0")"


# checkout and build libsbml-dependencies if they dont exist
if [ ! -f em-dependencies/lib/libexpat.a ]; then
    
    emcmake cmake -G Ninja -B em-build-dependencies -S libsbml-dependencies \
          -DCMAKE_INSTALL_PREFIX=em-dependencies \
          -DCMAKE_BUILD_TYPE=Release \
          -DWITH_LIBXML=OFF \
          -DBUILD_zlib=ON \
          -DWITH_XERCES=OFF \
          -DWITH_CHECK=OFF \
          -DCMAKE_PREFIX_PATH=em-dependencies
    cmake --build em-build-dependencies --config=Release
    cmake --install em-build-dependencies --config=Release
fi

# build COPASI SE if it doesn't exist
if [ ! -f em-dependencies/lib/libsbml-static.a ]; then
    
    emcmake cmake -G Ninja -B em-build-libsbml -S libsbml \
        -DCMAKE_BUILD_TYPE=Release \
        -DWITH_LIBXML=OFF \
        -DWITH_EXPAT=ON \
        -DWITH_CHECK=OFF \
        -DZLIB_LIBRARY=em-dependencies/lib/libzlib.a \
        -DZLIB_INCLUDE_DIR=em-dependencies/include \
        -DCMAKE_INSTALL_PREFIX=em-dependencies \
        -DLIBSBML_DEPENDENCY_DIR=em-dependencies \
        -DWITH_STABLE_PACKAGES=ON \
        -DCMAKE_PREFIX_PATH=em-dependencies
    
    cmake --build em-build-libsbml --config=Release
    cmake --install em-build-libsbml --config=Release
fi

popd

