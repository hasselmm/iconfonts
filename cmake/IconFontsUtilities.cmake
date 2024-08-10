# ----------------------------------------------------------------------------------------------------------------------
# Initializes usually required properties for `TARGET`
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_set_target_properties TARGET)
    if (NOT TARGET "${TARGET}")
        message(FATAL_ERROR "Invalid target: '${TARGET}'")
    endif()

    get_target_property(binary_dir "${TARGET}" BINARY_DIR)
    string(TOLOWER "${TARGET}" target_dirname)

    get_target_property(next_font_tag "${TARGET}" ICONFONTS_NEXT_FONT_TAG)

    if (NOT next_font_tag)
        set(next_font_tag 1)
    endif()

    set_target_properties(
        "${TARGET}" PROPERTIES

        ICONFONTS_GENERATED_INCLUDE_DIR "${binary_dir}/sources"
        ICONFONTS_GENERATED_SOURCES_DIR "${binary_dir}/sources/${target_dirname}"
        ICONFONTS_RESOURCE_DIR          "${binary_dir}/resources"
        ICONFONTS_NEXT_FONT_TAG         "${next_font_tag}"
    )
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Retreives usually required properties from `TARGET`
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_get_target_properties TARGET PREFIX)
    if (NOT TARGET "${TARGET}")
        message(FATAL_ERROR "Invalid target: '${TARGET}'")
    endif()

    set(mandatory_properties GENERATED_INCLUDE_DIR GENERATED_SOURCES_DIR RESOURCE_DIR NEXT_FONT_TAG)
    set(optional_properties FONT_OPTIONS FONT_NAMESPACES)

    foreach(name IN LISTS mandatory_properties optional_properties)
        set(property "ICONFONTS_${name}")
        get_target_property(value "${TARGET}" "${property}")

        if (NOT value AND NOT value STREQUAL "" AND name IN_LIST mandatory_properties)
            message(FATAL_ERROR "Property ${property} is missing for target ${TARGET}")
        endif()

        set("${PREFIX}_${name}" "${value}" PARENT_SCOPE)
    endforeach()
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Generates an enum member key of `NAME` with `VALUE` and an optional comment.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_add_enumkey OUTPUT_LIST NAME VALUE)
    if (ARGN)
        set(comment " // ${ARGN}")
    else()
        set(comment "")
    endif()

    list(APPEND "${OUTPUT_LIST}" "    ${NAME} = ${VALUE},${comment}\n")
    set("${OUTPUT_LIST}" ${${OUTPUT_LIST}} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Guesses the icon information format from `FONT_FILEPATH` and `INFO_FILEPATH`.
# The result is reported to `OUTPUT_VARIABLE`.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_guess_info_type INFO_FILEPATH FONT_FILEPATH FONT_VARIANT OUTPUT_VARIABLE)
    cmake_path(GET INFO_FILEPATH FILENAME info_filename)
    unset(info_type)

    if (info_filename MATCHES "\\.css$")
        set(info_type "css")
    elseif (info_filename MATCHES "\\.codepoints$")
        set(info_type "codepoints")
    elseif (INFO_FILEPATH MATCHES "(^|/)metadata/icons\\.json$")
        set(info_type "fontawesome")
    elseif (INFO_FILEPATH MATCHES "_foundicons\\.scss$")
        set(info_type "foundicons")
    elseif (info_filename STREQUAL "selection.json")
        set(info_type "icomoon")
    elseif (info_filename STREQUAL "mapping.json")
        set(info_type "mapping")
    elseif (info_filename MATCHES "\\.idl$")
        set(info_type "microsoft-idl")
    endif()

    if (NOT info_type AND FONT_FILEPATH)
        cmake_path(GET FONT_FILEPATH STEM font_basename)

        if (info_filename STREQUAL "${font_basename}.json")
            set(info_type "mapping")
        endif()
    endif()

    if (NOT info_type AND FONT_VARIANT)
        if (info_filename STREQUAL "${FONT_VARIANT}.js")
            set(info_type "mapping")
        endif()
    endif()

    if (NOT info_type)
        message(FATAL_ERROR "Failed to guess icon information type for '${info_filename}'")
    endif()

    set("${OUTPUT_VARIABLE}" "${info_type}" PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Converts the possible webfont at `FONT_FILEPATH` into a proper desktop font in `OUTPUT_DIRECTORY`.
# The desktop font's filepath is reported in `OUTPUT_VARIABLE`.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_convert_webfont FONT_FILEPATH OUTPUT_DIRECTORY OUTPUT_VARIABLE)
    if (FONT_FILEPATH MATCHES "\\.woff")
        cmake_path(GET FONT_FILEPATH STEM basename)

        set(fonttool_filepath "${OUTPUT_DIRECTORY}/${basename}.ttx")
        set(opentype_filepath "${OUTPUT_DIRECTORY}/${basename}.otf")

        add_custom_command(
            OUTPUT "${fonttool_filepath}"
            DEPENDS "${FONT_FILEPATH}"
            COMMAND Python3::Interpreter -m fontTools.ttx -q -o "${fonttool_filepath}" -i "${FONT_FILEPATH}"
            COMMENT "Generating ${fonttool_filepath}")

        add_custom_command(
            OUTPUT "${opentype_filepath}"
            DEPENDS "${fonttool_filepath}"
            COMMAND Python3::Interpreter -m fontTools.ttx -q -o "${opentype_filepath}" "${fonttool_filepath}"
            COMMENT "Generating ${opentype_filepath}")

        set_source_files_properties(
            "${fonttool_filepath}"
            "${opentype_filepath}"
            PROPERTIES GENERATED YES)

        set("${OUTPUT_VARIABLE}" "${opentype_filepath}" PARENT_SCOPE)
    else()
        set("${OUTPUT_VARIABLE}" "${FONT_FILEPATH}" PARENT_SCOPE)
    endif()
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Adds a CMake option for adding the font named by `FONT_FAMILY` and `FONT_VARIANT` to `TARGET`.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_option)
    set(mandatory_values
        TARGET              # the CMake target this font is added to
        FONT_FAMILY)        # the font family name

    set(optional_values
        FONT_VARIANT        # the font variant name
        DEFAULT_VALUE       # default state of the option, `OFF` if not specified
        OUTPUT_VARIABLE)    # variable into which to store the option's value

    set(single_values ${mandatory_values} ${optional_values}) # ------------------------------- parse function arguments
    cmake_parse_arguments(OPTION "" "${single_values}" "" ${ARGN})

    iconfonts_reject_unparsed_arguments(OPTION)
    iconfonts_require_mandatory_arguments(OPTION ${mandatory_values})

    if (NOT OPTION_DEFAULT_VALUE)
        set(OPTION_DEFAULT_VALUE OFF)
    endif()

    set(alphanum "A-Za-z0-9") # ---------------------------------------------------------------------- build option name

    string(REGEX REPLACE "[^${alphanum}]+" "" option_target "${OPTION_TARGET}")
    string(REGEX REPLACE "[^${alphanum}]+" "" option_family "${OPTION_FONT_FAMILY}")

    set(fontname "${OPTION_FONT_FAMILY}")
    set(option_name "${option_target}_ENABLE_${option_family}")

    if (OPTION_FONT_VARIANT)
        string(APPEND fontname " ${OPTION_FONT_VARIANT}")
        string(REGEX REPLACE "[^${alphanum}]+" "" option_variant "${OPTION_FONT_VARIANT}")
        string(APPEND option_name "_${option_variant}")
    endif()

    string(TOUPPER "${option_name}" option_name)

    option("${option_name}" "Bundle ${fontname} font" "${OPTION_DEFAULT_VALUE}") # ------------- define and check option
    set(option_enabled "${${option_name}}")

    if (ICONFONTS_ENABLE_ALL_FONTS)
        set(option_enabled YES)
    endif()

    if (option_enabled)
        message(STATUS "Bundling ${fontname} font")
    endif()

    if (OPTION_OUTPUT_VARIABLE) # ----------------------------------------------------------------------- report results
        set("${OPTION_OUTPUT_VARIABLE}" "${option_enabled}" PARENT_SCOPE)
    endif()

    set_property(
        TARGET "${OPTION_TARGET}" APPEND PROPERTY
        ICONFONTS_FONT_OPTIONS "${option_name}")
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Actual implementation of `iconfonts_show()`.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_show_impl FUNCTION_NAME LINE_NUMBER LOGLEVEL)
    if (NOT LOGLEVEL MATCHES "^(TRACE|VERBOSE|STATUS)\$")
        __iconfonts_show_impl("${FUNCTION_NAME}" "${LINE_NUMBER}" STATUS "${LOGLEVEL}" ${ARGN})
    else()
        list(LENGTH ARGN variable_count)

        if (variable_count GREATER 1)
            message("${LOGLEVEL}" "${FUNCTION_NAME}(), line ${LINE_NUMBER}:")

            foreach(variable IN LISTS ARGN)
                message("${LOGLEVEL}" "  > ${variable} => '${${variable}}'")
            endforeach()
        elseif (variable_count GREATER 0)
            message("${LOGLEVEL}" "${FUNCTION_NAME}(), line ${LINE_NUMBER}: ${ARGN} => '${${ARGN}}'")
        else()
            message("${LOGLEVEL}" "${FUNCTION_NAME}(), line ${LINE_NUMBER}: No variables give")
        endif()
    endif()
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Shows the current values of the given variables
# ----------------------------------------------------------------------------------------------------------------------
macro(iconfonts_show)
    __iconfonts_show_impl("${CMAKE_CURRENT_FUNCTION}" "${CMAKE_CURRENT_FUNCTION_LIST_LINE}" ${ARGN})
