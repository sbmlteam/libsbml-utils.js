#ifndef SBML_UTILS_H
#define SBML_UTILS_H

#include <string>
#include <vector>

#include <sbml/SBMLDocument.h>
#include <sbml/conversion/ConversionProperties.h>

#include <json.hpp>

namespace sbml_utils
{

struct ValidationError
{
    unsigned int line;
    std::string message;
    std::string severity;
    unsigned int errorId  = 0;
    unsigned int column   = 0;
    std::string category;
    std::string package  = "core";

    ValidationError(const LIBSBML_CPP_NAMESPACE_QUALIFIER SBMLError *err);
    ValidationError(unsigned int line, const std::string &message, const std::string &severity);
    std::string toJsonString() const;
};

/**
 * @brief Validates an SBML string and returns a JSON string containing the validation errors.
 * 
 * @param sbmlContent The SBML string to validate.
 * @param validationOptions The validation options to use as json object with the following keys: 
 * {
 *      "LIBSBML_CAT_GENERAL_CONSISTENCY": true,
 *      "LIBSBML_CAT_IDENTIFIER_CONSISTENCY": true,
 *      "LIBSBML_CAT_UNITS_CONSISTENCY": false,
 *      "LIBSBML_CAT_MATHML_CONSISTENCY": true,
 *      "LIBSBML_CAT_SBO_CONSISTENCY": true,
 *      "LIBSBML_CAT_OVERDETERMINED_MODEL": true,
 *      "LIBSBML_CAT_MODELING_PRACTICE": true,
 *      "LIBSBML_CAT_STRICT_UNITS_CONSISTENCY": true,
 *      "LIBSBML_CAT_OVERDETERMINED_MODEL": true,
 * }
 * 
 * 
 * @return A JSON string containing the validation errors.
 */
static std::string validateSBMLString(const std::string& sbmlContent, const std::string& validationOptions = "");


/**
 * @brief Get the LibSBML Version string
 * 
 * @return The version string of the libSBML library.
 */
static std::string getVersionString();


/**
 * @brief Get the available converter names
 * 
 * @return A vector of strings containing the available converters.
 */
static std::vector< std::string > getAvailableConverters();

/**
 * @brief Get the conversion errors as a JSON string
 * 
 * @return A JSON string containing the conversion errors.
 */
static std::string getConversionErrors();

/**
 * @brief Free the memory allocated for the converters map
 * and clear the conversion errors vector
 */
static void freeConvertersMap();


/**
 * @brief Replace all occurrences of a substring in a string
 * 
 * @param str The string to replace the substring in.
 * @param from The substring to replace.
 * @param to The substring to replace with.
 */
static void replaceAll(std::string &str, const std::string &from, const std::string &to);

/**
 * @brief Convert validation errors to a JSON string
 * 
 * @param errors The validation errors to convert.
 * @return The JSON string containing the validation errors.
 */
static std::string getErrorsAsJSON(const std::vector<ValidationError> &errors);


/**
 * @brief Print validation errors to a JSON string
 * 
 * @param errors The validation errors to print.
 */
static void printErrorsAsJSON(const std::vector<ValidationError> &errors);


/**
 * @brief Read a file into a string
 * 
 * @param filePath The path to the file to read.
 * @return The string containing the file content.
 */
static std::string readFileToString(const std::string &filePath);


/**
 * @brief Apply validation options to an SBML document
 * 
 * @param document The SBML document to apply the validation options to.
 * @param validationOptions The validation options to apply.
 */
static void applyValidationOptions(SBMLDocument *document, const std::string &validationOptions);


/**
 * @brief Add validation errors to a vector
 * 
 * @param errors The vector to add the validation errors to.
 * @param document The SBML document to add the validation errors from.
 * @param severityFilter The severity filter to apply.
 */
static void addErrorsToVector(std::vector<ValidationError> &errors, const SBMLDocument *document, unsigned int severityFilter = 0);


/**
 * @brief Fill conversion properties from a JSON object
 * 
 * @param properties The conversion properties to fill.
 * @param optionsObject The JSON object containing the conversion options.
 */
static void fillConversionProperties(ConversionProperties &properties, const nlohmann::json &optionsObject);


/**
 * @brief Convert an SBML string to a JSON string
 * 
 * @param sbmlContent The SBML string to convert.
 * @param options The options to use for the conversion.
 * @return The JSON string containing the converted SBML.
 */
static std::string convertSBMLString(const std::string &sbmlContent, const std::string &options);


/**
 * @brief Get the conversion errors as a JSON string
 * 
 * @return A JSON string containing the conversion errors.
 */
static std::string getConversionErrors();


} // namespace sbml_utils

#endif // SBML_UTILS_H