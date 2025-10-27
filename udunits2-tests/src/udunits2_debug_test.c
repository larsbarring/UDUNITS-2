/*
 * UDUNITS2 Debug Test Program
 * 
 * This is a diagnostic version that shows detailed information about what's failing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <udunits2.h>

// Global unit system
static ut_system* unit_system = NULL;

// Convert status to string for debugging
const char* status_to_string(ut_status status) {
    switch (status) {
        case UT_SUCCESS: return "UT_SUCCESS";
        case UT_BAD_ARG: return "UT_BAD_ARG";
        case UT_EXISTS: return "UT_EXISTS";
        case UT_NO_UNIT: return "UT_NO_UNIT";
        case UT_OS: return "UT_OS";
        case UT_NOT_SAME_SYSTEM: return "UT_NOT_SAME_SYSTEM";
        case UT_MEANINGLESS: return "UT_MEANINGLESS";
        case UT_NO_SECOND: return "UT_NO_SECOND";
        case UT_VISIT_ERROR: return "UT_VISIT_ERROR";
        case UT_CANT_FORMAT: return "UT_CANT_FORMAT";
        case UT_SYNTAX: return "UT_SYNTAX";
        case UT_UNKNOWN: return "UT_UNKNOWN";
        case UT_OPEN_ARG: return "UT_OPEN_ARG";
        case UT_OPEN_ENV: return "UT_OPEN_ENV";
        case UT_OPEN_DEFAULT: return "UT_OPEN_DEFAULT";
        case UT_PARSE: return "UT_PARSE";
        default: return "UNKNOWN_STATUS";
    }
}

// Initialize the unit system with debugging
int init_unit_system() {
    printf("DEBUG: Initializing unit system...\n");
    
    // Don't suppress error messages for debugging
    unit_system = ut_read_xml(NULL);
    
    if (unit_system == NULL) {
        printf("DEBUG: Failed to initialize unit system\n");
        printf("DEBUG: Status: %s\n", status_to_string(ut_get_status()));
        return 0;
    }
    
    printf("DEBUG: Unit system initialized successfully\n");
    return 1;
}

// Add a custom unit definition with debugging
int add_custom_unit(const char* unit_definition) {
    printf("DEBUG: Adding custom unit definition: '%s'\n", unit_definition);
    
    // Find the '=' sign
    const char* equals_pos = strchr(unit_definition, '=');
    if (equals_pos == NULL) {
        printf("DEBUG: No '=' found in unit definition\n");
        return 0;
    }
    
    // Extract the unit name (everything before '=', trimmed)
    int name_len = equals_pos - unit_definition;
    char* unit_name = malloc(name_len + 1);
    if (unit_name == NULL) {
        printf("DEBUG: Memory allocation failed\n");
        return 0;
    }
    
    strncpy(unit_name, unit_definition, name_len);
    unit_name[name_len] = '\0';
    
    // Trim whitespace from the end
    while (name_len > 0 && (unit_name[name_len-1] == ' ' || unit_name[name_len-1] == '\t')) {
        unit_name[--name_len] = '\0';
    }
    
    // Trim whitespace from the beginning
    char* trimmed_name = unit_name;
    while (*trimmed_name == ' ' || *trimmed_name == '\t') {
        trimmed_name++;
    }
    
    printf("DEBUG: Extracted unit name: '%s'\n", trimmed_name);
    
    // Extract the expression (everything after '=', trimmed)
    const char* expression_start = equals_pos + 1;
    while (*expression_start == ' ' || *expression_start == '\t') {
        expression_start++;
    }
    
    printf("DEBUG: Unit expression: '%s'\n", expression_start);
    
    // Parse the unit expression
    ut_unit* new_unit = ut_parse(unit_system, expression_start, UT_UTF8);
    
    if (new_unit == NULL) {
        printf("DEBUG: Failed to parse unit expression\n");
        printf("DEBUG: Status: %s\n", status_to_string(ut_get_status()));
        free(unit_name);
        return 0;
    }
    
    printf("DEBUG: Unit expression parsed successfully\n");
    
    // Try to format the parsed unit to see what we got
    char unit_str[256];
    int format_result = ut_format(new_unit, unit_str, sizeof(unit_str), UT_UTF8);
    if (format_result >= 0) {
        printf("DEBUG: Parsed unit represents: '%s'\n", unit_str);
    }
    
    // Map the name to the unit
    printf("DEBUG: Mapping name '%s' to unit...\n", trimmed_name);
    ut_status status = ut_map_name_to_unit(trimmed_name, UT_UTF8, new_unit);
    
    printf("DEBUG: Mapping status: %s\n", status_to_string(status));
    
    // Test if we can retrieve the unit by name
    ut_unit* test_unit = ut_get_unit_by_name(unit_system, trimmed_name);
    if (test_unit != NULL) {
        printf("DEBUG: Successfully retrieved unit by name\n");
        ut_free(test_unit);
    } else {
        printf("DEBUG: Failed to retrieve unit by name\n");
        printf("DEBUG: Status: %s\n", status_to_string(ut_get_status()));
    }
    
    free(unit_name);
    ut_free(new_unit);
    
    return (status == UT_SUCCESS) ? 1 : 0;
}

// Test parsing of an expression with debugging
void test_unit_expression(const char* expression) {
    printf("DEBUG: Testing expression: '%s'\n", expression);
    
    ut_unit* unit = ut_parse(unit_system, expression, UT_UTF8);
    
    if (unit == NULL) {
        printf("DEBUG: Failed to parse test expression\n");
        printf("DEBUG: Status: %s\n", status_to_string(ut_get_status()));
        printf("FAILED\n");
        return;
    }
    
    printf("DEBUG: Test expression parsed successfully\n");
    
    // Try to get the canonical representation
    char canonical_form[512];
    int format_result = ut_format(unit, canonical_form, sizeof(canonical_form), UT_UTF8);
    
    if (format_result >= 0) {
        if ((size_t)format_result < sizeof(canonical_form)) {
            printf("DEBUG: Formatted successfully: '%s'\n", canonical_form);
            printf("%s\n", canonical_form);
        } else {
            canonical_form[sizeof(canonical_form)-1] = '\0';
            printf("DEBUG: Formatted with truncation: '%s'\n", canonical_form);
            printf("%s\n", canonical_form);
        }
    } else {
        printf("DEBUG: Formatting failed with UTF-8, trying ASCII...\n");
        format_result = ut_format(unit, canonical_form, sizeof(canonical_form), UT_ASCII);
        if (format_result >= 0) {
            canonical_form[sizeof(canonical_form)-1] = '\0';
            printf("DEBUG: ASCII formatting succeeded: '%s'\n", canonical_form);
            printf("%s\n", canonical_form);
        } else {
            printf("DEBUG: All formatting failed\n");
            printf("1\n");
        }
    }
    
    ut_free(unit);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s \"new_unit_definition\" \"expression_to_test\"\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const char* unit_definition = argv[1];
    const char* expression_to_test = argv[2];
    
    printf("DEBUG: Starting test with:\n");
    printf("DEBUG: Unit definition: '%s'\n", unit_definition);
    printf("DEBUG: Test expression: '%s'\n", expression_to_test);
    printf("\n");
    
    // Initialize unit system
    if (!init_unit_system()) {
        printf("FAILED\n");
        return EXIT_FAILURE;
    }
    
    printf("\n");
    
    // Add the custom unit definition
    if (!add_custom_unit(unit_definition)) {
        printf("FAILED\n");
        ut_free_system(unit_system);
        return EXIT_FAILURE;
    }
    
    printf("\n");
    
    // Test the expression
    test_unit_expression(expression_to_test);
    
    // Clean up
    ut_free_system(unit_system);
    
    return EXIT_SUCCESS;
}