endmacro()

# ----------------------------------------------------------------------------------------------------------------------
# Some minimal unit testing
# ----------------------------------------------------------------------------------------------------------------------

if (ICONFONTS_ENABLE_TESTING)
    message(VERBOSE "Testing utility functions...")
    list(APPEND CMAKE_MESSAGE_INDENT "  ")

    unset(test_list) # -------------------------------------------------------------------------------------------------

    __iconfonts_add_enumkey(test_list "First" "0xf001")
    iconfonts_assert("${test_list}" STREQUAL "    First = 0xf001,\n")

    __iconfonts_add_enumkey(test_list "Second" "0xf002" "A comment")
    iconfonts_assert("[${test_list}]" STREQUAL "[    First = 0xf001,\n\;    Second = 0xf002, // A comment\n]")

    __iconfonts_guess_info_type("fonts/style.css" "" "" test_string) # -------------------------------------------------
    iconfonts_assert(test_string STREQUAL "css")

    __iconfonts_guess_info_type("fonts/variant.codepoints" "" "" test_string)
    iconfonts_assert(test_string STREQUAL "codepoints")

    __iconfonts_guess_info_type("metadata/icons.json" "" "" test_string)
    iconfonts_assert(test_string STREQUAL "fontawesome")

    __iconfonts_guess_info_type("fonts/metadata/icons.json" "" "" test_string)
    iconfonts_assert(test_string STREQUAL "fontawesome")

    __iconfonts_guess_info_type("sass/social_foundicons.scss" "" "" test_string)
    iconfonts_assert(test_string STREQUAL "foundicons")

    __iconfonts_guess_info_type("fonts/selection.json" "" "" test_string)
    iconfonts_assert(test_string STREQUAL "icomoon")

    __iconfonts_guess_info_type("fonts/mapping.json" "" "" test_string)
    iconfonts_assert(test_string STREQUAL "mapping")

    __iconfonts_guess_info_type("fonts/test.json" "fonts/test.ttf" "" test_string)
    iconfonts_assert(test_string STREQUAL "mapping")

    __iconfonts_guess_info_type("webfont/regular.js" "webfont/fonts/test-regular.ttf" "regular" test_string)
    iconfonts_assert(test_string STREQUAL "mapping")

    __iconfonts_guess_info_type("controls/microsoft.ui.xaml.controls.controls2.idl" "" "" test_string)
    iconfonts_assert(test_string STREQUAL "microsoft-idl")

    list(POP_BACK CMAKE_MESSAGE_INDENT)
endif(ICONFONTS_ENABLE_TESTING)
