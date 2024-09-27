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
    cmake_parse_arguments(CODEGEN "" "" "${codegen_variables}" ${PREPARSE_UNPARSED_ARGUMENTS})
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
        INFO_OPTIONS
        QUICK_TARGET)

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

    set(quick_template "${ICONFONTS_MODULE_DIR}/quicksymbol.h.in")
    set(quick_filepath "${ICONFONTS_GENERATED_SOURCES_DIR}/quick${basename}.h")

    set_property(
        DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${header_template}" "${source_template}" "${quick_template}")

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
    __iconfonts_check_recent( quick_is_recent  quick_filepath  quick_template ${common_dependency_list})

    if (header_is_recent AND source_is_recent AND quick_is_recent)
        message(STATUS "No code generation needed for ${font_namespace}")
    else()
        __iconfonts_collect_icons( # ----------------------------------------- collect icons and generate symbol definitions
            FONT_VARIANT    "${ICONFONTS_FONT_VARIANT}"
            FILEPATH        "${ICONFONTS_INFO_FILEPATH}"
            FILETYPE        "${ICONFONTS_INFO_FILETYPE}"
            OPTIONS         "${ICONFONTS_INFO_OPTIONS}"
            OUTPUT_VARIABLE  icon_definition_list)

        list(JOIN icon_definition_list "" icon_definition_list)

        string(
            REGEX REPLACE "\n\$" ""
            icon_definition_list "${icon_definition_list}")

        string(
            REGEX REPLACE
            "(([^=\t ]+)[\t ]*) = [^,]+,"
            "    \\1 = tagged<Symbol::\\2>,"
            quick_icon_definition_list "${icon_definition_list}")

        iconfonts_assert(NOT icon_definition_list STREQUAL quick_icon_definition_list)
    endif()

    set(mandatory_variables # ----------------------------------------------------- define variables for code generation
        FONT_NAMESPACE          # C++ namespace of the genereated font
        FONT_FAMILY_SYMBOL      # C++ symbol for the font family
        FONT_SYMBOL             # C++ symbol for the font containing family and variant
        INFO_FILEPATH)          # filepath of the icon definitions

    set(header_mandatory_variables ${mandatory_variables}
        FONT_TYPE               # the font type (Application, System...)
        HEADER_GUARD            # full header guard
        ICON_DEFINITIONS)       # List of icon definitions in C++

    set(source_mandatory_variables ${mandatory_variables}
        FONT_NAME               # Font name for displaying to the user
        FONT_FAMILY_EXPRESSION  # C++ expression for querying the font family
        FONT_FILENAME_LITERAL   # C++ literal with the font filename without path
        HEADER_FILENAME         # filename of the header to include
        LICENSE_FILEPATH        # filepath of the license text
        RESOURCE_SYMBOL)        # C++ symbol name of the Qt resource

    set(quick_mandatory_variables ${header_mandatory_variables}
        HEADER_FILENAME)        # filename of the header to include

    list(
        APPEND header_mandatory_variables
        FONT_TAG)               # unique tag for symbols of this font

    set(optional_variables
        FONT_VARIANT_SYMBOL)    # C++ symbol for the font variant

    if (NOT header_is_recent) # -------------------------------------------------------- generate static fontinfo header
        __iconfonts_generate_from_template(
            "${header_template}" "${header_filepath}"

            HEADER_GUARD            "${header_guard}"
            FONT_NAMESPACE          "${font_namespace}"
            FONT_SYMBOL             "${family_symbol}${variant_symbol}"
            FONT_TAG                "${ICONFONTS_NEXT_FONT_TAG}"
            FONT_FAMILY_SYMBOL      "${family_symbol}"
            FONT_VARIANT_SYMBOL     "${variant_symbol}"
            FONT_TYPE               "${font_type}"
            INFO_FILEPATH           "${pretty_info_filename}"
            LIST_FILEPATH           "${pretty_list_filename}"
            ICON_DEFINITIONS        "${icon_definition_list}"
            VARIABLES                header_mandatory_variables
            OPTIONAL_VARIABLES       optional_variables)
    endif()

    if (NOT source_is_recent) # -------------------------------------------------------- generate static fontinfo source
        __iconfonts_generate_from_template(
            "${source_template}" "${source_filepath}"

            FONT_NAMESPACE          "${font_namespace}"
            FONT_FAMILY_EXPRESSION  "${font_family_expression}"
            FONT_SYMBOL             "${family_symbol}${variant_symbol}"
            FONT_FAMILY_SYMBOL      "${family_symbol}"
            FONT_VARIANT_SYMBOL     "${variant_symbol}"
            FONT_FILENAME_LITERAL   "${font_filename_literal}"
            HEADER_FILENAME         "${basename}.h"
            FONT_NAME               "${font_name}"
            INFO_FILEPATH           "${pretty_info_filename}"
            LIST_FILEPATH           "${pretty_list_filename}"
            LICENSE_FILEPATH        "${ICONFONTS_RESOURCE_PREFIX}/${license_filename}"
            RESOURCE_SYMBOL         "${family_symbol}"
            VARIABLES                source_mandatory_variables
            OPTIONAL_VARIABLES       optional_variables)
    endif()

    if (NOT quick_is_recent AND ICONFONTS_QUICK_TARGET) # -------------------------- generate tagged symbols for QtQuick
        __iconfonts_generate_from_template(
            "${quick_template}" "${quick_filepath}"

            HEADER_FILENAME         "${basename}.h"
            HEADER_GUARD            "QUICK${header_guard}"
            FONT_NAMESPACE          "${font_namespace}"
            FONT_SYMBOL             "${family_symbol}${variant_symbol}"
            FONT_FAMILY_SYMBOL      "${family_symbol}"
            FONT_VARIANT_SYMBOL     "${variant_symbol}"
            FONT_TYPE               "${font_type}"
            INFO_FILEPATH           "${pretty_info_filename}"
            LIST_FILEPATH           "${pretty_list_filename}"
            ICON_DEFINITIONS        "${quick_icon_definition_list}"
            VARIABLES                quick_mandatory_variables
            OPTIONAL_VARIABLES       optional_variables)
    endif()

    math(EXPR next_font_tag "${ICONFONTS_NEXT_FONT_TAG} + 1") # ----------------------- generate and store next font tag

    set_property(
        TARGET "${ICONFONTS_TARGET}" PROPERTY
        ICONFONTS_NEXT_FONT_TAG "${next_font_tag}")

    target_sources( # ----------------------------------------------------------------------------------- report results
        "${ICONFONTS_TARGET}" PRIVATE
        "${header_filepath}"
        "${source_filepath}")

    if (ICONFONTS_QUICK_TARGET)
        target_sources("${ICONFONTS_QUICK_TARGET}" PRIVATE "${quick_filepath}")
    endif()

    set_property(
        TARGET "${ICONFONTS_TARGET}" APPEND PROPERTY
        ICONFONTS_FONT_NAMESPACES "${font_namespace}")
endfunction(__iconfonts_generate_source_code)

# ----------------------------------------------------------------------------------------------------------------------
# Finalizes `TARGET` by generating its configuration header and its font registry.
# ----------------------------------------------------------------------------------------------------------------------
function(iconfonts_finalize_target TARGET)
    __iconfonts_get_target_properties("${TARGET}" FINALIZE) # ------------------------------------------- inspect target

    list(LENGTH FINALIZE_FONT_NAMESPACES font_count)
    message(STATUS "Finalizing target ${TARGET} with ${font_count} icon font(s)")

    target_include_directories( # ---------------------------------------------------- update include path of the target
        "${TARGET}" PUBLIC
        "${FINALIZE_GENERATED_INCLUDE_DIR}"
        "${FINALIZE_GENERATED_SOURCES_DIR}")

    if (ICONFONTS_ENABLE_ALL_FONTS) # --------------------------- ensure we really list all fonts in "iconfontsconfig.h"
        set(define "<23>define")
    else()
        set(define "<23>cmakedefine")
    endif()

    iconfonts_unescape(define)

    set(known_fonts_include_list ${FINALIZE_FONT_NAMESPACES}) # ------------------ prepare variables for generating code
    list(TRANSFORM known_fonts_include_list REPLACE "::" "")
    list(TRANSFORM known_fonts_include_list REPLACE "_" "")
    list(TRANSFORM known_fonts_include_list TOLOWER)
    list(TRANSFORM known_fonts_include_list PREPEND "#include \"")
    list(TRANSFORM known_fonts_include_list APPEND ".h\"")
    list(JOIN known_fonts_include_list "\n" known_fonts_include_list)

    set(known_fonts_info_list ${FINALIZE_FONT_NAMESPACES})
    list(TRANSFORM known_fonts_info_list PREPEND "        FontInfo::instance<Symbols::")
    list(TRANSFORM known_fonts_info_list APPEND "::Symbol>(),")
    list(JOIN known_fonts_info_list "\n" known_fonts_info_list)

    unset(known_fonts_assertion_list)
    set(padded_namespace_list "-" ${FINALIZE_FONT_NAMESPACES})

    foreach(index RANGE 1 ${font_count})
        list(GET padded_namespace_list ${index} namespace)

        string(
            APPEND known_fonts_assertion_list
            "    static_assert(IconFonts::fontTag<${namespace}::Symbol>().index() == ${index})\\;\n")
    endforeach()

    set(font_option_defines ${FINALIZE_FONT_OPTIONS})
    list(TRANSFORM font_option_defines PREPEND "${define} ")
    list(JOIN font_option_defines "\n" font_option_defines)

    cmake_path(
        RELATIVE_PATH CMAKE_CURRENT_FUNCTION_LIST_FILE
        BASE_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE pretty_list_filename)

    set(config_filepath  "${FINALIZE_GENERATED_SOURCES_DIR}/iconfontsconfig.h") # ----------- generate iconfontsconfig.h
    set(config_variables FONT_OPTION_DEFINES)

    __iconfonts_generate_from_template(
        "${ICONFONTS_MODULE_DIR}/iconfontsconfig.h.in" "${config_filepath}.in"

        LIST_FILEPATH           "${pretty_list_filename}"
        FONT_OPTION_DEFINES     "${font_option_defines}"
        VARIABLES                config_variables)

    configure_file("${config_filepath}.in" "${config_filepath}")
    target_sources("${TARGET}" PRIVATE "${config_filepath}")

    set(registry_filepath "${FINALIZE_GENERATED_SOURCES_DIR}/iconfontsregistry.cpp") # ----------- iconfontsregistry.cpp
    set(registry_variables
        STATIC_ASSERTION_LIST
        FONT_INFO_LIST
        INCLUDE_LIST)

    __iconfonts_generate_from_template(
        "${ICONFONTS_MODULE_DIR}/iconfontsregistry.cpp.in" "${registry_filepath}"

        LIST_FILEPATH           "${pretty_list_filename}"
        FONT_INFO_LIST          "${known_fonts_info_list}"
        INCLUDE_LIST            "${known_fonts_include_list}"
        STATIC_ASSERTION_LIST   "${known_fonts_assertion_list}"
        VARIABLES                registry_variables)

    target_sources("${TARGET}" PRIVATE "${registry_filepath}")
endfunction(iconfonts_finalize_target)
