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
        {"main":{"description":"Convert the model to a given Level and Version of SBML","key":"setLevelAndVersion","type":"boolean","value":"true"},"options":[{"description":"Whether default units should be added when converting to L3","key":"addDefaultUnits","type":"boolean","value":"true"},{"description":"Whether validity should be strictly preserved","key":"strict","type":"boolean","value":"true"}],"targetNamespaces":{"level":3,"version":2}}
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

### Example Convertion

Running

```bash
node test_convertion.js dimerization-invalid.xml
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
      "description": "convert cobra sbml to fbc",
      "key": "convert cobra",
      "type": "boolean",
      "value": "true"
    },
    "options": [
      {
        "description": "checks level/version compatibility",
        "key": "checkCompatibility",
        "type": "boolean",
        "value": "false"
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
      "description": "flatten comp",
      "key": "flatten comp",
      "type": "boolean",
      "value": "true"
    },
    "options": [
      {
        "description": "specify whether to abort if any unflattenable packages are encountered",
        "key": "abortIfUnflattenable",
        "type": "string",
        "value": "requiredOnly"
      },
      {
        "description": "the base directory in which to search for external references",
        "key": "basePath",
        "type": "string",
        "value": "."
      },
      {
        "description": "any packages that cannot be flattened should be stripped (note: this option replaced by 'stripUnflattenablePackages').",
        "key": "ignorePackages",
        "type": "boolean",
        "value": "true"
      },
      {
        "description": "unused ports should be listed in the flattened model",
        "key": "leavePorts",
        "type": "boolean",
        "value": "false"
      },
      {
        "description": "all model definitions and external model definitions should remain in the SBMLDocument",
        "key": "listModelDefinitions",
        "type": "boolean",
        "value": "false"
      },
      {
        "description": "perform validation before and after trying to flatten",
        "key": "performValidation",
        "type": "boolean",
        "value": "true"
      },
      {
        "description": "comma separated list of packages to be stripped before flattening is attempted",
        "key": "stripPackages",
        "type": "string",
        "value": ""
      },
      {
        "description": "specify whether to strip any unflattenable packages ignored by 'abortIfUnflattenable'",
        "key": "stripUnflattenablePackages",
        "type": "boolean",
        "value": "true"
      }
    ]
  },
  "SBML Distributions Annotations Converter": {
    "main": {
      "description": "convert distrib to annotations",
      "key": "convert distrib to annotations",
      "type": "boolean",
      "value": "true"
    },
    "options": [
      {
        "description": "Use the UncertML namespace for the annotation instead of Wikipedia ones",
        "key": "useUncertMLNamespaces",
        "type": "boolean",
        "value": "false"
      },
      {
        "description": "Created functions return means of distributions instead of NaN",
        "key": "writeMeans",
        "type": "boolean",
        "value": "false"
      }
    ]
  },
  "SBML FBC to COBRA Converter": {
    "main": {
      "description": "convert FBC L3V1 to SBML L2V4 with COBRA annotation",
      "key": "convert fbc to cobra",
      "type": "boolean",
      "value": "true"
    },
    "options": [
      {
        "description": "write gene association into reaction notes, even if the reaction has notes already",
        "key": "overwriteReactionNotes",
        "type": "boolean",
        "value": "false"
      }
    ]
  },
  "SBML FBC v1 to FBC v2 Converter": {
    "main": {
      "description": "convert fbc v1 to fbc v2",
      "key": "convert fbc v1 to fbc v2",
      "type": "boolean",
      "value": "true"
    },
    "options": [
      {
        "description": "should the model be a strict one (i.e.: all non-specified bounds will be filled)",
        "key": "strict",
        "type": "boolean",
        "value": "true"
      }
    ]
  },
  "SBML FBC v2 to FBC v1 Converter": {
    "main": {
      "description": "convert fbc v2 to fbc v1",
      "key": "convert fbc v2 to fbc v1",
      "type": "boolean",
      "value": "true"
    },
    "options": []
  },
  "SBML Function Definition Converter": {
    "main": {
      "description": "Expand all function definitions in the model",
      "key": "expandFunctionDefinitions",
      "type": "boolean",
      "value": "true"
    },
    "options": [
      {
        "description": "Comma separated list of ids to skip during expansion",
        "key": "skipIds",
        "type": "string",
        "value": ""
      }
    ]
  },
  "SBML Id Converter": {
    "main": {
      "description": "Rename all SIds specified in the 'currentIds' option to the ones specified in 'newIds'",
      "key": "renameSIds",
      "type": "boolean",
      "value": "true"
    },
    "options": [
      {
        "description": "Comma separated list of ids to rename",
        "key": "currentIds",
        "type": "string",
        "value": ""
      },
      {
        "description": "Comma separated list of the new ids",
        "key": "newIds",
        "type": "string",
        "value": ""
      }
    ]
  },
  "SBML Infer Units Converter": {
    "main": {
      "description": "Infer the units of Parameters",
      "key": "inferUnits",
      "type": "boolean",
      "value": "true"
    },
    "options": []
  },
  "SBML Initial Assignment Converter": {
    "main": {
      "description": "Expand initial assignments in the model",
      "key": "expandInitialAssignments",
      "type": "boolean",
      "value": "true"
    },
    "options": []
  },
  "SBML Level 1 Version 1 Converter": {
    "main": {
      "description": "convert the document to SBML Level 1 Version 1",
      "key": "convertToL1V1",
      "type": "boolean",
      "value": "true"
    },
    "options": [
      {
        "description": "change pow expressions to the (^) hat notation",
        "key": "changePow",
        "type": "boolean",
        "value": "false"
      },
      {
        "description": "if true, occurrances of compartment ids in expressions will be replaced with their initial size",
        "key": "inlineCompartmentSizes",
        "type": "boolean",
        "value": "false"
      }
    ],
    "targetNamespaces": { "level": 1, "version": 1 }
  },
  "SBML Level Version Converter": {
    "main": {
      "description": "Convert the model to a given Level and Version of SBML",
      "key": "setLevelAndVersion",
      "type": "boolean",
      "value": "true"
    },
    "options": [
      {
        "description": "Whether default units should be added when converting to L3",
        "key": "addDefaultUnits",
        "type": "boolean",
        "value": "true"
      },
      {
        "description": "Whether validity should be strictly preserved",
        "key": "strict",
        "type": "boolean",
        "value": "true"
      }
    ],
    "targetNamespaces": { "level": 3, "version": 2 }
  },
  "SBML Local Parameter Converter": {
    "main": {
      "description": "Promotes all Local Parameters to Global ones",
      "key": "promoteLocalParameters",
      "type": "boolean",
      "value": "true"
    },
    "options": []
  },
  "SBML Rate Of Converter": {
    "main": {
      "description": "Replace rateOf with functionDefinition",
      "key": "replaceRateOf",
      "type": "boolean",
      "value": "true"
    },
    "options": [
      {
        "description": "create FunctionDefinition",
        "key": "toFunction",
        "type": "boolean",
        "value": "true"
      }
    ]
  },
  "SBML RateRule Converter": {
    "main": {
      "description": "Infer reactions from rateRules in the model",
      "key": "inferReactions",
      "type": "boolean",
      "value": "true"
    },
    "options": [
      {
        "description": "Whether the model should be checked for suitability first",
        "key": "performSanityCheck",
        "type": "boolean",
        "value": "true"
      },
      {
        "description": "If a number appears in the math use it as the stoichiometry",
        "key": "useStoichiometryFromMath",
        "type": "boolean",
        "value": "true"
      }
    ]
  },
  "SBML Reaction Converter": {
    "main": {
      "description": "Replace reactions with rateRules",
      "key": "replaceReactions",
      "type": "boolean",
      "value": "true"
    },
    "options": [
      {
        "description": "make any species into parameters",
        "key": "rateRuleVariablesShouldBeParameters",
        "type": "boolean",
        "value": "false"
      }
    ]
  },
  "SBML Rule Converter": {
    "main": {
      "description": "Sort AssignmentRules and InitialAssignments in the model",
      "key": "sortRules",
      "type": "boolean",
      "value": "true"
    },
    "options": []
  },
  "SBML Strip Package Converter": {
    "main": {
      "description": "Strip SBML Level 3 package constructs from the model",
      "key": "stripPackage",
      "type": "boolean",
      "value": "true"
    },
    "options": [
      {
        "description": "Name of the SBML Level 3 package to be stripped",
        "key": "package",
        "type": "string",
        "value": ""
      },
      {
        "description": "If set, all unsupported packages will be removed.",
        "key": "stripAllUnrecognized",
        "type": "boolean",
        "value": "false"
      }
    ]
  },
  "SBML Units Converter": {
    "main": {
      "description": "Convert units in the model to SI units",
      "key": "units",
      "type": "boolean",
      "value": "true"
    },
    "options": [
      {
        "description": "Whether unused UnitDefinition objects should be removed",
        "key": "removeUnusedUnits",
        "type": "boolean",
        "value": "true"
      }
    ]
  }
}

```

## Acknowledgements

This project has been possible thanks to [LiBiS](https://libis.bioquant.uni-heidelberg.de), the Baden-Württemberg Institute for Bioinformatics Infrastructure:

![LiBiS Logo](./docs/_static/libis-logo.svg)

and to the BMBF funded [de.NBI](https://www.denbi.de) initiative (031L0104A, W-de.NBI-016)):

![de.NBI logo](./docs/_static/deNBI_logo.jpg)

## License

BSD 2-Clause License see [LICENSE](./LICENSE)
