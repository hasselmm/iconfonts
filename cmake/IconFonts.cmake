include(IconFontsAssert)
include(IconFontsChrono)
include(IconFontsCodeGenerator)
include(IconFontsNetwork)
include(IconFontsParser)
include(IconFontsPython)
include(IconFontsStrings)
include(IconFontsUtilities)

# ----------------------------------------------------------------------------------------------------------------------
# A command line tool for processing icon font metadata.
# ----------------------------------------------------------------------------------------------------------------------
iconfonts_add_python_script(IconFontsTool "${ICONFONTS_TOOL_EXECUTABLE}")

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
    list(TRANSFORM known_fonts_include_list TOLOWER)
    list(TRANSFORM known_fonts_include_list PREPEND "#include \"")
    list(TRANSFORM known_fonts_include_list APPEND ".h\"")
    list(JOIN known_fonts_include_list "\n" known_fonts_include_list)

    set(known_fonts_type_list ${FINALIZE_FONT_NAMESPACES})
    list(TRANSFORM known_fonts_type_list PREPEND "        FontInfo::instance<Symbols::")
    list(TRANSFORM known_fonts_type_list APPEND "::Symbol>(),")
    list(JOIN known_fonts_type_list "\n" known_fonts_type_list)

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
    set(registry_variables FONT_TYPE_LIST INCLUDE_LIST)

    __iconfonts_generate_from_template(
        "${ICONFONTS_MODULE_DIR}/iconfontsregistry.cpp.in" "${registry_filepath}"

        LIST_FILEPATH           "${pretty_list_filename}"
        FONT_TYPE_LIST          "${known_fonts_type_list}"
        INCLUDE_LIST            "${known_fonts_include_list}"
        VARIABLES                registry_variables)

    target_sources("${TARGET}" PRIVATE "${registry_filepath}")
endfunction(iconfonts_finalize_target)

