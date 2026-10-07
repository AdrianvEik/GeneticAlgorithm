# Check both the verifier's exit code and diagnostic, not just any failing process.
execute_process(COMMAND "${PROGRAM}" "${CASE}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT "${result}" STREQUAL "250")
    message(FATAL_ERROR "Expected exit 250, got ${result}: ${output}${error}")
endif()
string(FIND "${error}" "${EXPECTED}" diagnostic_position)
if(diagnostic_position EQUAL -1)
    message(FATAL_ERROR "Missing diagnostic '${EXPECTED}': ${error}")
endif()
