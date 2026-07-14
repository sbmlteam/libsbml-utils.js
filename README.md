## libSBML JS Utils

This is a C++ project exposes libSBML functionality to JavaScript. Starting with the validation API as well as the conversion API. It uses emscripten to compile the libsbml library and its dependencies, with just a couple of functions being exposed.

### Building

Its all encapsulated in scripts

```bash
./dev/git-dependencies.sh  # clones updates the dependencies
./dev/git-libsbml.sh       # clones / updates libsbml
```

for building the node.js version

```bash
./dev/em-dependencies.sh    # builds the dependencies if not done
./dev/em-build.sh           # build the code from ./src/
./dev/em-test.sh            # runs the test_node after copying to install dir
```

to build the version to be embedded into web sites use

```bash
./dev/em-web.sh
```

### The API

The following JavaScript methods are available:

- **validateSBMLString(content, validationOptions)**  
  Validates an SBML string with optional validation options (as JSON). Returns validation errors as a JSON list.
  validation options are:

```js
  {
    "LIBSBML_CAT_GENERAL_CONSISTENCY": true,
    "LIBSBML_CAT_IDENTIFIER_CONSISTENCY": true,
    "LIBSBML_CAT_UNITS_CONSISTENCY": false,
    "LIBSBML_CAT_MATHML_CONSISTENCY": true,
    "LIBSBML_CAT_SBO_CONSISTENCY": true,
    "LIBSBML_CAT_OVERDETERMINED_MODEL": true,
    "LIBSBML_CAT_MODELING_PRACTICE": true,
    "LIBSBML_CAT_STRICT_UNITS_CONSISTENCY": false,
  }
```

- **getLibSBMLVersion()**  
  Returns the linked libSBML version as a dotted string.

- **getAvailableConverters()**  
  Returns a list of the names of all available SBML converters.

- **getConverterOptions(converterName)**  
  Returns a JSON object describing the options accepted by a specific converter.
  Valid names are the ones returned by `getAvailableConverters`

- **getAllConvertersOptions()**  
  Returns a JSON object listing all converters and their available options.

- **convertSBMLString(content, conversionOptions)**  
  Converts the supplied SBML string using the provided options (as JSON). Returns the converted SBML as a string.

  Valid options are the one returned. For example for the level / version converter:

```javascript
    {
        "options":[
            {"key":"setLevelAndVersion","value":"true"},
            {"key":"strict","value":"true"}
        ],
        "targetNamespaces":{"level":3,"version":2}
    }
```

but a simplified format is also supported, the simplified format treats
`options` and `targetNamespaces` as special symbols all other options
will be passed along:

```javascript
  {
      "setLevelAndVersion":"true",
      "strict":"false",
      "targetNamespaces":{"level":1,"version":2}
  }
```

- **getConversionErrors()**  
  Returns any errors collected during the last conversion call, as a JSON list. Will be
  cleared at the start of the conversion.

- **freeConvertersMap()**  
  Frees memory allocated for the internal converter registry. Should be called
  to clear the static map of converters whenever you are done with them.

### Example Validation

Running

```bash
node test_validation.js dimerization-invalid.xml
```

produces a json list like:

```json
[
  {
    "line": 3,
    "column": 4,
    "message": "A Model object may only have the following attributes, all of which are optional: 'metaid', 'sboTerm', 'id', 'name', 'substanceUnits', 'timeUnits', 'volumeUnits', 'areaUnits', 'lengthUnits', 'extentUnits' and 'conversionFactor'. No other attributes from the SBML Level 3 Core namespace are permitted on a Model object. Reference: L3V1 Section 4.2  Attribute 'invalid' is not part of the definition of an SBML Level 3 Version 1 <model> element. ",
    "severity": "error",
    "category": "SBML component consistency",
    "errorId": 20222,
    "package": "core"
  }
]
```

### Example conversion

Running

```
node test_conversion.js dimerization.xml
```

prints several conversions to different SBML Levels and Versions. As well
as the list of conversion errors.

To get the conversion options:

```javascript
// either get individual converters
var converters = instance.getAvailableConverters();
for (var i = 0; i < converters.size(); i++) {
  console.log(converters.get(i));
  // and their options
  console.log(instance.getConverterOptions(converters.get(i)));
}
```

which produces an output like:

```json
SBML Function Definition Converter

{"main":{"description":"Expand all function definitions in the model","key":"expandFunctionDefinitions","type":"boolean","value":"true"},"options":[{"description":"Comma separated list of ids to skip during expansion","key":"skipIds","type":"string","value":""}]}
```

or

```javascript
console.log(instance.getAllConvertersOptions());
```

which produces a complete list of all converters and their options:

```json
{
    "Layout Converter L2 <=> L3": {
        "main": {
            "description": "convert the layout to the given namespaces",
            "key": "convert layout",
            "type": "boolean",
            "value": "true"
        },
        "options": []
    },
    "SBML COBRA to FBC Converter": {
        "main": {
            "description": "checks level/version compatibility",
            "key": "checkCompatibility",
            "type": "boolean",
            "value": "false"
        },
        "options": [
            {
                "description": "convert cobra sbml to fbc",
                "key": "convert cobra",
                "type": "boolean",
                "value": "true"
            },
            {
                "description": "removes unit definitions",
                "key": "removeUnits",
                "type": "boolean",
                "value": "false"
            }
        ]
    },
    "SBML Comp Flattening Converter": {
        "main": {
            "description": "specify whether to abort if any unflattenable packages are encountered",
            "key": "abortIfUnflattenable",
            "type": "string",
            "value": "requiredOnly"
        },
        "options": [
            {
                "description": "the base directory in which to search for external references",
                "key": "basePath",
                "type": "string",
                "value": "."
            }
        ]
    },
    ...
}
```

## Acknowledgements

This project has been possible thanks to [LiBiS](https://libis.bioquant.uni-heidelberg.de), the Baden-Württemberg Institute for Bioinformatics Infrastructure:

![LiBiS Logo](./docs/_static/libis-logo.svg)

and to the BMBF funded [de.NBI](https://www.denbi.de) initiative (031L0104A, W-de.NBI-016)):

![de.NBI logo](./docs/_static/deNBI_logo.jpg)

## License

BSD 2-Clause License see [LICENSE](./LICENSE)
