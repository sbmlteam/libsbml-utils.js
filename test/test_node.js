var validator = require('./sbml_validator.js');
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


validator().then((Module) => {

    // instantiate validator
    var instance = Module;


    console.log(instance.getLibSBMLVersion());

    // load a local file using the filesystem 
    var data = fs.readFileSync(process.argv[2], 'utf8');

    // print validation errors
    console.log(instance.validateSBMLString(data, JSON.stringify(validationOptions)));

});