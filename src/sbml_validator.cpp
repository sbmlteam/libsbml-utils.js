#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <sbml/SBMLTypes.h>
#include <algorithm> // For std::replace
#include <emscripten.h>
#include <emscripten/bind.h>

#include <json.hpp>

using namespace emscripten;
LIBSBML_CPP_NAMESPACE_USE

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

EMSCRIPTEN_BINDINGS(sbml_validator)
{
    emscripten::function("validateSBMLString", &validateSBMLString);
    emscripten::function("getLibSBMLVersion", &getVersionString);
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
