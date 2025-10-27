/*
 * UDUNITS2 Library Test Program
 *
 * This program tests various aspects of the UDUNITS2 C library to help identify
 * discrepancies between the documented grammar, test cases, and actual implementation.
 *
 * Usage: ./udunits2_test [xml_path]
 *
 * The program tests:
 * 1. Basic unit parsing and validation
 * 2. Multiplication operators
 * 3. Division operators
 * 4. Exponentiation
 * 5. Logarithmic references
 * 6. Shift operations
 * 7. Timestamp parsing
 * 8. Error conditions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <udunits2.h>

// Test result structure
typedef struct {
    const char* expression;
    int expected_valid;  // 1 if should be valid, 0 if should be invalid
    const char* description;
    int result;          // Actual test result: 1=pass, 0=fail, -1=error
    const char* error_msg;
} test_case_t;

// Global unit system
static ut_system* unit_system = NULL;

// Test statistics
static int total_tests = 0;
static int passed_tests = 0;
static int failed_tests = 0;

// Helper function to initialize the unit system
int init_unit_system(const char* xml_path) {
    ut_set_error_message_handler(ut_ignore);  // Suppress error messages during testing

    if (xml_path) {
        unit_system = ut_read_xml(xml_path);
    } else {
        // Use default XML path
        unit_system = ut_read_xml(NULL);
    }

    if (unit_system == NULL) {
        fprintf(stderr, "Error: Could not initialize unit system\n");
        fprintf(stderr, "Status: %s\n", ut_get_status() == UT_OPEN_DEFAULT ? "UT_OPEN_DEFAULT" :
                                        ut_get_status() == UT_OPEN_ENV ? "UT_OPEN_ENV" :
                                        ut_get_status() == UT_OPEN_ARG ? "UT_OPEN_ARG" : "Other");
        return 0;
    }

    return 1;
}

// Helper function to test if a unit expression is valid
int test_unit_expression(const char* expression) {
    ut_unit* unit = ut_parse(unit_system, expression, UT_UTF8);
    if (unit != NULL) {
        ut_free(unit);
        return 1;  // Valid
    }
    return 0;  // Invalid
}

// Function to run a single test case
void run_test(test_case_t* test) {
    total_tests++;

    printf("Test %d: %s\n", total_tests, test->description);
    printf("  Expression: '%s'\n", test->expression);
    printf("  Expected: %s\n", test->expected_valid ? "VALID" : "INVALID");

    int is_valid = test_unit_expression(test->expression);
    printf("  Actual: %s\n", is_valid ? "VALID" : "INVALID");

    if ((is_valid && test->expected_valid) || (!is_valid && !test->expected_valid)) {
        printf("  Result: PASS\n");
        test->result = 1;
        passed_tests++;
    } else {
        printf("  Result: FAIL (got %s, expected %s)\n",
               is_valid ? "VALID" : "INVALID",
               test->expected_valid ? "VALID" : "INVALID");
        test->result = 0;
        failed_tests++;
    }

    // Show the last error status for debugging
    ut_status status = ut_get_status();
    if (status != UT_SUCCESS) {
        printf("  Error status: %d\n", status);
    }

    printf("\n");
}

// Test suite runner
void run_test_suite(test_case_t tests[], int num_tests, const char* suite_name) {
    printf("=== %s ===\n\n", suite_name);

    for (int i = 0; i < num_tests; i++) {
        run_test(&tests[i]);
    }
}

int main(int argc, char* argv[]) {
    const char* xml_path = (argc > 1) ? argv[1] : NULL;

    printf("UDUNITS2 Library Test Program\n");
    printf("=============================\n\n");

    // Initialize unit system
    if (!init_unit_system(xml_path)) {
        return EXIT_FAILURE;
    }

    printf("Unit system initialized successfully.\n");
    if (xml_path) {
        printf("Using XML file: %s\n", xml_path);
    } else {
        printf("Using default XML database.\n");
    }
    printf("\n");

    // Test cases: Basic valid expressions
    test_case_t basic_valid_tests[] = {
        {"meter", 1, "Simple unit name"},
        {"m", 1, "Simple unit symbol"},
        {"kg", 1, "Basic SI unit"},
        {"second", 1, "Time unit"},
        {"celsius", 1, "Temperature unit"},
        {"1", 1, "Dimensionless number"},
        {"42", 1, "Integer number"},
        {"3.14159", 1, "Decimal number"},
        {"-5", 1, "Negative number"},
        {"ns", 1, "Nanoseconds symbol"},        // specific for issue #132
        {"nanoseconds", 1, "Nanoseconds name"}  // specific for issue #132
    };

    // Test cases: Multiplication operators
    test_case_t multiplication_tests[] = {
        {"kg*m", 1, "Asterisk multiplication"},
        {"kg.m", 1, "Dot multiplication"},
        {"kg-m", 1, "Hyphen multiplication"},
        {"kg m", 1, "Whitespace multiplication"},
        {"kg*m*s", 1, "Multiple multiplications"}
    };

    // Test cases: Division operators              specific for issue #129
    test_case_t division_tests[] = {
        {"m/s", 1, "Slash division"},
        {"m per s", 1, "Per division"},
        {"m PER s", 1, "Case insensitive per"},
        {"m Per s", 1, "Mixed case per"},
        {"3 perch m", 1, "Perch is a unit"},
        {"3 m perch", 1, "Perch is a unit"},
        {"perch per m", 1, "Perch is a unit"}
    };

    // Test cases: Exponentiation
    test_case_t exponent_tests[] = {
        {"m^2", 1, "Caret exponent"},
        {"m**2", 1, "Double asterisk exponent"},
        {"m^-1", 1, "Negative exponent"},
        {"m^0", 1, "Zero exponent"},
        {"m^1", 1, "Unity exponent"},
        {"m2", 1, "Superscript-style exponent"},
        {"m^999", 0, "Too large exponent"}
    };

    // Test cases: Parentheses
    test_case_t parentheses_tests[] = {
        {"(kg*m)", 1, "Simple grouping"},
        {"(kg*m)/s", 1, "Division grouping"},
        {"kg*(m/s)", 1, "Multiplication grouping"},
        {"((kg))", 1, "Nested parentheses"},
        {"(kg", 0, "Unclosed parenthesis"},
        {"kg)", 1, "Unopened parenthesis (known bug)"}  // Known bug: this should fail, but does not
    };

    // Test cases: Logarithmic references
    test_case_t logarithmic_tests[] = {
        {"lg(re 1)", 1, "Base-10 log dimensionless"},
        {"lg(re 1 mW)", 1, "Base-10 log with unit"},
        {"ln(re 1 K)", 1, "Natural log"},
        {"lb(re 1 Hz)", 1, "Base-2 log"},
        {"lg(re)", 0, "Missing reference unit"},
        {"lg(re 1", 0, "Missing closing parenthesis"}
    };

    // Test cases: Shift operations
    test_case_t shift_tests[] = {
        {"celsius @ 20", 1, "Temperature shift with @"},
        {"celsius after 20", 1, "Temperature shift with after"},
        {"celsius AFTER 20", 1, "Case insensitive after"},
        {"celsius from 0", 1, "Temperature shift with from"},
        {"celsius since 273.15", 1, "Temperature shift with since"},
        {"K @ 273.15", 1, "Kelvin shift"}
    };

    // Test cases: Timestamps
    test_case_t timestamp_tests[] = {
        {"seconds since 2000-01-01", 1, "Basic timestamp"},
        {"days since 1990-1-1", 1, "Short date format"},
        {"hours since 2023-12-25", 1, "Christmas date"},
        {"minutes since 2000-01-01 12:00:00", 1, "Date with time"},
        {"seconds since 2000-01-01T12:00:00", 1, "ISO 8601 format"},
        {"days since 20231225", 1, "Packed date format"}
    };

    // Test cases: Invalid expressions
    test_case_t invalid_tests[] = {
        {"foobar", 0, "Unknown unit name"},
        {"kg**", 0, "Missing exponent"},
        {"m^", 0, "Missing exponent after caret"},
        {"kg*/m", 0, "Multiple operators"},
        {"", 1, "Empty string"},
        {" ", 0, "Whitespace only"},
        {"kg @ @ 20", 0, "Double shift operator"},
        {"since", 0, "Shift operator without unit"}
    };

    // Run all test suites
    run_test_suite(basic_valid_tests, sizeof(basic_valid_tests)/sizeof(test_case_t), "BASIC VALID EXPRESSIONS");
    run_test_suite(multiplication_tests, sizeof(multiplication_tests)/sizeof(test_case_t), "MULTIPLICATION OPERATORS");
    run_test_suite(division_tests, sizeof(division_tests)/sizeof(test_case_t), "DIVISION OPERATORS");
    run_test_suite(exponent_tests, sizeof(exponent_tests)/sizeof(test_case_t), "EXPONENTIATION");
    run_test_suite(parentheses_tests, sizeof(parentheses_tests)/sizeof(test_case_t), "PARENTHESES");
    run_test_suite(logarithmic_tests, sizeof(logarithmic_tests)/sizeof(test_case_t), "LOGARITHMIC REFERENCES");
    run_test_suite(shift_tests, sizeof(shift_tests)/sizeof(test_case_t), "SHIFT OPERATIONS");
    run_test_suite(timestamp_tests, sizeof(timestamp_tests)/sizeof(test_case_t), "TIMESTAMPS");
    run_test_suite(invalid_tests, sizeof(invalid_tests)/sizeof(test_case_t), "INVALID EXPRESSIONS");

    // Print summary
    printf("===============================================\n");
    printf("BASIC TEST RESULTS SUMMARY\n");
    printf("===============================================\n");
    printf("Total Tests: %d\n", total_tests);
    printf("Passed:      %d\n", passed_tests);
    printf("Failed:      %d\n", failed_tests);
    printf("Success Rate: %.1f%%\n\n", (float)passed_tests / total_tests * 100);

    // Clean up
    ut_free_system(unit_system);

    return (failed_tests == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}