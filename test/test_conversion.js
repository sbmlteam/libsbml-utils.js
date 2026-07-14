var utils = require('./sbml_utils.js');
var fs = require("fs");

utils().then((Module) => {

    // instantiate validator
    var instance = Module;


    console.log(instance.getLibSBMLVersion());

    // load a local file using the filesystem 
    var data = fs.readFileSync(process.argv[2], 'utf8');

    // either get individual converters
    var converters = instance.getAvailableConverters();
    for (var i = 0; i < converters.size(); i++)
    {
        console.log(converters.get(i));
        // and their options
        console.log(instance.getConverterOptions(converters.get(i)));
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

    console.log(instance.getConversionErrors());

    // and l2v4
    console.log(instance.convertSBMLString(data, JSON.stringify({
        "options":[
            {"key":"setLevelAndVersion","value":"true"},
            {"key":"strict","value":"true"}
        ],
        "targetNamespaces":{"level":2,"version":4}
    })));

    console.log(instance.getConversionErrors());

    // a conversion with simplified options
    console.log(instance.convertSBMLString(data, JSON.stringify({
        "setLevelAndVersion":"true",
        "strict":"false",
        "targetNamespaces":{"level":1,"version":2}
    })));

    console.log(instance.getConversionErrors());
    

    // and an invalid conversion
    console.log(instance.convertSBMLString(data, JSON.stringify(
        ["something","invalid"]
    )));

    console.log(instance.getConversionErrors());
});
