#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <sbml/SBMLTypes.h>
#include <algorithm> // For std::replace

#include <sbml/conversion/SBMLConverterRegistry.h>
#include <sbml/conversion/ConversionProperties.h>
#include <sbml/conversion/ConversionOption.h>


#include <emscripten.h>
#include <emscripten/bind.h>

#include <json.hpp>

using namespace emscripten;
LIBSBML_CPP_NAMESPACE_USE

// map of converters
std::map<std::string, SBMLConverter *> converters;


void freeConvertersMap()
{
    for (auto &converter : converters)
    {
        delete converter.second;
    }
    converters.clear();
}

void initConvertersMap()
{
    freeConvertersMap();
    auto& instance = SBMLConverterRegistry::getInstance();
    for (int i = 0; i < instance.getNumConverters(); i++)
    {
        auto *converter = instance.getConverterByIndex(i);
        converters[converter->getName()] = converter;
    }
}


// Replace all occurrences of a substring in-place
static void replaceAll(std::string &str, const std::string &from, const std::string &to)
{
    if (from.empty()) return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Move past the replacement
    }
}

std::string getSeverityString(unsigned int severity)
{
    switch (severity)
    {
        case LIBSBML_SEV_ERROR:
            return "error";
        case LIBSBML_SEV_WARNING:
            return "warning";
        case LIBSBML_SEV_INFO:
            return "info";
        default:
            return "";
    }
}

// Struct to store validation error info
struct ValidationError
{
    ValidationError(const SBMLError *err)
        : line(err->getLine())
        , message(err->getMessage())
        , severity(getSeverityString(err->getSeverity()))
        , errorId(err->getErrorId())
        //, level(err->getLevel())
        //, version(err->getVersion())
        , column(err->getColumn())
        , category(err->getCategoryAsString())
        , package(err->getPackage())
        //, pkgVersion(err->getPgetPackageVersion())
        {
            std::replace(message.begin(), message.end(), '\n', ' '); // Normalize line endings
            // replace all quotes in the message with escaped quotes
            replaceAll(message, "\"", "\\\""); // Replace double quotes with \"
            // ensure we have no \\\" (double-escaped) sequences in the message
            replaceAll(message, "\\\\\"", "\\\""); // Replace \\\" with \\"

            // ensure we have a package name
            if (package.empty()) {
                package = "core";
            }
        }

    ValidationError(unsigned int line, const std::string &message, const std::string &severity)
        : line(line), message(message), severity(severity)
    {
        std::replace(this->message.begin(), this->message.end(), '\n', ' '); // Normalize line endings
    }

    std::string toJsonString() const
    {
        return std::string("{ ") +
                "\"line\": " + std::to_string(line) +
                ", \"column\": " + std::to_string(column) +
                ", \"message\": \"" + message + "\"" +
                ", \"severity\": \"" + severity + "\""+
                ", \"category\": \"" + category + "\"" +
                ", \"errorId\": " + std::to_string(errorId) +
                //", \"level\": " + std::to_string(level) +
                //", \"version\": " + std::to_string(version) +
                ", \"package\": \"" + package + "\"" +
                //", \"pkgVersion\": " + std::to_string(pkgVersion) +
                "}";
    }

    unsigned int line;
    std::string message;
    std::string severity;
    unsigned int errorId  = 0;
    //unsigned int level;
    //unsigned int version;
    unsigned int column   = 0;
    std::string category;
    std::string package  = "core";
    //unsigned int pkgVersion = 1;
};

std::string getErrorsAsJSON(const std::vector<ValidationError> &errors)
{
    std::stringstream str;
    str << "[\n";
    for (size_t i = 0; i < errors.size(); ++i)
    {
        str << "  " << errors[i].toJsonString();
        if (i < errors.size() - 1)
            str << ",";
        str << "\n";
    }
    str << "]\n";
    return str.str();
}

// Converts validation errors to JSON
void printErrorsAsJSON(const std::vector<ValidationError> &errors)
{
    std::cout << getErrorsAsJSON(errors);
}