# FIXME doxs
function(iconfonts_add_font)
    set(options
        OPTIONAL                # the option for enabling this font is disabled by default for OPTIONAL fonts
        SKIP_RESOURCES)         # do not generate Qt resources

    set(mandatory_values
        TARGET                  # the target to which to add this font variant
        BASE_URL                # common base URL for all the paths passed
        FONT_FAMILY             # the font family this font belongs to
        INFO_FILEPATH           # path of the icon description; releative to `BASE_URL`
        INFO_FILEHASH           # SHA1 hash for `INFO_FILEPATH`
        LICENSE_FILEPATH        # path of the font's license file; releative to `BASE_URL`
        LICENSE_FILEHASH        # SHA1 hash for `LICENSE_FILEPATH`
        RESOURCE_PREFIX)        # prefix of the Qt resources; if not specified no resources are generated

    set(optional_values
        ARCHIVE                 # path or URL of an archive to download, instead of directly accessing the web
        ARCHIVE_FILEHASH        # SHA1 hash for `ARCHIVE`
        FONT_VARIANT            # the font variant within the font's family
        FONT_FILEPATH           # path of the font file; releative to `BASE_URL`
        FONT_FILEHASH           # SHA1 hash for `FONT_FILEPATH`
        INFO_FILETYPE           # file format of `INFO_FILEPATH`
        INFO_OPTIONS            # additional parser options for `INFO_FILEPATH`
        OUTPUT_RESOURCES_LIST)  # the resources generated by this function are reported to this variable

    set(single_values ${mandatory_values} ${optional_values}) # ------------------------------- parse function arguments
    cmake_parse_arguments(ICONFONTS "${options}" "${single_values}" "" ${ARGN})

    iconfonts_reject_unparsed_arguments(ICONFONTS)
    iconfonts_require_mandatory_arguments(ICONFONTS ${mandatory_values})

    if (ICONFONTS_OPTIONAL)
        set(ICONFONTS_DEFAULT_VALUE OFF)
    else()
        set(ICONFONTS_DEFAULT_VALUE ON)
    endif()

    if (NOT ICONFONTS_INFO_TYPE)
        __iconfonts_guess_info_type(
            "${ICONFONTS_INFO_FILEPATH}" "${ICONFONTS_FONT_FILEPATH}"
            "${ICONFONTS_FONT_VARIANT}" ICONFONTS_INFO_FILETYPE)
    endif()

    __iconfonts_set_target_properties("${ICONFONTS_TARGET}")
    __iconfonts_get_target_properties("${ICONFONTS_TARGET}" ICONFONTS)

    if (ICONFONTS_BASE_URL MATCHES "^https://github\.com(/.*)\$")  # ------------------------------------ parse BASE_URL
        set(github_details "${CMAKE_MATCH_1}")

        message(TRACE "github_details: '${github_details}'")

        if (github_details MATCHES "^/([^/@]+)(/.*)\$")
            set(github_account "${CMAKE_MATCH_1}")
            set(github_details "${CMAKE_MATCH_2}")
        else()
            message(FATAL_ERROR "Github URL detected, but the account name is missing: ${ICONFONTS_BASE_URL}")
        endif()

        message(TRACE "github_account: '${github_account}'")

        if (github_details MATCHES "^/([^/@]+)([@/].*)\$")
            set(github_repository "${CMAKE_MATCH_1}")
            set(github_details "${CMAKE_MATCH_2}")
        else()
            message(FATAL_ERROR "Github URL detected, but the repository name is missing: ${ICONFONTS_BASE_URL}")
        endif()

        message(TRACE "github_repository: '${github_repository}'")

        if (github_details MATCHES "^@(.+)\$")
            set(github_revision "${CMAKE_MATCH_1}")
        else()
            message(FATAL_ERROR "Github URL detected, but the revision is missing: ${ICONFONTS_BASE_URL}")
        endif()

        message(TRACE "github_revision: '${github_revision}'")

        set(ICONFONTS_BASE_URL "https://github.com/${github_account}/${github_repository}/")
    elseif (NOT ICONFONTS_BASE_URL MATCHES "/\$")
        message(FATAL_ERROR "Invalid value for BASE_URL")
    endif()

    function(make_url PATH OUTPUT_VARIABLE)
        if (github_revision)
            if (PATH MATCHES "^releases/(.*)")
                set(PATH "releases/download/${github_revision}/${CMAKE_MATCH_1}")
            else()
                string(PREPEND PATH "raw/${github_revision}/")
            endif()
        endif(github_revision)

        iconfonts_encode_url("${PATH}" encoded_path SKIP /)
        set("${OUTPUT_VARIABLE}" "${ICONFONTS_BASE_URL}${encoded_path}" PARENT_SCOPE)
    endfunction(make_url)

    __iconfonts_option( # ---------------------------------------------------------- check whether to consider this font
        TARGET          "${ICONFONTS_TARGET}"
        FONT_FAMILY     "${ICONFONTS_FONT_FAMILY}"
        FONT_VARIANT    "${ICONFONTS_FONT_VARIANT}"
        DEFAULT_VALUE   "${ICONFONTS_DEFAULT_VALUE}"
        OUTPUT_VARIABLE  font_enabled)

    if (NOT font_enabled)
        return()
    endif()

    list(APPEND CMAKE_MESSAGE_INDENT "  ")
    unset(resources_list)

    set(resource_dirpath "${ICONFONTS_RESOURCE_DIR}") # ----------------------------------------------- build file paths

    __iconfonts_make_symbol("${ICONFONTS_FONT_FAMILY}"  font_family_symbol)
    string(APPEND resource_dirpath "/${font_family_symbol}")
    set(info_dirpath "${resource_dirpath}")

    if (ICONFONTS_FONT_VARIANT)
        __iconfonts_make_symbol("${ICONFONTS_FONT_VARIANT}" font_variant_symbol)
        string(APPEND info_dirpath "/${font_variant_symbol}")
    endif()

    if (ICONFONTS_ARCHIVE) # ----------------------------------------------------------------- maybe download an archive
        if (ICONFONTS_ARCHIVE MATCHES "npm:(.*)/([^/]+)\$")
            iconfonts_query_npm_package(npminfo "${CMAKE_MATCH_1}" "${CMAKE_MATCH_2}" "dist/tarball" "dist/shasum")
            iconfonts_assert(ICONFONTS_ARCHIVE_FILEHASH STREQUAL "SHA1=${npminfo_dist_shasum}")
            set(archive_url "${npminfo_dist_tarball}")
        else()
            make_url("${ICONFONTS_ARCHIVE}" archive_url)
        endif()

        cmake_path(GET archive_url FILENAME archive_filename)

        iconfonts_download_archive(
            URL             "${archive_url}"
            EXPECTED_HASH   "${ICONFONTS_ARCHIVE_FILEHASH}"
            FILENAME        "${resource_dirpath}/${archive_filename}"
            DESTINATION     "${resource_dirpath}"
            PATTERNS        "${ICONFONTS_FONT_FILEPATH}"
                            "${ICONFONTS_INFO_FILEPATH}")
    endif()

    cmake_path(GET ICONFONTS_INFO_FILEPATH FILENAME info_filename) # ------------------------- download icon information

    if (ICONFONTS_ARCHIVE)
        set(info_filepath "${resource_dirpath}/${ICONFONTS_INFO_FILEPATH}")
        message(STATUS "Validating ${info_filepath}")
        iconfonts_validate_file("${info_filepath}" "${ICONFONTS_INFO_FILEHASH}" validation_result)
        iconfonts_assert(validation_result STREQUAL "ok")
    else()
        set(info_filepath "${info_dirpath}/${info_filename}")
        make_url("${ICONFONTS_INFO_FILEPATH}" info_url)
        iconfonts_download("${info_url}" "${ICONFONTS_INFO_FILEHASH}" "${info_filepath}")
    endif()

    if (ICONFONTS_FONT_FILEPATH) # ------------------------------------------------------------------ download font file
        cmake_path(GET ICONFONTS_FONT_FILEPATH FILENAME font_filename)

        if (ICONFONTS_ARCHIVE)
            set(font_filepath "${resource_dirpath}/${ICONFONTS_FONT_FILEPATH}")
            message(STATUS "Validating ${font_filepath}")
            iconfonts_validate_file("${font_filepath}" "${ICONFONTS_FONT_FILEHASH}" validation_result)
            iconfonts_assert(validation_result STREQUAL "ok")
        else()
            set(font_filepath "${resource_dirpath}/${font_filename}")
            make_url("${ICONFONTS_FONT_FILEPATH}" font_url)
            iconfonts_download("${font_url}" "${ICONFONTS_FONT_FILEHASH}" "${font_filepath}")
        endif()

        __iconfonts_convert_webfont("${font_filepath}" "${resource_dirpath}" opentype_filepath)

        cmake_path(GET opentype_filepath FILENAME opentype_filename)

        set_property(
            SOURCE "${opentype_filepath}"
            PROPERTY QT_RESOURCE_ALIAS "${opentype_filename}")

        list(APPEND resources_list "${opentype_filepath}")
    endif()

    if (ICONFONTS_RESOURCE_PREFIX) # ------------------------------------------------------------- download license file
        make_url("${ICONFONTS_LICENSE_FILEPATH}" license_url)

        __iconfonts_download_license_file(
            "${license_url}" "${ICONFONTS_LICENSE_FILEHASH}"
            "${resource_dirpath}" license_resource_filepath)

        list(APPEND resources_list "${license_resource_filepath}")
    endif()

    __iconfonts_generate_source_code( # ----------------------------------------------------------- generate source code
        TARGET              "${ICONFONTS_TARGET}"
        FONT_FAMILY         "${ICONFONTS_FONT_FAMILY}"
        FONT_VARIANT        "${ICONFONTS_FONT_VARIANT}"
        FONT_FILEPATH       "${opentype_filepath}"
        INFO_FILEPATH       "${info_filepath}"
        INFO_OPTIONS        "${ICONFONTS_INFO_OPTIONS}"
        INFO_FILETYPE       "${ICONFONTS_INFO_FILETYPE}"
        LICENSE_FILEPATH    "${ICONFONTS_LICENSE_FILEPATH}"
    )

    if (NOT ICONFONTS_SKIP_RESOURCES) # ------------------------------------------------------ generate Qt resource file
        qt_add_resources(
            "${ICONFONTS_TARGET}" "${font_family_symbol}"
            PREFIX  "${ICONFONTS_RESOURCE_PREFIX}"
            BASE    "${resource_dirpath}"
            FILES    ${resources_list}
            BIG_RESOURCES)
    endif()

    set(deferred_call_id "iconfonts_finalize_target(${ICONFONTS_TARGET})") # --------------------- finalize target later
    set(deferred_call "iconfonts_finalize_target \"${ICONFONTS_TARGET}\"")

    cmake_language(DEFER CANCEL_CALL "${deferred_call_id}")
    cmake_language(EVAL CODE "cmake_language(DEFER ID \"${deferred_call_id}\" CALL ${deferred_call})")

    list(POP_BACK CMAKE_MESSAGE_INDENT)

    if (ICONFONTS_OUTPUT_RESOURCES_LIST) # -------------------------------------------------------------- report results
        set("${ICONFONTS_OUTPUT_RESOURCES_LIST}" "${resources_list}" PARENT_SCOPE)
    endif()
