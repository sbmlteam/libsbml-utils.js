var utils = require('./sbml_utils.js');
var fs = require("fs");

var validationOptions = {
    "LIBSBML_CAT_GENERAL_CONSISTENCY": true,
    "LIBSBML_CAT_IDENTIFIER_CONSISTENCY": true,
    "LIBSBML_CAT_UNITS_CONSISTENCY": false,
    "LIBSBML_CAT_MATHML_CONSISTENCY": true,
    "LIBSBML_CAT_SBO_CONSISTENCY": true,
    "LIBSBML_CAT_OVERDETERMINED_MODEL": true,
    "LIBSBML_CAT_MODELING_PRACTICE": true,
    "LIBSBML_CAT_STRICT_UNITS_CONSISTENCY": false,
};


utils().then((Module) => {

    // instantiate validator
    var instance = Module;


    console.log(instance.getLibSBMLVersion());

    // load a local file using the filesystem 
    var data = fs.readFileSync(process.argv[2], 'utf8');

    // print validation errors
    console.log(instance.validateSBMLString(data, JSON.stringify(validationOptions)));

    // print available converters
    var converters = instance.getAvailableConverters();
    console.log("num available converters: ", converters.size());
    for (var i = 0; i < converters.size(); i++) {
        console.log(converters.get(i));
        console.log(instance.getConverterOptions(converters.get(i)));
        console.log("--------------------------------");
    }

    // get all options at once
    console.log(instance.getAllConvertersOptions());

    // now convert the model to SBML Level 3 Version 2
    console.log(instance.convertSBMLString(data, JSON.stringify({
        "options":[
            {"key":"setLevelAndVersion","value":"true"},
            {"key":"strict","value":"true"}
        ],
        "targetNamespaces":{"level":3,"version":2}
    })));

    // and l2v4
    console.log(instance.convertSBMLString(data, JSON.stringify({
        "options":[
            {"key":"setLevelAndVersion","value":"true"},
            {"key":"strict","value":"true"}
        ],
        "targetNamespaces":{"level":2,"version":4}
    })));
    
});