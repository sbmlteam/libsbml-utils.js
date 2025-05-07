#!/bin/bash
pushd "$(dirname "$0")"

LIBSBML_REPO="https://github.com/sbmlteam/libsbml.git"
LIBSBML_DIR="libsbml"

if [ ! -d "$LIBSBML_DIR" ]; then
    echo "Cloning libSBML repository..."
    git clone "$LIBSBML_REPO" "$LIBSBML_DIR"
else
    echo "Updating libSBML repository..."
    pushd "$LIBSBML_DIR"
    git pull origin master
    popd
fi

echo "libSBML repository is up to date"
popd