endfunction(iconfonts_add_font)

# FIXME doxs
function(iconfonts_add_font_family)
    set(options
        OPTIONAL)               # the option for enabling this font is disabled by default for OPTIONAL fonts

    set(mandatory_values
        TARGET                  # the target to which to add this font variant
        BASE_URL                # common base URL for all the paths passed
        FONT_FAMILY             # the font family this font belongs to
        LICENSE_FILEPATH        # path of the font's license file; releative to `BASE_URL`
        LICENSE_FILEHASH)       # SHA1 hash for `LICENSE_FILEPATH`

    set(optional_values
        ARCHIVE                 # path or URL of an archive to download, instead of directly accessing the web
        ARCHIVE_FILEHASH        # SHA1 hash for `ARCHIVE`
        INFO_FILETYPE           # file format of `INFO_FILEPATH`
        RESOURCE_PREFIX         # prefix of the Qt resources; if not specified no resources are generated
        VARIANT_PATTERN)        # regular expression to extract font variants from filenames

    set(list_values
        FONT_VARIANTS)          # description of the font variants within the font's family

    set(single_values ${mandatory_values} ${optional_values}) # ------------------------------- parse function arguments
    cmake_parse_arguments(ICONFONTS "${options}" "${single_values}" "${list_values}" ${ARGN})

    iconfonts_reject_unparsed_arguments(ICONFONTS)
    iconfonts_require_mandatory_arguments(ICONFONTS ${mandatory_values} FONT_VARIANTS)
    iconfonts_unescape(ICONFONTS_VARIANT_PATTERN)

    if (ICONFONTS_OPTIONAL)
        set(ICONFONTS_OPTIONAL "OPTIONAL")
    else()
        unset(ICONFONTS_OPTIONAL)
    endif()

    unset(combined_resources_list) # ------------------------------------------------------------- collect font variants

    foreach(variant_spec IN LISTS ICONFONTS_FONT_VARIANTS)
        string(REGEX REPLACE "[ \n\r\t]*:" ";" variant_spec "${variant_spec}:")

        list(GET variant_spec 0 font_filepath)
        list(GET variant_spec 1 font_filehash)
        list(GET variant_spec 2 info_filepath)
        list(GET variant_spec 3 info_filehash)
        list(GET variant_spec 4 info_options)

        if (info_options)
            __iconfonts_parse_info_options(info info_options)
        else()
            unset(info_variant)
        endif()

        cmake_path(GET font_filepath STEM font_basename)

        if (info_variant)
            set(font_variant "${info_variant}")
        elseif (ICONFONTS_VARIANT_PATTERN
                AND font_basename MATCHES "${ICONFONTS_VARIANT_PATTERN}"
                AND CMAKE_MATCH_1)
            set(font_variant "${CMAKE_MATCH_1}")
        elseif(NOT ICONFONTS_VARIANT_PATTERN)
            message(FATAL_ERROR "Cannot resolve font variant name. "
                "Either pass the VARIANT_PATTERN option to ${CMAKE_CURRENT_FUNCTION}(), "
                "or add 'variant' entries the info file options in FONT_VARIANTS.")
        else()
            message(FATAL_ERROR "Invalid filepath '${font_filepath}': "
                "Cannot extract the font variant from basename '${font_basename}' "
                "using the regular expression '${ICONFONTS_VARIANT_PATTERN}', but also "
                "could not find a 'variant' entry in the info file options of FONT_VARIANTS.")
        endif()

        iconfonts_add_font(
            TARGET                  "${ICONFONTS_TARGET}"
                                    "${ICONFONTS_OPTIONAL}"
            SKIP_RESOURCES                                          # must collect resources for all variants here

            BASE_URL                "${ICONFONTS_BASE_URL}"
            RESOURCE_PREFIX         "${ICONFONTS_RESOURCE_PREFIX}"

            ARCHIVE                 "${ICONFONTS_ARCHIVE}"
            ARCHIVE_FILEHASH        "${ICONFONTS_ARCHIVE_FILEHASH}"

            FONT_FAMILY             "${ICONFONTS_FONT_FAMILY}"
            FONT_VARIANT            "${font_variant}"
            FONT_FILEPATH           "${font_filepath}"
            FONT_FILEHASH           "${font_filehash}"

            INFO_FILEPATH           "${info_filepath}"
            INFO_FILEHASH           "${info_filehash}"
            INFO_OPTIONS            "${info_options}"
            INFO_FILETYPE           "${ICONFONTS_INFO_FILETYPE}"

            LICENSE_FILEPATH        "${ICONFONTS_LICENSE_FILEPATH}"
            LICENSE_FILEHASH        "${ICONFONTS_LICENSE_FILEHASH}"

            OUTPUT_RESOURCES_LIST    resources_list)

        list(APPEND combined_resources_list ${resources_list})
    endforeach()

    if (ICONFONTS_RESOURCE_PREFIX) # --------------------------------------------------------- generate Qt resource file
        list(REMOVE_DUPLICATES combined_resources_list)

        get_target_property(resource_dirpath "${ICONFONTS_TARGET}" ICONFONTS_RESOURCE_DIR)
        __iconfonts_make_symbol("${ICONFONTS_FONT_FAMILY}" font_family_symbol)
        string(APPEND resource_dirpath "/${font_family_symbol}")

        qt_add_resources(
            "${ICONFONTS_TARGET}" "${font_family_symbol}"
            PREFIX  "${ICONFONTS_RESOURCE_PREFIX}"
            BASE    "${resource_dirpath}"
            FILES    ${combined_resources_list}
            BIG_RESOURCES)
    endif()