// Reads an entire file into a string
std::string readFileToString(const std::string &filePath)
{
    std::ifstream t(filePath);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

void applyValidationOptions(SBMLDocument *document, const std::string &validationOptions)
{
    if (validationOptions.empty())
    {
        document->setApplicableValidators(AllChecksON);
        return;
    }
    nlohmann::json option = nlohmann::json::parse(validationOptions);

    unsigned char applicableValidators = AllChecksON;

    if (option.find("LIBSBML_CAT_GENERAL_CONSISTENCY") != option.end())
    {
        if (option["LIBSBML_CAT_GENERAL_CONSISTENCY"].get<bool>())
        {
            applicableValidators |= SBMLCheckON;
        }
        else
        {
            applicableValidators &= SBMLCheckOFF;
        }
    }

    if (option.find("LIBSBML_CAT_IDENTIFIER_CONSISTENCY") != option.end())
    {
        if (option["LIBSBML_CAT_IDENTIFIER_CONSISTENCY"].get<bool>())
        {
            applicableValidators |= IdCheckON;
        }
        else
        {
            applicableValidators &= IdCheckOFF;
        }
    }

    if (option.find("LIBSBML_CAT_UNITS_CONSISTENCY") != option.end())
    {
        if (option["LIBSBML_CAT_UNITS_CONSISTENCY"].get<bool>())
        {
            applicableValidators |= UnitsCheckON;
        }
        else
        {
            applicableValidators &= UnitsCheckOFF;
        }
    }

    if (option.find("LIBSBML_CAT_MATHML_CONSISTENCY") != option.end())
    {
        if (option["LIBSBML_CAT_MATHML_CONSISTENCY"].get<bool>())
        {
            applicableValidators |= MathCheckON;
        }
        else
        {
            applicableValidators &= MathCheckOFF;
        }
    }

    if (option.find("LIBSBML_CAT_SBO_CONSISTENCY") != option.end())
    {
        if (option["LIBSBML_CAT_SBO_CONSISTENCY"].get<bool>())
        {
            applicableValidators |= SBOCheckON;
        }
        else
        {
            applicableValidators &= SBOCheckOFF;
        }
    }

    if (option.find("LIBSBML_CAT_OVERDETERMINED_MODEL") != option.end())
    {
        if (option["LIBSBML_CAT_OVERDETERMINED_MODEL"].get<bool>())
        {
            applicableValidators |= OverdeterCheckON;
        }
        else
        {
            applicableValidators &= OverdeterCheckOFF;
        }
    }

    if (option.find("LIBSBML_CAT_MODELING_PRACTICE") != option.end())
    {
        if (option["LIBSBML_CAT_MODELING_PRACTICE"].get<bool>())
        {
            applicableValidators |= PracticeCheckON;
        }
        else
        {
            applicableValidators &= PracticeCheckOFF;
        }
    }

    if (option.find("LIBSBML_CAT_STRICT_UNITS_CONSISTENCY") != option.end())
    {
        if (option["LIBSBML_CAT_STRICT_UNITS_CONSISTENCY"].get<bool>())
        {
            applicableValidators |= StrictUnitsCheckON;
        }
        else
        {
            applicableValidators &= StrictUnitsCheckOFF;
        }
    }

    document->setApplicableValidators(applicableValidators);
}

std::string validateSBMLString(const std::string &sbmlContent, const std::string &validationOptions = "{}")
{

    SBMLDocument *document = readSBMLFromString(sbmlContent.c_str());
    applyValidationOptions(document, validationOptions);

    std::vector<ValidationError> errors;

    if (!document)
    {
        errors.push_back({0, "Failed to parse SBML file.", "fatal"});
    }
    else
    {
        if (document->getNumErrors() > 0)
        {
            for (unsigned int i = 0; i < document->getNumErrors(); ++i)
            {
                errors.push_back(ValidationError(document->getError(i)));
            }
        }

        unsigned int consistencyErrors = document->checkConsistency();
        if (consistencyErrors > 0)
        {
            for (unsigned int i = 0; i < document->getNumErrors(); ++i)
            {
                const SBMLError *err = document->getError(i);
                auto severityLevel = err->getSeverity();
                if (severityLevel == LIBSBML_SEV_WARNING || severityLevel == LIBSBML_SEV_ERROR)
                {                    
                    errors.push_back(ValidationError(document->getError(i)));
                }
            }
        }
    }

    // printErrorsAsJSON(errors);
    delete document;

    return getErrorsAsJSON(errors);
}

std::string getVersionString()
{
    return getLibSBMLDottedVersion();
}

std::vector< std::string > getAvailableConverters()
{
    if (converters.empty())
    {
        initConvertersMap();
    }

    std::vector< std::string > converterNames;
    for (auto &converter : converters)
    {
        converterNames.push_back(converter.first);
    }
    return converterNames;
}

nlohmann::json optionToObject(const ConversionOption *option)
{
    auto optionObject = nlohmann::json::object();
    optionObject["key"] = option->getKey();
    optionObject["value"] = option->getValue();
    optionObject["description"] = option->getDescription();
    switch (option->getType())
        {
            case ConversionOptionType_t::CNV_TYPE_BOOL:
                optionObject["type"] = "boolean";
                break;
            case ConversionOptionType_t::CNV_TYPE_DOUBLE:
                optionObject["type"] = "number";
                break;
            case ConversionOptionType_t::CNV_TYPE_INT:
                optionObject["type"] = "integer";
                break;
            case ConversionOptionType_t::CNV_TYPE_SINGLE:
                optionObject["type"] = "number";
                break;
            case ConversionOptionType_t::CNV_TYPE_STRING:
                optionObject["type"] = "string";
                break;
        }
    return optionObject;
}

void fillConverterOptions(const std::string &converterName, nlohmann::json &options)
{
    if (converters.empty())
    {
        initConvertersMap();
    }
    
    auto *converter = converters[converterName];
    if (!converter)
    {
        return;
    }

    const auto& props = converter->getDefaultProperties();

    if (props.getNumOptions() == 0)
    {
        return;
    }


    // extract main option 
    options["main"] = optionToObject(props.getOption(0));
    
    
    // add target namespaces if needed
    auto *targetNamespaces = props.getTargetNamespaces();

    if (targetNamespaces)
    {
        options["targetNamespaces"]["level"] = targetNamespaces->getLevel();
        options["targetNamespaces"]["version"] = targetNamespaces->getVersion();
    }


    // remaining options
    auto optionsArray = nlohmann::json::array();

    for (int i = 1; i < props.getNumOptions(); i++)
    {
        auto* option = props.getOption(i);
        if (!option) 
            continue;
        
        optionsArray.push_back(optionToObject(option));
    }

    options["options"] = optionsArray;

}

std::string getConverterOptions(const std::string &converterName)
{
    nlohmann::json options;
    fillConverterOptions(converterName, options);
    return options.dump();
}

std::string getAllConvertersOptions()
{
    nlohmann::json options;
    for (auto &converter : converters)
    {
        options[converter.first] = nlohmann::json::object();
        fillConverterOptions(converter.first, options[converter.first]);
    }
    return options.dump();
}

void fillConversionProperties(ConversionProperties &properties, const nlohmann::json &optionsObject)
{
    for (auto &option : optionsObject["options"])
    {
        properties.addOption(option["key"], option["value"].get<std::string>());
    }

    if (optionsObject.find("targetNamespaces") != optionsObject.end())
    {
        auto ns = SBMLNamespaces(optionsObject["targetNamespaces"]["level"].get<int>(), optionsObject["targetNamespaces"]["version"].get<int>());
        properties.setTargetNamespaces(&ns);
    }
}

std::string convertSBMLString(const std::string &sbmlContent, const std::string &options)
{
    nlohmann::json optionsObject = nlohmann::json::parse(options);
    ConversionProperties properties;
    fillConversionProperties(properties, optionsObject);
    SBMLDocument *document = readSBMLFromString(sbmlContent.c_str());
    document->convert(properties);    
    char* sbmlString = document->toSBML();
    std::string sbmlStringStr(sbmlString);
    free(sbmlString);
    return sbmlStringStr;

}

EMSCRIPTEN_BINDINGS(sbml_validator)
{
    initConvertersMap();
    emscripten::register_vector<std::string>("StringVector");
    emscripten::function("validateSBMLString", &validateSBMLString);
    emscripten::function("getLibSBMLVersion", &getVersionString);
    emscripten::function("getAvailableConverters", &getAvailableConverters);
    emscripten::function("getConverterOptions", &getConverterOptions);
    emscripten::function("getAllConvertersOptions", &getAllConvertersOptions);
    emscripten::function("convertSBMLString", &convertSBMLString);
    emscripten::function("freeConvertersMap", &freeConvertersMap);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: sbml-validator <file.sbml>\n";
        return 1;
    }

    std::string sbmlContent = readFileToString(argv[1]);
    std::string errors = validateSBMLString(sbmlContent);
    std::cout << errors;

    return 0;
}
