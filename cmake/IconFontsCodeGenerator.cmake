# ----------------------------------------------------------------------------------------------------------------------
# Verifies that all variables in the code template `FILEPATH` are defined and start with `PREFIX`.
# All text following `FILEPATH` is considered to be variable names.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_validate_code_template PREFIX FILEPATH)
    file(STRINGS "${SOURCE_FILEPATH}" lines_with_variables)
    set(variable_ref "\\\${([^}]+)}")
    set(variables ${ARGN})
    set(line_number 1)

    foreach(line IN LISTS lines_with_variables)
        while(line)
            if (NOT line MATCHES "${variable_ref}(.*)")
                break()
            endif()

            set(name "${CMAKE_MATCH_1}")
            set(line "${CMAKE_MATCH_2}")

            if (NOT name MATCHES "^${PREFIX}_(.*)")
                message(FATAL_ERROR "${FILEPATH}:${line_number}: Variable '${name}' does't start with '${PREFIX}'")
            elseif (NOT CMAKE_MATCH_1 IN_LIST variables)
                message(FATAL_ERROR "${FILEPATH}:${line_number}: Unknown variable '${name}'")
            endif()
        endwhile()

        math(EXPR line_number "${line_number} + 1")
    endforeach()
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Generates `TARGET_FILEPATH` from `SOURCE_FILEPATH` while only allowing the variables listed in `VARIABLES`.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_generate_from_template SOURCE_FILEPATH TARGET_FILEPATH)
    set(codegen_variables
        LIST_FILEPATH)      # filepath of the CMake lists file

    cmake_parse_arguments(PREPARSE "" "VARIABLES;OPTIONAL_VARIABLES" "" ${ARGN}) # --------------- inject more variables

    set(codegen_mandatory_variables ${codegen_variables})
    set(codegen_optional_variables)

    if (PREPARSE_VARIABLES)
        list(APPEND codegen_mandatory_variables ${${PREPARSE_VARIABLES}})
    endif()

    if (PREPARSE_OPTIONAL_VARIABLES)
        list(APPEND codegen_optional_variables ${${PREPARSE_OPTIONAL_VARIABLES}})
    endif()

    set(codegen_variables ${codegen_mandatory_variables} ${codegen_optional_variables}) # ----- parse function arguments
    cmake_parse_arguments(CODEGEN "" "${codegen_variables}" "" ${PREPARSE_UNPARSED_ARGUMENTS})
    iconfonts_require_mandatory_arguments(CODEGEN ${codegen_mandatory_variables})
    iconfonts_reject_unparsed_arguments(CODEGEN)

    list(APPEND codegen_variables ${codegen_optional_variables} TIMESTAMP) # -------------------- validate template file
    __iconfonts_validate_code_template(CODEGEN "${SOURCE_FILEPATH}" ${codegen_variables})

    message(VERBOSE "Generating ${TARGET_FILEPATH}") # -------------------------------------------- generate source code
    string(TIMESTAMP CODEGEN_TIMESTAMP)

    configure_file("${SOURCE_FILEPATH}" "${TARGET_FILEPATH}")
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Generates C++ code from `ICON_DEFINITIONS`.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_generate_source_code)
    set(mandatory_values
        TARGET              # FIXME doxs
        INFO_FILEPATH
        FONT_FAMILY
        LICENSE_FILEPATH)

    set(optional_values
        FONT_FILEPATH
        FONT_VARIANT
        INFO_FILETYPE
        INFO_OPTIONS)

    set(single_values ${mandatory_values} ${optional_values})
    cmake_parse_arguments(ICONFONTS "" "${single_values}" "" ${ARGN}) # ----------------------- parse function arguments
    iconfonts_require_mandatory_arguments(ICONFONTS ${mandatory_values})
    iconfonts_reject_unparsed_arguments(ICONFONTS)

    __iconfonts_get_target_properties("${ICONFONTS_TARGET}" ICONFONTS)

    set(font_name "${ICONFONTS_FONT_FAMILY}") # ------------------------------------------- generate various identifiers

    __iconfonts_make_symbol("${ICONFONTS_FONT_FAMILY}" family_symbol)

    if (ICONFONTS_FONT_VARIANT)
        string(TOLOWER "${ICONFONTS_FONT_FAMILY}-${ICONFONTS_FONT_VARIANT}" basename)
        __iconfonts_make_symbol("${ICONFONTS_FONT_VARIANT}" variant_symbol)
        set(font_namespace "${family_symbol}::${variant_symbol}")
        string(APPEND font_name " ${ICONFONTS_FONT_VARIANT}")
    else()
        string(TOLOWER "${ICONFONTS_FONT_FAMILY}" basename)
        set(font_namespace "${family_symbol}")
        set(variant_symbol "")
    endif()

    string(REGEX REPLACE "[\t _-]" "" basename "${basename}")

    set(header_template "${ICONFONTS_MODULE_DIR}/staticfontinfo.h.in")
    set(header_filepath "${ICONFONTS_GENERATED_SOURCES_DIR}/${basename}.h")

    set(source_template "${ICONFONTS_MODULE_DIR}/staticfontinfo.cpp.in")
    set(source_filepath "${ICONFONTS_GENERATED_SOURCES_DIR}/${basename}.cpp")

    set_property(
        DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${header_template}" "${source_template}")

    string(TOUPPER "ICONFONTS_${basename}_H" header_guard) # ---------------------------- generate file header variables

    cmake_path(
        RELATIVE_PATH ICONFONTS_INFO_FILEPATH
        BASE_DIRECTORY "${CMAKE_BINARY_DIR}"
        OUTPUT_VARIABLE pretty_info_filename)

    cmake_path(
        RELATIVE_PATH CMAKE_CURRENT_FUNCTION_LIST_FILE
        BASE_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE pretty_list_filename)

    if (ICONFONTS_FONT_FILEPATH) # -------------------------- address differences between system fonts and bundled fonts
        cmake_path(GET ICONFONTS_FONT_FILEPATH FILENAME filename)

        set(font_type "Application")
        set(font_family_expression "font<Symbols::${font_namespace}::Symbol>().family()")
        set(font_filename_literal "u\":${ICONFONTS_RESOURCE_PREFIX}/${filename}\"_s")
    elseif (ICONFONTS_FONT_FAMILY)
        set(font_type "System")
        set(font_family_expression "u\"${ICONFONTS_FONT_FAMILY} ${ICONFONTS_FONT_VARIANT}\"_s")
        set(font_filename_literal "{}")
    else()
        message(FATAL_ERROR "Either FONT_FILEPATH or FONT_FAMILY is needed")
    endif()

    if (ICONFONTS_LICENSE_FILEPATH MATCHES "\\.md\$") # ------------------ resolve resource filename of the font license
        set(license_filename "LICENSE.md")
    else()
        set(license_filename "LICENSE.txt")
    endif()

    set(current_list_file ${CMAKE_CURRENT_FUNCTION_LIST_FILE}) # ------------------------------- generate code if needed
    set(common_dependency_list ICONFONTS_INFO_FILEPATH CMAKE_CURRENT_LIST_FILE current_list_file)

    __iconfonts_check_recent(header_is_recent header_filepath header_template ${common_dependency_list})
    __iconfonts_check_recent(source_is_recent source_filepath source_template ${common_dependency_list})

    if (header_is_recent AND source_is_recent)
        message(STATUS "No code generation needed for ${font_namespace}")
    else()
        __iconfonts_collect_icons(
            FONT_VARIANT    "${ICONFONTS_FONT_VARIANT}"
            FILEPATH        "${ICONFONTS_INFO_FILEPATH}"
            FILETYPE        "${ICONFONTS_INFO_FILETYPE}"
            OPTIONS         "${ICONFONTS_INFO_OPTIONS}"
            OUTPUT_VARIABLE  icon_definition_list)

        list(JOIN icon_definition_list "" icon_definition_list)
        string(REGEX REPLACE "\n\$" "" icon_definition_list "${icon_definition_list}")

        set(mandatory_header_variables # ----------------------------------------------- generate static fontinfo header
            FONT_NAMESPACE          # C++ namespace of the genereated font
            FONT_FAMILY_SYMBOL      # C++ symbol for the font family
            FONT_TYPE               # the font type (Application, System...)
            HEADER_GUARD            # full header guard
            INFO_FILEPATH           # filepath of the icon definitions
            ICON_DEFINITIONS)       # List of icon definitions in C++

        set(optional_header_variables
            FONT_VARIANT_SYMBOL)    # C++ symbol for the font variant

        __iconfonts_generate_from_template(
            "${header_template}" "${header_filepath}"

            HEADER_GUARD            "${header_guard}"
            FONT_NAMESPACE          "${font_namespace}"
            FONT_FAMILY_SYMBOL      "${family_symbol}"
            FONT_VARIANT_SYMBOL     "${variant_symbol}"
            FONT_TYPE               "${font_type}"
            INFO_FILEPATH           "${pretty_info_filename}"
            LIST_FILEPATH           "${pretty_list_filename}"
            ICON_DEFINITIONS        "${icon_definition_list}"
            OPTIONAL_VARIABLES       optional_header_variables
            VARIABLES                mandatory_header_variables)

        set(mandatory_source_variables # ----------------------------------------------- generate static fontinfo source
            FONT_NAME               # Font name for displaying to the user
            FONT_NAMESPACE          # C++ namespace of the genereated font
            FONT_FAMILY_EXPRESSION  # C++ expression for querying the font family
            FONT_FAMILY_SYMBOL      # C++ symbol for the font family
            FONT_FILENAME_LITERAL   # C++ literal with the font filename without path
            HEADER_GUARD            # full header guard
            HEADER                  # filename of the header to include
            INFO_FILEPATH           # filepath of the icon definitions
            LICENSE_FILEPATH        # filepath of the license text
            RESOURCE_SYMBOL)        # C++ symbol name of the Qt resource

        set(optional_header_variables
            FONT_VARIANT_SYMBOL)    # C++ symbol for the font variant

        __iconfonts_generate_from_template(
            "${source_template}" "${source_filepath}"

            FONT_NAMESPACE          "${font_namespace}"
            FONT_FAMILY_EXPRESSION  "${font_family_expression}"
            FONT_FAMILY_SYMBOL      "${family_symbol}"
            FONT_VARIANT_SYMBOL     "${variant_symbol}"
            FONT_FILENAME_LITERAL   "${font_filename_literal}"
            HEADER_GUARD            "${header_guard}"
            HEADER                  "${basename}.h"
            FONT_NAME               "${font_name}"
            INFO_FILEPATH           "${pretty_info_filename}"
            LIST_FILEPATH           "${pretty_list_filename}"
            LICENSE_FILEPATH        "${ICONFONTS_RESOURCE_PREFIX}/${license_filename}"
            RESOURCE_SYMBOL         "${family_symbol}"
            OPTIONAL_VARIABLES       optional_header_variables
            VARIABLES                mandatory_source_variables)
    endif()

    target_sources( # ----------------------------------------------------------------------------------- report results
        "${ICONFONTS_TARGET}" PRIVATE
        "${header_filepath}"
        "${source_filepath}")

    set_property(
        TARGET "${ICONFONTS_TARGET}" APPEND PROPERTY
        ICONFONTS_FONT_NAMESPACES "${font_namespace}")
endfunction(__iconfonts_generate_source_code)
