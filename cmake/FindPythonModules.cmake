# Build list of required variables for `find_package_handle_standard_args()`.

set(_required_variables PythonModules_PythonInterpreter_FOUND)

foreach(_module IN LISTS PythonModules_FIND_COMPONENTS)
    if (PythonModules_FIND_REQUIRED OR ${PythonModules_FIND_REQUIRED_${_module}})
        list(APPEND _required_variables "PythonModules_${_module}_FOUND")
    endif()
endforeach()

# Ensure we can use a Python interpreter

if (NOT TARGET Python3::Interpreter)
    find_package(Python3 COMPONENTS Interpreter QUIET)
endif()

if (PythonModules_FIND_QUIETLY)
    set(_loglevel "STATUS")
else()
    set(_loglevel "VERBOSE")
endif()

if (TARGET Python3::Interpreter)
    set(PythonModules_PythonInterpreter_FOUND YES)

    # Check usability of requested Python modules by quickly importing them

    foreach(_module IN LISTS PythonModules_FIND_COMPONENTS)
        execute_process(
            COMMAND "${Python3_EXECUTABLE}"
            -c "import ${_module}; print(vars(${_module}).get('version', ''))"
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_VARIABLE _output
            RESULT_VARIABLE _result
            ERROR_VARIABLE  _error)

        if (_result EQUAL 0)
            set(PythonModules_${_module}_FOUND YES)

            if (_output)
                message(${_loglevel} "Python module found: ${_module} (${_output})")
                set(PythonModules_${_module}_VERSION "${_output}")
            else()
                message(${_loglevel} "Python module found: ${_module}")
            endif()
        else()
            set(PythonModules_${_module}_FOUND NO)
            message(${_loglevel} "Python module NOT found: ${_module}")
            message(VERBOSE "${_error}")

            if (${PythonModules_FIND_REQUIRED_${_module}})
                set(PythonModules_FOUND NO)
            endif()
        endif()
    endforeach()
else()
    message(${_loglevel} "Python interpreter NOT found")
    set(PythonModules_PythonInterpreter_FOUND NO)
endif()

find_package_handle_standard_args(
    PythonModules "Could NOT find required Python modules"
    ${_required_variables})
