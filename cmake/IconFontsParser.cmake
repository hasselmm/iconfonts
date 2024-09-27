# ----------------------------------------------------------------------------------------------------------------------
# Collects icon definitions from `INFO_FILEPATH` using the Python tool.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_collect_icons_python OUTPUT_VARIABLE INFO_FILEPATH)
    if (NOT TARGET Python3::Interpreter)
        message(FATAL_ERROR "Python required")
    endif()

    execute_process(
        COMMAND "${Python3_EXECUTABLE}" "${ICONFONTS_TOOL_EXECUTABLE}" "${INFO_FILEPATH}"
        OUTPUT_VARIABLE icon_definitions COMMAND_ERROR_IS_FATAL ANY)

    set("${OUTPUT_VARIABLE}" ${icon_definitions} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Extracts additional parser options from `OPTIONS_VARIABLE` and stores in variables starting with `PREFIX`.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_parse_info_options PREFIX OPTIONS_VARIABLE)
    set(single_values
        prefix              # the prefix of the CSS rules to consider
        suffix              # the suffix of the CSS rules to consider
        filename            # filename of the stylesheet to parse
        mapping)            # name of the Javascript table containing glyph mapping

    set(multi_values
        sizes               # possible sizes of the font
        variant)            # an explicit font variant name

    string(REGEX REPLACE "[=,\t\r\n ]+" ";" options_list "${${OPTIONS_VARIABLE}}")
    cmake_parse_arguments(info "" "${single_values}" "${multi_values}" ${options_list})
    iconfonts_reject_unparsed_arguments(info)

    list(JOIN info_variant " " info_variant)

    foreach(option IN LISTS single_values multi_values)
        set("${PREFIX}_${option}" "${info_${option}}" PARENT_SCOPE)
    endforeach()
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Collects icon definitions from simple JSON mapping in `INFO_FILEPATH`.
# When passing "mapping=variable-name" in options `INFO_FILEPATH` is assumed
# to be Javascript, and the JSON mapping is extracted from that code.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_collect_icons_mapping OUTPUT_VARIABLE INFO_FILEPATH OPTIONS)
    __iconfonts_parse_info_options(options OPTIONS)
    file(READ "${INFO_FILEPATH}" code)

    if (options_mapping) # --------------------------------------------------- extract JSON mapping from Javascript code
        iconfonts_encode_regex("${options_mapping}" escaped_mapping)

        set(ws "[ \t\r\n]")
        set(mapping_start   ".*(const|let|var)${ws}+${escaped_mapping}${ws}*=${ws}*{")
        set(mapping_badkey  "${ws}*([A-Za-z0-9_]+)${ws}*:")
        set(mapping_end     "}.*")

        string(REGEX REPLACE "${mapping_start}"  "{"        code "${code}")
        string(REGEX REPLACE "${mapping_end}"    "}"        code "${code}")
        string(REGEX REPLACE "${mapping_badkey}" "\"\\1\":" code "${code}")
    endif()

    string(JSON count LENGTH "${code}") # ----------------------------------------------------- process the JSON mapping
    math(EXPR last "${count} - 1")

    foreach(index RANGE "${last}")
        string(JSON name MEMBER "${code}" "${index}")
        string(JSON unicode GET "${code}" "${name}")

        __iconfonts_make_symbol("${name}" name)
        __iconfonts_add_enumkey(icon_definitions "${name}" "${unicode}")
    endforeach()

    set("${OUTPUT_VARIABLE}" ${icon_definitions} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Reads information in FontAwesome format into `OUTPUT_VARIABLE` from `INFO_FILEPATH`.
#
# These files are several megabytes in size, which makes them painfully slow to process with CMake's
# builtin JSON parsing routines. Currently they parse the entire JSON string with each call. Therefore
# attempts are made to remove all unused information from that JSON. This step takes time and is fragile,
# but it is worth the effort: The JSON's size is reduced by more than 95%, and therefore processing time
# is reduced from e.g. 2:10 minutes to 19 seconds.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_fontawesome_read_metadata OUTPUT_VARIABLE INFO_FILEPATH)
    cmake_path(GET INFO_FILEPATH PARENT_PATH basedir)
    set(compact_filename "${basedir}/icons-compact.json")

    if ("${INFO_FILEPATH}" IS_NEWER_THAN "${compact_filename}"
        OR "${CMAKE_CURRENT_FUNCTION_LIST_FILE}" IS_NEWER_THAN "${compact_filename}")
        file(READ "${INFO_FILEPATH}" iconinfo)

        iconfonts_timestamp(start)

        string(LENGTH "${iconinfo}" fullsize)
        math(EXPR fullsize "${fullsize} / 1024")

        string(REGEX REPLACE "[\t\n ]" "" iconinfo "${iconinfo}")
        string(REPLACE "\\\"" "'"         iconinfo "${iconinfo}")

        iconfonts_condense(iconinfo "\"(voted)\":(true|false),?")
        iconfonts_condense(iconinfo "\"(last_modified|height|width)\":[0-9]+,?")
        iconfonts_condense(iconinfo "\"(label|raw|path)\":\"[^\"]*\",?")
        iconfonts_condense(iconinfo "\"(changes|composite|ligatures|primary|secondary|terms|viewBox)\":\\[[^]]*\\],?")
        iconfonts_condense(iconinfo "\"(aliases|brands|regular|search|solid|svg|unicodes)\":{},?")

        string(REGEX REPLACE ",+}" "}" iconinfo "${iconinfo}")

        iconfonts_duration(start duration)

        file(WRITE "${compact_filename}" "${iconinfo}")

        string(LENGTH "${iconinfo}" smallsize)
        math(EXPR smallsize "${smallsize} / 1024")

        message(VERBOSE "Condensing icon metadata from ${fullsize} kiB to ${smallsize} kiB: ${duration} ms")
    else()
        message(VERBOSE "Reusing already condensed icon metadata")
        file(READ "${compact_filename}" iconinfo)
    endif()

    set("${OUTPUT_VARIABLE}" "${iconinfo}" PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Collects icon information from JSON string `ICONINFO` for `LICENSE` and `STYLENAME`.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_fontawesome_parse_metadata OUTPUT_VARIABLE ICONINFO LICENSE STYLENAME)
    string(JSON count LENGTH "${ICONINFO}")

    math(EXPR last "${count} - 1")
    math(EXPR step "${count} / 8")

    foreach(index RANGE ${last})
        math(EXPR progress "${index} % ${step}") # -------------------------------------------- show conversion progress

        if (progress EQUAL 0 OR index EQUAL last)
            math(EXPR next "${index} + 1")
            math(EXPR percent "100 * ${next} / ${count}")
            message(STATUS "${next}/${count} icons processed (${percent}%)")
        endif()

        string(JSON name    MEMBER "${iconinfo}"  ${index}) # --------------------------------- extract icon information
        string(JSON info    GET    "${iconinfo}" "${name}")
        string(JSON unicode GET    "${info}"     "unicode")

        if (LICENSE STREQUAL "free")
            string(JSON styles GET "${info}" "free")
        else()
            string(JSON styles GET "${info}" "styles")
        endif()

        __iconfonts_make_symbol("${name}" name)
        iconfonts_json_to_list(supported_style_list "${styles}")
        list(JOIN supported_style_list ", " comment)


        if ("${STYLENAME}" IN_LIST supported_style_list) # --------------------------------------- build icon definition
            __iconfonts_add_enumkey(icon_definitions "${name}" "0x${unicode}" "${comment}")

            string(
                JSON alias_names
                ERROR_VARIABLE alias_errors
                GET "${info}" aliases names)

            if (NOT alias_errors)
                iconfonts_json_to_list(alias_names "${alias_names}")

                foreach(alias IN LISTS alias_names)
                    __iconfonts_make_symbol("${alias}" alias)
                    __iconfonts_add_enumkey(icon_definitions "${alias}" "${name}")
                endforeach()
            endif()
        else()
            list(APPEND icon_definitions "    // ${name}: ${comment}\n")
        endif()
    endforeach()

    set("${OUTPUT_VARIABLE}" ${icon_definitions} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Collects icon definitions for `FONT_VARIANT` from Fontawesome icons file in `INFO_FILEPATH`.
# If possible an ultra-fast Python-based parser is run; otherwise a plain CMake solution is used.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_collect_icons_fontawesome OUTPUT_VARIABLE INFO_FILEPATH FONT_VARIANT)
    if (FONT_VARIANT MATCHES "^Brands-")
        set(license   "free")
        set(stylename "brands")
    elseif (FONT_VARIANT MATCHES "^([^-]+)-(.*)")
        string(TOLOWER "${CMAKE_MATCH_1}" license)
        string(TOLOWER "${CMAKE_MATCH_2}" stylename)
    else()
        message(FATAL_ERROR "Unsupported font variant: '${FONT_VARIANT}'")
    endif()

    iconfonts_timestamp(start)

    if (TARGET Python3::Interpreter)
        execute_process(
            COMMAND "${Python3_EXECUTABLE}" "${ICONFONTS_TOOL_EXECUTABLE}"
            "${INFO_FILEPATH}" "${license}" "${stylename}"
            OUTPUT_VARIABLE icon_definitions
            COMMAND_ERROR_IS_FATAL ANY)
    else()
        cmake_path(
            RELATIVE_PATH INFO_FILEPATH
            BASE_DIRECTORY "${CMAKE_BINARY_DIR}"
            OUTPUT_VARIABLE relative_INFO_FILEPATH)

        message(WARNING "No Python interpreter found; reading ${relative_INFO_FILEPATH} will be slow.")

        __iconfonts_fontawesome_read_metadata(iconinfo "${INFO_FILEPATH}")
        __iconfonts_fontawesome_parse_metadata(icon_definitions "${iconinfo}" "${license}" "${stylename}")
    endif()

    set("${OUTPUT_VARIABLE}" ${icon_definitions} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Collects icon definitions from `INFO_FILEPATH` in Foundicons' SCSS format.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_collect_icons_foundicons OUTPUT_VARIABLE INFO_FILEPATH)
    file(STRINGS "${INFO_FILEPATH}" scss_defines REGEX "@include i-class")

    foreach(scss IN LISTS scss_defines)
        if (scss MATCHES "@include i-class\\((.*),\"(.*)\"\\)")
            __iconfonts_make_symbol("${CMAKE_MATCH_1}" name)
            set(unicode "0xf${CMAKE_MATCH_2}")
            __iconfonts_add_enumkey(icon_definitions "${name}" "${unicode}")
        endif()
    endforeach()

    set("${OUTPUT_VARIABLE}" ${icon_definitions} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Collects icon definitions from MaterialSymbols codepoints file `INFO_FILEPATH`.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_collect_icons_materialsymbols OUTPUT_VARIABLE INFO_FILEPATH)
    if (TARGET Python3::Interpreter)
        __iconfonts_collect_icons_python("${OUTPUT_VARIABLE}" "${INFO_FILEPATH}")
    else()
        file(STRINGS "${INFO_FILEPATH}" codepoint_list)
        set(name_list)

        foreach(codepoint IN LISTS codepoint_list)
            string(REGEX REPLACE " +" ";" codepoint "${codepoint}")

            list(GET codepoint 0 name)
            list(GET codepoint 1 unicode)

            __iconfonts_make_symbol("${name}" name)

            if (name IN_LIST name_list) # --------------------------------- find alternate name for confliciting symbols
                message(STATUS "Searching alternate name for ${name}...")

                foreach(i RANGE 0 999)
                    if (i GREATER 0)
                        set(alt_name "${name}Alt${i}")
                    else()
                        set(alt_name "${name}Alt")
                    endif()

                    if (NOT alt_name IN_LIST name_list)
                        set(name "${alt_name}")
                        unset(alt_name)
                        break()
                    endif()
                endforeach()

                if (alt_name)
                    message(FATAL_ERROR "Could not find alternate name for ${name}")
                    return()
                endif()
            endif()

            __iconfonts_add_enumkey(icon_definitions "${name}" "0x${unicode}")

            list(APPEND name_list "${name}")
        endforeach()
    endif()

    set("${OUTPUT_VARIABLE}" ${icon_definitions} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Collects icon definitions from `ENUM` in `NAMESPACE` of the Microsoft IDL file `INFO_FILEPATH`.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_collect_icons_microsoft_idl OUTPUT_VARIABLE INFO_FILEPATH FONT_VARIANT NAMESPACE ENUM)
    # FIXME at some point we dropped that very long file name, but somehow we pass the wrong filename here
    string(REPLACE "dxaml/xcp/dxaml/idl/winrt/controls/" "" INFO_FILEPATH "${INFO_FILEPATH}")

    string(REPLACE "." "\\." "${NAMESPACE}" escaped_namespace)

    set(namespace "namespace[ \t]+${escaped_namespace}")
    set(enumstart "enum[ \t]+${ENUM}")
    set(enumkey   "([A-Za-z0-9]+)[ \t]*(=[ \t]*([^,]+))?")
    set(enumfinal "}")

    set(filter "^[ \t]*(${namespace}|${enumstart}|${enumkey}|${enumfinal})")

    file(STRINGS "${INFO_FILEPATH}" source_lines REGEX "${filter}")

    set(icon_definitions)
    set(namespace_needed YES)
    set(enumstart_needed YES)
    set(value -1)

    foreach(line IN LISTS source_lines)
        if (namespace_needed)
            if (line MATCHES "${namespace}")
                set(namespace_needed NO)
            endif()
        elseif (enumstart_needed)
            if (line MATCHES "${enumstart}")
                set(enumstart_needed NO)
            endif()
        elseif (line MATCHES "${enumfinal}")
            break()
        else()
            if (line MATCHES "^[ \t]*${enumkey}")
                if (CMAKE_MATCH_3)
                    set(value "${CMAKE_MATCH_3}")
                else()
                    math(EXPR value "${value} + 1")
                endif()

                __iconfonts_add_enumkey(icon_definitions "${CMAKE_MATCH_1}" "${value}")
            endif()
        endif()
    endforeach()

    set("${OUTPUT_VARIABLE}" ${icon_definitions} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Collects icon definitions from Cascading Stylesheet in `INFO_FILEPATH`.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_collect_icons_css OUTPUT_VARIABLE INFO_FILEPATH OPTIONS)
    __iconfonts_parse_info_options(css OPTIONS)
    iconfonts_require_mandatory_arguments(css prefix)

    iconfonts_encode_regex("${css_prefix}" prefix)
    iconfonts_encode_regex("${css_suffix}" suffix)

    set(ws          "[\t ]")
    set(nws         "[^\t ]")
    set(qm          "[\"']")
    set(selector    "${ws}*${prefix}(${nws}+)${suffix}:before${ws}*{")
    set(attribute   "${ws}*content${ws}*:${ws}*(${qm}${nws}+)")

    file(
        STRINGS "${INFO_FILEPATH}" css_lines
        REGEX "^${selector}(${attribute})?|^${attribute}")

    set(name)
    set(unicode)

    foreach(css IN LISTS css_lines)
        set(line_matched FALSE)

        if (css MATCHES "${selector}") # ------------------------------------------- extract icon name from CSS selector
            unset(unicode) # a new glyph has started
            __iconfonts_make_symbol("${CMAKE_MATCH_1}" name)
            set(line_matched TRUE)
        endif()

        if (css MATCHES "${attribute}") # --------------------------------- extract Unicode codepoint from CSS attribute
            set(content "${CMAKE_MATCH_1}")
            set(line_matched TRUE)

            if (content MATCHES "(${qm})\\\\([0-9A-Fa-f]+)(${qm})"
                    AND CMAKE_MATCH_1 STREQUAL CMAKE_MATCH_3)
                set(unicode "0x${CMAKE_MATCH_2}")
            else()
                message(FATAL_ERROR "Unexpected content attributes '${content}' in line '${css}' of '${INFO_FILEPATH}'")
            endif()
        endif()

        if (NOT line_matched) # --------------------------------------------------------------- fail on unexpected lines
            message(FATAL_ERROR "Unexpected line '${css}' in '${INFO_FILEPATH}'")
        endif()

        if (name AND unicode) # --------------------------------------------------------------- generate icon definition
            set(cached_unicode "${symbol_cache_${name}}")

            if (NOT cached_unicode)
                __iconfonts_add_enumkey(icon_definitions "${name}" "${unicode}")
                set("symbol_cache_${name}" "${unicode}")
            elseif (NOT "${cached_unicode}" STREQUAL "${unicode}")
                message(
                    FATAL_ERROR "Conflicting entries for symbol '${name}'"
                    " ('${cached_unicode}' and '${unicode}') in '${INFO_FILEPATH}'")
            endif()

            unset(unicode)
            unset(name)
        endif()
    endforeach()

    if (css_sizes) # --------------------------------------------------- validate that expected sizes are actually sound
        string(REGEX REPLACE "[()]+" "" size_check "${selector}")
        string(REGEX REPLACE "[0-9]+" "([0-9]+)" size_check "${size_check}")

        file(
            STRINGS "${INFO_FILEPATH}" actual_sizes
            REGEX "^${size_check}")

        list(TRANSFORM actual_sizes REPLACE "^${size_check}.*" "\\1")
        list(REMOVE_DUPLICATES actual_sizes)
        list(SORT actual_sizes)

        if (NOT css_sizes STREQUAL actual_sizes)
            message(FATAL_ERROR
                "Symbol sizes do not match for ${INFO_FILEPATH} "
                "(expected sizes: ${css_sizes}, actual sizes: ${actual_sizes})")
            return()
        endif()
    endif()

    set("${OUTPUT_VARIABLE}" ${icon_definitions} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Collects icon definitions from IcoMoon icon definition in `INFO_FILEPATH`.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_collect_icons_icomoon OUTPUT_VARIABLE INFO_FILEPATH)
    __iconfonts_collect_icons_python(icon_definitions "${INFO_FILEPATH}")
    set("${OUTPUT_VARIABLE}" ${icon_definitions} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Collects icon definitions in `FORMAT`from `FILENAME` for `FONT_VARIANT`.
# `FILENAME` is considered relative to `DIRECTORY`.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_collect_icons)
    set(mandatory_values
        FILEPATH            # the icon definition file to process
        FILETYPE            # icon definition format of `FILENAME`
        OUTPUT_VARIABLE)    # output variable for the icon definition list

    set(optional_values
        FONT_VARIANT        # the font variant to extract
        OPTIONS)            # additional file format specific parser options

    set(single_values ${mandatory_values} ${optional_values}) # ------------------------------- parse function arguments
    cmake_parse_arguments(COLLECT "" "${single_values}" "" ${ARGN})

    iconfonts_reject_unparsed_arguments(COLLECT)
    iconfonts_require_mandatory_arguments(COLLECT ${mandatory_values})

    iconfonts_timestamp(start) # ----------------------------------------------------- actually collect icon definitions

    # FIXME Maybe read COLLECT_FILEPATH here already and pass resulting string to functions.
    # This will enable unit testing.

    if (COLLECT_FILETYPE STREQUAL "codepoints")
        __iconfonts_collect_icons_materialsymbols(icon_definitions "${COLLECT_FILEPATH}")
    elseif (COLLECT_FILETYPE STREQUAL "css")
        __iconfonts_collect_icons_css(icon_definitions "${COLLECT_FILEPATH}" "${COLLECT_OPTIONS}")
    elseif (COLLECT_FILETYPE STREQUAL "fontawesome")
        __iconfonts_collect_icons_fontawesome(icon_definitions "${COLLECT_FILEPATH}" "${COLLECT_FONT_VARIANT}")
    elseif (COLLECT_FILETYPE STREQUAL "foundicons")
        __iconfonts_collect_icons_foundicons(icon_definitions "${COLLECT_FILEPATH}")
    elseif (COLLECT_FILETYPE STREQUAL "icomoon")
        __iconfonts_collect_icons_icomoon(icon_definitions "${COLLECT_FILEPATH}")
    elseif (COLLECT_FILETYPE STREQUAL "mapping")
        __iconfonts_collect_icons_mapping(icon_definitions "${COLLECT_FILEPATH}" "${COLLECT_OPTIONS}")
    elseif (COLLECT_FILETYPE STREQUAL "microsoft-idl")
        __iconfonts_collect_icons_microsoft_idl(
            icon_definitions "${COLLECT_FILEPATH}" "${COLLECT_FONT_VARIANT}"
            "Microsoft.UI.Xaml.Controls" "Symbol")
    else()
        message(FATAL_ERROR "Unknown icon info format ${COLLECT_FILETYPE} for '${COLLECT_FILEPATH}'")
    endif()

    iconfonts_duration_with_unit(start duration) # ------------------------------------------------------ report results

    # The `execute_process()` function returns a string by default, not a list.
    # There we count assignments in `icon_definitions`, instead of just calling `list(LENGTH)`.
    if (icon_definitions)
        string(REGEX REPLACE "[^=]" "" icon_count ${icon_definitions})
        string(LENGTH "${icon_count}" icon_count)
        message(STATUS "${icon_count} icons collected within ${duration}")
    else()
        message(FATAL_ERROR "No icons collected for '${COLLECT_FILEPATH}'")
        set(icon_count "NO")
    endif()


    set("${COLLECT_OUTPUT_VARIABLE}" ${icon_definitions} PARENT_SCOPE)
endfunction()
