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

#include <sbml_utils.h>

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;
LIBSBML_CPP_NAMESPACE_USE

namespace sbml_utils
{

// map of converters
static std::map<std::string, SBMLConverter *> converters;

static std::vector<ValidationError> conversionErrors;

static void freeConvertersMap()
{
    for (auto &converter : converters)
    {
        delete converter.second;
    }
    converters.clear();
}

static void initConvertersMap()
{
    freeConvertersMap();
    auto &instance = SBMLConverterRegistry::getInstance();
    for (int i = 0; i < instance.getNumConverters(); i++)
    {
        auto *converter = instance.getConverterByIndex(i);
        converters[converter->getName()] = converter;
    }
}

// Replace all occurrences of a substring in-place
static void replaceAll(std::string &str, const std::string &from, const std::string &to)
{
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Move past the replacement
    }
}

static std::string getSeverityString(unsigned int severity)
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

ValidationError::ValidationError(const LIBSBML_CPP_NAMESPACE_QUALIFIER SBMLError *err)
    : line(err->getLine()), message(err->getMessage()), severity(getSeverityString(err->getSeverity())), errorId(err->getErrorId())
      //, level(err->getLevel())
      //, version(err->getVersion())
      ,
      column(err->getColumn()), category(err->getCategoryAsString()), package(err->getPackage())
//, pkgVersion(err->getPgetPackageVersion())
{
    std::replace(message.begin(), message.end(), '\n', ' '); // Normalize line endings
    // replace all quotes in the message with escaped quotes
    replaceAll(message, "\"", "\\\""); // Replace double quotes with \"
    // ensure we have no \\\" (double-escaped) sequences in the message
    replaceAll(message, "\\\\\"", "\\\""); // Replace \\\" with \\"

    // ensure we have a package name
    if (package.empty())
    {
        package = "core";
    }
}

ValidationError::ValidationError(unsigned int line, const std::string &message, const std::string &severity)
    : line(line), message(message), severity(severity)
{
    std::replace(this->message.begin(), this->message.end(), '\n', ' '); // Normalize line endings
}

std::string ValidationError::toJsonString() const
{
    return std::string("{ ") +
           "\"line\": " + std::to_string(line) +
           ", \"column\": " + std::to_string(column) +
           ", \"message\": \"" + message + "\"" +
           ", \"severity\": \"" + severity + "\"" +
           ", \"category\": \"" + category + "\"" +
           ", \"errorId\": " + std::to_string(errorId) +
           //", \"level\": " + std::to_string(level) +
           //", \"version\": " + std::to_string(version) +
           ", \"package\": \"" + package + "\"" +
           //", \"pkgVersion\": " + std::to_string(pkgVersion) +
           "}";
}

static std::string getErrorsAsJSON(const std::vector<ValidationError> &errors)
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
static void printErrorsAsJSON(const std::vector<ValidationError> &errors)
{
    std::cout << getErrorsAsJSON(errors);
}

// Reads an entire file into a string
static std::string readFileToString(const std::string &filePath)
{
    std::ifstream t(filePath);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

static void applyValidationOptions(SBMLDocument *document, const std::string &validationOptions)
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

static void addErrorsToVector(std::vector<ValidationError> &errors, const SBMLDocument *document, unsigned int severityFilter)
{
    for (unsigned int i = 0; i < document->getNumErrors(); ++i)
    {
        const SBMLError *err = document->getError(i);
        auto severityLevel = err->getSeverity();
        if (severityLevel < severityFilter)
            continue;

        errors.push_back(ValidationError(err));
    }
}

static std::string validateSBMLString(const std::string &sbmlContent, const std::string &validationOptions)
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
            addErrorsToVector(errors, document);
        }

        unsigned int consistencyErrors = document->checkConsistency();
        if (consistencyErrors > 0)
        {
            addErrorsToVector(errors, document, LIBSBML_SEV_WARNING);
        }
    }

    // printErrorsAsJSON(errors);
    delete document;

    return getErrorsAsJSON(errors);
}

static std::string getVersionString()
{
    return getLibSBMLDottedVersion();
}

static std::vector<std::string> getAvailableConverters()
{
    if (converters.empty())
    {
        initConvertersMap();
    }

    std::vector<std::string> converterNames;
    for (auto &converter : converters)
    {
        converterNames.push_back(converter.first);
    }
    return converterNames;
}

static nlohmann::json optionToObject(const ConversionOption *option)
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

