#!/bin/bash
pushd "$(dirname "$0")"

LIBSBML_REPO="https://github.com/sbmlteam/libsbml.git"
LIBSBML_DIR="libsbml"
LIBSBML_BRANCH="${LIBSBML_BRANCH:-development}"

if [ ! -d "$LIBSBML_DIR" ]; then
    echo "Cloning libSBML repository..."
    git clone -b $LIBSBML_BRANCH "$LIBSBML_REPO" "$LIBSBML_DIR"
else
    echo "Updating libSBML repository..."
    pushd "$LIBSBML_DIR"
    git pull origin $LIBSBML_BRANCH
    popd
fi

echo "libSBML repository is up to date"

# read version from ./libsbml/VERSION.txt
LIBSBML_VERSION=$(cat "$LIBSBML_DIR/VERSION.txt")
echo "libSBML version: $LIBSBML_VERSION"

popd
