enable_testing()
add_test(test test_furl.sh)

configure_file(test_furl.sh.cmake test_furl.sh @ONLY)
