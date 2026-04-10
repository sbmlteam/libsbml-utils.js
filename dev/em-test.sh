
INSTALL_PREFIX=dist-node

rm -rf $INSTALL_PREFIX

# execute the build script
./dev/em-build.sh

# fail if the build script fails
if [ $? -ne 0 ]; then
    echo "Build failed. Aborting tests."
    exit 1
fi

# copy test files
cp test/* $INSTALL_PREFIX

# run tests
cd $INSTALL_PREFIX
node test_validation.js dimerization-invalid.xml

node test_conversion.js dimerization.xml

ls -l .