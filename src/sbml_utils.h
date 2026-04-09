#ifndef SBML_UTILS_H
#define SBML_UTILS_H

#include <string>
#include <vector>

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
std::string validateSBMLString(const std::string& sbmlContent, const std::string& validationOptions = "");

struct ValidationError {
    unsigned int line;
    std::string message;
    std::string severity;
};

#endif // SBML_UTILS_H