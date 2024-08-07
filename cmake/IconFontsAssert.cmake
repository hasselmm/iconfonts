# ----------------------------------------------------------------------------------------------------------------------
# Check if the expression passed to this function is true,
# and aborts otherwise with an error message useful for debuggin.
# ----------------------------------------------------------------------------------------------------------------------
function(iconfonts_assert CONDITION)
    string(JOIN " " description "${CONDITION}" ${ARGN})
    set(description "(${description})")

    if (ARGN AND "${ARGN}" MATCHES "^((STR)?(EQUAL|GREATER|LESS)|MATCHES)")
        if (CONDITION MATCHES "^[A-Za-z_][A-Za-z0-9_]*\$")
            set(actual_value "${${CONDITION}}")
        else()
            set(actual_value "${CONDITION}")
        endif()

        if (actual_value MATCHES "[ \t]")
            set(actual_value "'${actual_value}'")
        endif()

        string(APPEND description ", actual value: '${actual_value}'")
    endif()

    if (NOT (${CONDITION} ${ARGN}))
        message(FATAL_ERROR "Assertion failed: ${description}")
    elseif (ICONFONTS_ASSERT_VERBOSE)
        message(STATUS "Assertion passed: ${description}")
    endif()
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Rejects unparsed arguments for `PREFIX` from `cmake_parse_arguments()`.
# ----------------------------------------------------------------------------------------------------------------------
macro(iconfonts_reject_unparsed_arguments PREFIX)
    if (${PREFIX}_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unexpected arguments for ${CMAKE_CURRENT_FUNCTION}(): ${${PREFIX}_UNPARSED_ARGUMENTS}")
    endif()
endmacro()

# ----------------------------------------------------------------------------------------------------------------------
# Actual implementation of `iconfonts_require_mandatory_arguments()`.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_require_mandatory_arguments_impl FUNCTION PREFIX MANDATORY_ARGUMENTS)
    set(missing_arguments)

    foreach(option IN LISTS MANDATORY_ARGUMENTS ARGN)
        if (NOT DEFINED "${PREFIX}_${option}")
            list(APPEND missing_arguments "${option}")
        endif()
    endforeach()

    if (missing_arguments)
        message(FATAL_ERROR "Mandatory arguments missing for ${FUNCTION}(): ${missing_arguments}")
    endif()
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Requires mandatory arguments for `PREFIX` from `cmake_parse_arguments()`.
# ----------------------------------------------------------------------------------------------------------------------
macro(iconfonts_require_mandatory_arguments PREFIX MANDATORY_ARGUMENTS)
    __iconfonts_require_mandatory_arguments_impl("${CMAKE_CURRENT_FUNCTION}" "${PREFIX}" ${MANDATORY_ARGUMENTS} ${ARGN})
endmacro()