endfunction(iconfonts_add_font_family)

# FIXME doxs
function(iconfonts_add_system_font_family)
    set(options
        OPTIONAL)               # the option for enabling this font is disabled by default for OPTIONAL fonts

    set(mandatory_values
        TARGET                  # the target to which to add this font variant
        BASE_URL                # common base URL for all the paths passed
        FONT_FAMILY             # the font family this font belongs to
        INFO_FILEPATH           # path of the icon description; releative to `BASE_URL`
        INFO_FILEHASH           # SHA1 hash for `INFO_FILEPATH`
        LICENSE_FILEPATH        # path of the font's license file; releative to `BASE_URL`
        LICENSE_FILEHASH)       # SHA1 hash for `LICENSE_FILEPATH`

    set(optional_values
        INFO_FILETYPE           # file format of `INFO_FILEPATH`
        INFO_OPTIONS            # additional parser options for `INFO_FILEPATH`
        RESOURCE_PREFIX)        # prefix of the Qt resources; if not specified no resources are generated

    set(list_values
        FONT_VARIANTS)          # the font variants within the font's family

    set(single_values ${mandatory_values} ${optional_values}) # ------------------------------- parse function arguments
    cmake_parse_arguments(ICONFONTS "${options}" "${single_values}" "${list_values}" ${ARGN})

    iconfonts_reject_unparsed_arguments(ICONFONTS)
    iconfonts_require_mandatory_arguments(ICONFONTS ${mandatory_values})

    if (ICONFONTS_OPTIONAL)
        set(ICONFONTS_OPTIONAL "OPTIONAL")
    else()
        unset(ICONFONTS_OPTIONAL)
    endif()

    foreach(font_variant IN LISTS ICONFONTS_FONT_VARIANTS) # ------------------------------------- iterate font variants
        iconfonts_add_font(
            TARGET                  "${ICONFONTS_TARGET}"
                                    "${ICONFONTS_OPTIONAL}"
            SKIP_RESOURCES                                          # must collect resources for all variants here

            BASE_URL                "${ICONFONTS_BASE_URL}"
            RESOURCE_PREFIX         "${ICONFONTS_RESOURCE_PREFIX}"

            FONT_FAMILY             "${ICONFONTS_FONT_FAMILY}"
            FONT_VARIANT            "${font_variant}"
            FONT_FILEPATH           ""                                  # none: this is a system font
            FONT_FILEHASH           ""

            INFO_FILEPATH           "${ICONFONTS_INFO_FILEPATH}"
            INFO_FILEHASH           "${ICONFONTS_INFO_FILEHASH}"
            INFO_OPTIONS            "${ICONFONTS_INFO_OPTIONS}"
            INFO_FILETYPE           "${ICONFONTS_INFO_FILETYPE}"

            LICENSE_FILEPATH        "${ICONFONTS_LICENSE_FILEPATH}"
            LICENSE_FILEHASH        "${ICONFONTS_LICENSE_FILEHASH}"

            OUTPUT_RESOURCES_LIST    resources_list)
    endforeach()

    if (ICONFONTS_RESOURCE_PREFIX) # --------------------------------------------------------- generate Qt resource file
        get_target_property(resource_dirpath "${ICONFONTS_TARGET}" ICONFONTS_RESOURCE_DIR)
        __iconfonts_make_symbol("${ICONFONTS_FONT_FAMILY}" font_family_symbol)
        string(APPEND resource_dirpath "/${font_family_symbol}")

        qt_add_resources(
            "${ICONFONTS_TARGET}" "${font_family_symbol}"
            PREFIX  "${ICONFONTS_RESOURCE_PREFIX}"
            BASE    "${resource_dirpath}"
            FILES    ${resources_list}
            BIG_RESOURCES)
    endif()
endfunction(iconfonts_add_system_font_family)