static void fillConverterOptions(const std::string &converterName, nlohmann::json &options)
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

    const auto &props = converter->getDefaultProperties();

    if (props.getNumOptions() == 0)
    {
        return;
    }

    // extract main option
    auto *mainOption = props.getOption(converter->getMainOption());
    if (mainOption)
    {
        options["main"] = optionToObject(mainOption);
    }
    

    // add target namespaces if needed
    auto *targetNamespaces = props.getTargetNamespaces();

    if (targetNamespaces)
    {
        options["targetNamespaces"]["level"] = targetNamespaces->getLevel();
        options["targetNamespaces"]["version"] = targetNamespaces->getVersion();
    }

    // remaining options
    auto optionsArray = nlohmann::json::array();

    for (int i = 0; i < props.getNumOptions(); i++)
    {
        auto *option = props.getOption(i);
        // leave main options out from the list
        if (!option || option->getKey() == converter->getMainOption())
            continue;

        optionsArray.push_back(optionToObject(option));
    }

    options["options"] = optionsArray;
}

static std::string getConverterOptions(const std::string &converterName)
{
    nlohmann::json options;
    fillConverterOptions(converterName, options);
    return options.dump();
}

static std::string getAllConvertersOptions()
{
    if (converters.empty())
    {
        initConvertersMap();
    }
    nlohmann::json options;
    for (auto &converter : converters)
    {
        options[converter.first] = nlohmann::json::object();
        fillConverterOptions(converter.first, options[converter.first]);
    }
    return options.dump();
}

static void fillConversionProperties(ConversionProperties &properties, const nlohmann::json &optionsObject)
{
    try
    {

        // iterate over the options object (values alone have no .key(); use .items())
        for (auto &option : optionsObject.items())
        {
            if (option.key() == "options")
            {
                for (auto &_option : option.value())
                {
                    properties.addOption(_option["key"], _option["value"].get<std::string>());
                }
            }
            else if (option.key() == "targetNamespaces")
            {
                const auto &tn = option.value();
                auto ns = SBMLNamespaces(tn["level"].get<int>(), tn["version"].get<int>());
                properties.setTargetNamespaces(&ns);
            }
            else
            {
                // assume basic option
                properties.addOption(option.key(), option.value().get<std::string>());
            }
        }
        
    }
    catch (const std::exception &e)
    {
        conversionErrors.push_back({0, "Invalid conversion options: " + std::string(e.what()), "fatal"});
    }
}

static std::string convertSBMLString(const std::string &sbmlContent, const std::string &options)
{
    conversionErrors.clear();

    nlohmann::json optionsObject = nlohmann::json::parse(options, nullptr, false);
    SBMLDocument *document = readSBMLFromString(sbmlContent.c_str());
    document->getErrorLog()->clearLog();

    ConversionProperties properties;
    fillConversionProperties(properties, optionsObject);

    if (document->convert(properties) != LIBSBML_OPERATION_SUCCESS)
    {
        addErrorsToVector(conversionErrors, document);

        if (conversionErrors.empty())
        {
            conversionErrors.push_back({0, "Conversion failed.", "fatal"});
        }
    }

    char *sbmlString = document->toSBML();
    std::string sbmlStringStr(sbmlString);
    free(sbmlString);
    return sbmlStringStr;
}

static std::string getConversionErrors()
{
    return getErrorsAsJSON(conversionErrors);
}

} // namespace sbml_utils

EMSCRIPTEN_BINDINGS(sbml_validator)
{
    sbml_utils::initConvertersMap();
    emscripten::register_vector<std::string>("StringVector");
    emscripten::function("validateSBMLString", &sbml_utils::validateSBMLString);
    emscripten::function("getLibSBMLVersion", &sbml_utils::getVersionString);
    emscripten::function("getAvailableConverters", &sbml_utils::getAvailableConverters);
    emscripten::function("getConverterOptions", &sbml_utils::getConverterOptions);
    emscripten::function("getAllConvertersOptions", &sbml_utils::getAllConvertersOptions);
    emscripten::function("convertSBMLString", &sbml_utils::convertSBMLString);
    emscripten::function("getConversionErrors", &sbml_utils::getConversionErrors);
    emscripten::function("freeConvertersMap", &sbml_utils::freeConvertersMap);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: sbml-validator <file.sbml>\n";
        return 1;
    }

    std::string sbmlContent = sbml_utils::readFileToString(argv[1]);
    std::string errors = sbml_utils::validateSBMLString(sbmlContent);
    std::cout << errors;

    return 0;
}
