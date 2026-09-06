#!/bin/bash

# Run code coverage analysis
lcov -c -d . -o main_coverage.info

# Remove unwanted stuff
lcov --remove main_coverage.info -o main_coverage.info \
'/usr/*' \
'*/third_party/*' \
'*/squey-utils/*' \
'*/external/*' \
'*/build/*'

# Generate HTML report
genhtml main_coverage.info --output-directory ../public/code_coverage_report

# Generate Cobertura XML report
python3 /usr/local/bin/lcov_cobertura main_coverage.info --output ../public/code_coverage_report/cobertura-coverage.xml --demangle
