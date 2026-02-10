var validator = require('./sbml_validator.js');
var fs = require("fs");

validator().then((Module) => {

    // instantiate validator
    var instance = Module;

    // load a local file using the filesystem 
    var data = fs.readFileSync(process.argv[2], 'utf8');

    // print validation errors
    console.log(instance.validateSBMLString(data));

});