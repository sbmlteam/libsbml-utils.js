
INSTALL_PREFIX=dist-node

rm -rf $INSTALL_PREFIX

# execute the build script
./dev/em-build.sh

# copy test files
cp test/* $INSTALL_PREFIX

# run tests
cd $INSTALL_PREFIX
node test_node.js dimerization-invalid.xml

ls -l .