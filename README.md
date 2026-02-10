## libsbml JS Validator
This is a small C++ project just exposing the libSBML validation API as JavaScript. It uses emscripten to compile the libsbml library and its dependencies, with just a couple of functions being exposed. 

### Building

Its all encapsulated in scripts

```bash
./dev/git-dependencies.sh. # clones updates the dependencies
./dev/git-libsbml.sh.      # clones / updates libsbml 
```

for building the node.js version

```bash
./dev/em-dependencies.sh.   # builds the dependencies if not done
./dev/em-build.sh           # build the code from ./src/
./dev/em-test.sh            # runs the test_node after copying to install dir
```

to build the version to be embedded into web sites use

```bash
./dev/em-web.sh
```

### Example
Running 

```bash
node test_node.js dimerization-invalid.xml
```

produces a json list like: 

```json
[
  { "line": 3, 
    "message": "A Model object may only have the following attributes, all of which are optional: 'metaid', 'sboTerm', 'id', 'name', 'substanceUnits', 'timeUnits', 'volumeUnits', 'areaUnits', 'lengthUnits', 'extentUnits' and 'conversionFactor'. No other attributes from the SBML Level 3 Core namespace are permitted on a Model object. Reference: L3V1 Section 4.2  Attribute 'invalid' is not part of the definition of an SBML Level 3 Version 1 <model> element. ", 
    "severity": "error" 
  }
]
```
