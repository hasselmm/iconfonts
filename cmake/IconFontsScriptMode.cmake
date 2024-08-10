# FIXME doxs
function(iconfonts_add_validate_command FILEPATH EXPECTED_HASH)
    add_custom_command(
        COMMENT "Validating ${FILEPATH}"
        OUTPUT  "${FILEPATH}-${EXPECTED_HASH}"
        DEPENDS "${FILEPATH}"

        COMMAND "${CMAKE_COMMAND}" -P "${ICONFONTS_MODULE_DIR}/IconFonts.cmake"
        -- validate "${FILEPATH}" "${EXPECTED_HASH}")
endfunction()

# FIXME doxs
function(iconfonts_add_download_command URL EXPECTED_HASH OUTPUT_FILEPATH)
    add_custom_command(
        COMMENT "Downloading ${URL}..."
        OUTPUT  "${OUTPUT_FILEPATH}"

        COMMAND "${CMAKE_COMMAND}" -P "${ICONFONTS_MODULE_DIR}/IconFonts.cmake"
        -- download "${URL}" "${EXPECTED_HASH}" "${OUTPUT_FILEPATH}")

    iconfonts_add_validate_command("${OUTPUT_FILEPATH}" "${EXPECTED_HASH}")
endfunction()

# FIXME doxs
function(iconfonts_add_extract_command ARCHIVE_FILEPATH ARCHIVE_FILEHASH OUTPUT_DIRECTORY FILEPATH EXPECTED_HASH)
    set(OUTPUT_FILEPATH "${OUTPUT_DIRECTORY}/${FILEPATH}")

    add_custom_command(
        COMMENT "Extracting ${OUTPUT_FILEPATH}"
        DEPENDS "${ARCHIVE_FILEPATH}-${ARCHIVE_FILEHASH}"
        OUTPUT  "${OUTPUT_FILEPATH}"

        COMMAND "${CMAKE_COMMAND}" -P "${ICONFONTS_MODULE_DIR}/IconFonts.cmake"
        -- extract "${ARCHIVE_FILEPATH}" "${OUTPUT_DIRECTORY}" "${FILEPATH}")

    iconfonts_add_validate_command("${OUTPUT_FILEPATH}" "${EXPECTED_HASH}")
endfunction()

# FIXME doxs
function(iconfonts_add_generate_code_command)
    set(mandatory_values
        COMMON_TARGET
        QUICK_TARGET
        FONT_NAMESPACE
        INFO_FILEPATH
        INFO_FILEHASH)      # FIXME doxs
    set(optional_values)

    set(single_values ${mandatory_values} ${optional_values})
    cmake_parse_arguments(GENERATE "" "${single_values}" "" ${ARGN})
    iconfonts_require_mandatory_arguments(GENERATE ${mandatory_values})

    string(TOLOWER "${GENERATE_FONT_NAMESPACE}" basename)
    string(REPLACE ":" "" basename "${basename}")

    set(header_filepath "${ICONFONTS_GENERATED_SOURCES_DIR}/${basename}.h")
    set(source_filepath "${ICONFONTS_GENERATED_SOURCES_DIR}/${basename}.cpp")
    set(quick_filepath  "${ICONFONTS_GENERATED_SOURCES_DIR}/quick${basename}.h")

    set(script_options
        HEADER_FILEPATH "${header_filepath}"
        SOURCE_FILEPATH "${source_filepath}"
        QUICK_FILEPATH  "${quick_filepath}"
        FONT_NAMESPACE  "${GENERATE_FONT_NAMESPACE}"
        INFO_FILEPATH   "${GENERATE_INFO_FILEPATH}"
        ${GENERATE_UNPARSED_ARGUMENTS})

    __iconfonts_define_generate_source_code_arguments()
    cmake_parse_arguments(CHECK "" "${single_values}" "" ${script_options})

    iconfonts_reject_unparsed_arguments(CHECK)
    iconfonts_require_mandatory_arguments(CHECK ${mandatory_values})

    add_custom_command(
        COMMENT "Generating code for ${GENERATE_COMMON_TARGET}"

        DEPENDS
            "${GENERATE_INFO_FILEPATH}-${GENERATE_INFO_FILEHASH}"
            "${ICONFONTS_MODULE_DIR}/IconFontsCodeGenerator.cmake"
            "${ICONFONTS_MODULE_DIR}/staticfontinfo.cpp.in"
            "${ICONFONTS_MODULE_DIR}/staticfontinfo.h.in"
            "${ICONFONTS_MODULE_DIR}/quicksymbol.h.in"

        OUTPUT
            "${header_filepath}"
            "${source_filepath}"
            "${quick_filepath}"

        COMMAND "${CMAKE_COMMAND}" -P "${ICONFONTS_MODULE_DIR}/IconFonts.cmake"
        -- generate ${script_options})

    target_sources("${GENERATE_COMMON_TARGET}" PRIVATE "${header_filepath}" "${source_filepath}")
    target_sources("${GENERATE_QUICK_TARGET}"  PRIVATE "${quick_filepath}")
endfunction()

# FIXME doxs
function(iconfonts_extract ARCHIVE_FILEPATH OUTPUT_DIRECTORY FILEPATH)
    file(ARCHIVE_EXTRACT INPUT "${ARCHIVE_FILEPATH}" DESTINATION "${OUTPUT_DIRECTORY}" PATTERNS "${FILEPATH}")
endfunction()

# FIXME doxs
function(iconfonts_validate FILEPATH EXPECTED_HASH)
    iconfonts_validate_file("${FILEPATH}" "${EXPECTED_HASH}" validation_result)

    if (validation_result STREQUAL "ok")
        file(TOUCH "${FILEPATH}-${EXPECTED_HASH}")
    else()
        message(FATAL_ERROR "Validation failed for ${FILEPATH}: ${validation_result}")
    endif()
endfunction()

# FIXME doxs
function(iconfonts_run_script_command)
    math(EXPR argv_last "${CMAKE_ARGC} - 1")
    set(script_options)

    foreach(index RANGE 1 ${argv_last})
        if (script_options OR CMAKE_ARGV${index} STREQUAL "--")
            list(APPEND script_options "${CMAKE_ARGV${index}}")
        endif()
    endforeach()

    list(POP_FRONT script_options dashes script_command)

    if (script_command STREQUAL "download")
        iconfonts_download(${script_options})
    elseif (script_command STREQUAL "extract")
        iconfonts_extract(${script_options})
    elseif (script_command STREQUAL "generate")
        __iconfonts_generate_source_code(${script_options})
    elseif (script_command STREQUAL "validate")
        iconfonts_validate(${script_options})
    else()
        message(FATAL_ERROR "Unknown script command: ${script_command}")
    endif()
endfunction()
