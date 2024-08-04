# ----------------------------------------------------------------------------------------------------------------------
# Check for Python as using Python can significantly accelerate metadata parsing.
# ----------------------------------------------------------------------------------------------------------------------

if (NOT TARGET Python3::Interpreter)
    find_package(Python3 COMPONENTS Interpreter)
    message(STATUS "Using Python: ${Python3_EXECUTABLE}")
endif()

# ----------------------------------------------------------------------------------------------------------------------
# Enable minimal automated testing
# ----------------------------------------------------------------------------------------------------------------------

option(ICONFONTS_ENABLE_TESTING "Run trivial unit tests while configuring" OFF)

if (ICONFONTS_ENABLE_TESTING)
    find_package(PythonModules REQUIRED COMPONENTS pylint mypy)
endif()

# ----------------------------------------------------------------------------------------------------------------------
# Defines a custom command to run `LINTER` for `SOURCE_FILENAME` of `TARGET`.
# ----------------------------------------------------------------------------------------------------------------------
function(iconfonts_add_python_linter TARGET LINTER SOURCE_FILENAME)
    iconfonts_assert(TARGET "${TARGET}")

    add_custom_command(
        OUTPUT  ${NAME}_${LINTER}.timestamp
        COMMAND Python3::Interpreter -m "${LINTER}" "${SOURCE_FILENAME}"
        COMMAND "${CMAKE_COMMAND}" -E touch ${NAME}_${LINTER}.timestamp
        COMMENT "Running ${LINTER} on ${SOURCE_FILENAME}"
        DEPENDS "${SOURCE_FILENAME}"
    )
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Defines a custom target of `NAME` for the Python script `SOURCE_FILENAME`.
# ----------------------------------------------------------------------------------------------------------------------
function(iconfonts_add_python_script NAME SOURCE_FILENAME)
    add_custom_target(
        "${NAME}"

        DEPENDS
            "${NAME}_pylint.timestamp"
            "${NAME}_mypy.timestamp"

        SOURCES "${SOURCE_FILENAME}"
    )

    iconfonts_add_python_linter("${NAME}" pylint "${SOURCE_FILENAME}")
    iconfonts_add_python_linter("${NAME}" mypy "${SOURCE_FILENAME}")

    if (ICONFONTS_ENABLE_TESTING)
        set_target_properties("${NAME}" PROPERTIES EXCLUDE_FROM_ALL NO)
    endif()
endfunction()
