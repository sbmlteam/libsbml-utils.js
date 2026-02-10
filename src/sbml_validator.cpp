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

// Struct to store validation error info
struct ValidationError
{
    unsigned int line;
    std::string message;
    std::string severity;
};

std::string getErrorsAsJSON(const std::vector<ValidationError> &errors)
{
    std::stringstream str;
    str << "[\n";
    for (size_t i = 0; i < errors.size(); ++i)
    {
        std::string message = errors[i].message;
        std::replace(message.begin(), message.end(), '\n', ' '); // Normalize line endings
        str << "  { \"line\": " << errors[i].line
            << ", \"message\": \"" << message << "\""
            << ", \"severity\": \"" << errors[i].severity << "\" }";
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
                const SBMLError *err = document->getError(i);
                std::string severity;
                switch (err->getSeverity())
                {
                case LIBSBML_SEV_ERROR:
                    severity = "error";
                    break;
                case LIBSBML_SEV_FATAL:
                    severity = "fatal";
                    break;
                case LIBSBML_SEV_WARNING:
                    severity = "warning";
                    break;
                default:
                    severity = "info";
                    break;
                }
                errors.push_back({err->getLine(), err->getMessage(), severity});
            }
        }

        unsigned int consistencyErrors = document->checkConsistency();
        if (consistencyErrors > 0)
        {
            for (unsigned int i = 0; i < document->getNumErrors(); ++i)
            {
                const SBMLError *err = document->getError(i);
                if (err->getSeverity() == LIBSBML_SEV_WARNING || err->getSeverity() == LIBSBML_SEV_ERROR)
                {
                    std::string severity;
                    switch (err->getSeverity())
                    {
                    case LIBSBML_SEV_ERROR:
                        severity = "error";
                        break;
                    case LIBSBML_SEV_FATAL:
                        severity = "fatal";
                        break;
                    case LIBSBML_SEV_WARNING:
                        severity = "warning";
                        break;
                    default:
                        severity = "info";
                        break;
                    }
                    errors.push_back({err->getLine(), err->getMessage(), severity});
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
