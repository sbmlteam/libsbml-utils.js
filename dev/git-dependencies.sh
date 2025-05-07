#!/bin/bash
pushd "$(dirname "$0")"

LIBSBML_REPO="https://github.com/sbmlteam/libsbml-dependencies.git"
LIBSBML_DIR="libsbml-dependencies"

if [ ! -d "$LIBSBML_DIR" ]; then
    echo "Cloning libSBML dependencies repository..."
    git clone "$LIBSBML_REPO" "$LIBSBML_DIR"
else
    echo "Updating libSBML dependencies repository..."
    pushd "$LIBSBML_DIR"
    git pull origin master
    popd
fi

pushd "$LIBSBML_DIR"
git submodule update --init --remote expat
popd

echo "libSBML dependencies repository is up to date"
popd
