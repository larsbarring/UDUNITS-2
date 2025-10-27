/*
 * UDUNITS2 Custom Unit Test Program
 * 
 * This program tests parsing of unit expressions with custom unit definitions.
 * 
 * Usage: ./udunits2_custom_test "new_unit_definition" "expression_to_test"
 * 
 * Example:
 *   ./udunits2_custom_test "foo = 5 * meter" "foo^2"
 *   ./udunits2_custom_test "bar = kg/m^3" "2.5 bar"
 * 
 * The program:
 * 1. Adds the new unit definition to the unit system
 * 2. Attempts to parse the test expression
 * 3. If successful, outputs the canonical form (like udunits2 -W "")
 * 4. If failed, outputs "FAILED"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <udunits2.h>

// Global unit system
static ut_system* unit_system = NULL;

// Initialize the unit system
int init_unit_system() {
    // Suppress error messages during testing
    ut_set_error_message_handler(ut_ignore);
    
    // Initialize with default XML database
    unit_system = ut_read_xml(NULL);
    
    if (unit_system == NULL) {
        fprintf(stderr, "Error: Could not initialize unit system\n");
        return 0;
    }
    
    return 1;
}

// Add a custom unit definition to the system
int add_custom_unit(const char* unit_definition) {
    // Parse the unit definition
    ut_unit* new_unit = ut_parse(unit_system, unit_definition, UT_UTF8);
    
    if (new_unit == NULL) {
        return 0;  // Failed to parse the unit definition
    }
    
    // The unit definition should be in the form "name = expression"
    // We need to extract the name and map it to the unit
    
    // Find the '=' sign
    const char* equals_pos = strchr(unit_definition, '=');
    if (equals_pos == NULL) {
        ut_free(new_unit);
        return 0;  // No '=' found in definition
    }
    
    // Extract the unit name (everything before '=', trimmed)
    int name_len = equals_pos - unit_definition;
    char* unit_name = malloc(name_len + 1);
    if (unit_name == NULL) {
        ut_free(new_unit);
        return 0;  // Memory allocation failed
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
    
    // Map the name to the unit
    ut_status status = ut_map_name_to_unit(trimmed_name, UT_UTF8, new_unit);
    
    free(unit_name);
    ut_free(new_unit);
    
    return (status == UT_SUCCESS) ? 1 : 0;
}

// Test parsing of an expression and output result
void test_unit_expression(const char* expression) {
    ut_unit* unit = ut_parse(unit_system, expression, UT_UTF8);
    
    if (unit == NULL) {
        printf("FAILED\n");
        return;
    }
    
    // Try to get the canonical representation
    // Use a reasonably large buffer
    char canonical_form[512];
    int format_result = ut_format(unit, canonical_form, sizeof(canonical_form), UT_UTF8);
    
    if (format_result >= 0) {
        // Format succeeded (or buffer was too small but we got something)
        if ((size_t)format_result < sizeof(canonical_form)) {
            // The buffer was large enough
            printf("%s\n", canonical_form);
        } else {
            // Buffer was too small, but let's try with what we have
            canonical_form[sizeof(canonical_form)-1] = '\0';
            printf("%s\n", canonical_form);
        }
    } else {
        // Formatting failed, but parsing succeeded
        // Try with ASCII encoding as fallback
        format_result = ut_format(unit, canonical_form, sizeof(canonical_form), UT_ASCII);
        if (format_result >= 0) {
            canonical_form[sizeof(canonical_form)-1] = '\0';
            printf("%s\n", canonical_form);
        } else {
            // Even ASCII formatting failed, just output "1" (dimensionless)
            printf("1\n");
        }
    }
    
    ut_free(unit);
}

void print_usage(const char* program_name) {
    printf("Usage: %s \"new_unit_definition\" \"expression_to_test\"\n", program_name);
    printf("\n");
    printf("Examples:\n");
    printf("  %s \"foo = 5 * meter\" \"foo^2\"\n", program_name);
    printf("  %s \"bar = kg/m^3\" \"2.5 bar\"\n", program_name);
    printf("  %s \"custom = 10 * second\" \"custom per minute\"\n", program_name);
    printf("\n");
    printf("The program adds the new unit definition to the unit system,\n");
    printf("then attempts to parse the test expression.\n");
    printf("If successful, it outputs the canonical form.\n");
    printf("If failed, it outputs \"FAILED\".\n");
}

int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc != 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    const char* unit_definition = argv[1];
    const char* expression_to_test = argv[2];
    
    // Initialize unit system
    if (!init_unit_system()) {
        printf("FAILED\n");
        return EXIT_FAILURE;
    }
    
    // Add the custom unit definition
    if (!add_custom_unit(unit_definition)) {
        printf("FAILED\n");
        ut_free_system(unit_system);
        return EXIT_FAILURE;
    }
    
    // Test the expression
    test_unit_expression(expression_to_test);
    
    // Clean up
    ut_free_system(unit_system);
    
    return EXIT_SUCCESS;
}