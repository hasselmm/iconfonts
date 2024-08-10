if (CMAKE_SCRIPT_MODE_FILE) # ---------------------------------------------- set some required variables for script mode
    cmake_minimum_required(VERSION 3.19)

    set(ICONFONTS_ENABLE_TESTING OFF)
    set(ICONFONTS_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")
    set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
endif()

include(IconFontsAssert) # --------------------------------------------------------------------- include library scripts
include(IconFontsChrono)
include(IconFontsCodeGenerator)
include(IconFontsNetwork)
include(IconFontsParser)
include(IconFontsPython)
include(IconFontsStrings)
include(IconFontsScriptMode)
include(IconFontsUtilities)

# FIXME doxs
function(iconfonts_add_font)
    set(options
        RECOMMENDED)            # the option for enabling this font is enabled by default for RECOMMENDED fonts

    set(mandatory_values
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
        INFO_OPTIONS)           # additional parser options for `INFO_FILEPATH`

    set(single_values ${mandatory_values} ${optional_values}) # ------------------------------- parse function arguments
    cmake_parse_arguments(ICONFONTS "${options}" "${single_values}" "" ${ARGN})

    iconfonts_reject_unparsed_arguments(ICONFONTS)
    iconfonts_require_mandatory_arguments(ICONFONTS ${mandatory_values})

    if (NOT ICONFONTS_INFO_TYPE)
        __iconfonts_guess_info_type(
            "${ICONFONTS_INFO_FILEPATH}" "${ICONFONTS_FONT_FILEPATH}"
            "${ICONFONTS_FONT_VARIANT}" ICONFONTS_INFO_FILETYPE)
    endif()

    __iconfonts_set_target_properties(IconFonts)
    __iconfonts_get_target_properties(IconFonts ICONFONTS)

    if (ICONFONTS_BASE_URL MATCHES "^https://github\.com(/.*)\$")  # ------------------------------------ parse BASE_URL
        set(github_details "${CMAKE_MATCH_1}")

        message(TRACE "github_details: '${github_details}'")

        if (github_details MATCHES "^/([^/@]+)(/.*)\$")
            set(github_account "${CMAKE_MATCH_1}")
            set(github_details "${CMAKE_MATCH_2}")
        else()
            message(FATAL_ERROR "Github URL detected, but the account name is missing: ${ICONFONTS_BASE_URL}")
        endif()

        iconfonts_show(TRACE github_account)

        if (github_details MATCHES "^/([^/@]+)([@/].*)\$")
            set(github_repository "${CMAKE_MATCH_1}")
            set(github_details "${CMAKE_MATCH_2}")
        else()
            message(FATAL_ERROR "Github URL detected, but the repository name is missing: ${ICONFONTS_BASE_URL}")
        endif()

        iconfonts_show(TRACE github_repository)

        if (github_details MATCHES "^@(.+)\$")
            set(github_revision "${CMAKE_MATCH_1}")
        else()
            message(FATAL_ERROR "Github URL detected, but the revision is missing: ${ICONFONTS_BASE_URL}")
        endif()

        iconfonts_show(TRACE github_revision)

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

    set(resource_dirpath "${ICONFONTS_RESOURCE_DIR}") # ------------------------------- build file paths and target name

    __iconfonts_make_symbol("${ICONFONTS_FONT_FAMILY}" font_family_symbol)
    string(APPEND resource_dirpath "/${font_family_symbol}")
    set(info_dirpath "${resource_dirpath}")

    set(license_target "IconFonts_${font_family_symbol}_license")
    set(common_font_target "IconFonts_${font_family_symbol}")
    set(font_namespace "${font_family_symbol}")

    if (ICONFONTS_FONT_VARIANT)
        __iconfonts_make_symbol("${ICONFONTS_FONT_VARIANT}" font_variant_symbol)
        string(APPEND common_font_target "${font_variant_symbol}")
        string(APPEND font_namespace "::${font_variant_symbol}")
        string(APPEND info_dirpath "/${font_variant_symbol}")
    endif()

    set(quick_font_target "Quick${common_font_target}")

    qt_add_library("${common_font_target}" STATIC EXCLUDE_FROM_ALL) # --------------------------- define library targets
    target_compile_definitions("${common_font_target}" PUBLIC ICONFONTSFONTS_EXPORT=) # FIXME use font specific name
    target_link_libraries("${common_font_target}" PUBLIC IconFonts)

    target_link_options(
        "${common_font_target}" PUBLIC
        "SHELL: -Wl,-whole-archive $<TARGET_FILE:${common_font_target}> -Wl,-no-whole-archive")

    qt_add_library("${quick_font_target}" STATIC EXCLUDE_FROM_ALL)
    target_link_libraries("${quick_font_target}" PRIVATE QuickIconFonts "${common_font_target}")

    if (ICONFONTS_RECOMMENDED) # -------------------------------- check whether to add this target to IconFonts_AllFonts
        set(font_enabled_default ON)
    else()
        set(font_enabled_default OFF)
    endif()

    __iconfonts_option(
        TARGET           IconFonts
        OUTPUT_VARIABLE  font_enabled
        FONT_FAMILY     "${ICONFONTS_FONT_FAMILY}"
        FONT_VARIANT    "${ICONFONTS_FONT_VARIANT}"
        DEFAULT_VALUE   "${font_enabled_default}")

    if (font_enabled)
        target_link_libraries(IconFontsCollection       PRIVATE "${common_font_target}")
        target_link_libraries(QuickIconFontsCollection  PRIVATE "${quick_font_target}")

        set(deferred_call_id "iconfonts_finalize_target(IconFonts)")
        set(deferred_call "iconfonts_finalize_target IconFonts")

        cmake_language(DEFER CANCEL_CALL "${deferred_call_id}")
        cmake_language(EVAL CODE "cmake_language(DEFER ID \"${deferred_call_id}\" CALL ${deferred_call})")
    endif()

    list(APPEND CMAKE_MESSAGE_INDENT "  ")

    if (ICONFONTS_ARCHIVE) # ----------------------------------------------------------------- maybe download as archive
        if (ICONFONTS_ARCHIVE MATCHES "npm:(.*)/([^/]+)\$")
            iconfonts_query_npm_package(npminfo "${CMAKE_MATCH_1}" "${CMAKE_MATCH_2}" "dist/tarball" "dist/shasum")
            iconfonts_assert(ICONFONTS_ARCHIVE_FILEHASH STREQUAL "SHA1=${npminfo_dist_shasum}")
            set(archive_url "${npminfo_dist_tarball}")
        else()
            make_url("${ICONFONTS_ARCHIVE}" archive_url)
        endif()

        cmake_path(GET archive_url FILENAME archive_filename)

        iconfonts_add_download_command(
            "${archive_url}" "${ICONFONTS_ARCHIVE_FILEHASH}"
            "${resource_dirpath}/${archive_filename}"
        )

        iconfonts_add_extract_command(
            "${resource_dirpath}/${archive_filename}" "${ICONFONTS_ARCHIVE_FILEHASH}"
            "${resource_dirpath}" "${ICONFONTS_FONT_FILEPATH}" "${ICONFONTS_FONT_FILEHASH}")

        iconfonts_add_extract_command(
            "${resource_dirpath}/${archive_filename}" "${ICONFONTS_ARCHIVE_FILEHASH}"
            "${resource_dirpath}" "${ICONFONTS_INFO_FILEPATH}" "${ICONFONTS_INFO_FILEHASH}")
    endif()

    cmake_path(GET ICONFONTS_INFO_FILEPATH FILENAME info_filename) # ------------------------- download icon information

    if (ICONFONTS_ARCHIVE)
        set(info_filepath "${resource_dirpath}/${ICONFONTS_INFO_FILEPATH}")
    else()
        make_url("${ICONFONTS_INFO_FILEPATH}" info_url)
        set(info_filepath "${info_dirpath}/${info_filename}")
        iconfonts_add_download_command("${info_url}" "${ICONFONTS_INFO_FILEHASH}" "${info_filepath}")
    endif()

    if (ICONFONTS_FONT_FILEPATH) # ------------------------------------------------------------------ download font file
        cmake_path(GET ICONFONTS_FONT_FILEPATH FILENAME font_filename)

        if (ICONFONTS_ARCHIVE)
            set(font_filepath "${resource_dirpath}/${ICONFONTS_FONT_FILEPATH}")
        else()
            make_url("${ICONFONTS_FONT_FILEPATH}" font_url)
            set(font_filepath "${resource_dirpath}/${font_filename}")
            iconfonts_add_download_command("${font_url}" "${ICONFONTS_FONT_FILEHASH}" "${font_filepath}")
        endif()

        __iconfonts_convert_webfont("${font_filepath}" "${resource_dirpath}" opentype_filepath)
        cmake_path(GET opentype_filepath FILENAME opentype_filename)

        set_property(
            SOURCE "${opentype_filepath}"
            PROPERTY QT_RESOURCE_ALIAS "${opentype_filename}")

        qt_add_resources(
            "${common_font_target}"
            "${common_font_target}_font"
            PREFIX  "${ICONFONTS_RESOURCE_PREFIX}"
            BASE    "${resource_dirpath}"
            FILES    ${opentype_filepath}
            BIG_RESOURCES)
    endif()

    if (ICONFONTS_RESOURCE_PREFIX) # ------------------------------------------------------------- download license file
        if (URL MATCHES "\\.md\$")
            set(license_filepath "${resource_dirpath}/LICENSE.md")
        else()
            set(license_filepath "${resource_dirpath}/LICENSE.txt")
        endif()

        make_url("${ICONFONTS_LICENSE_FILEPATH}" license_url)
        iconfonts_add_download_command("${license_url}" "${ICONFONTS_LICENSE_FILEHASH}" "${license_filepath}")
    endif()

    if (NOT TARGET "${license_target}") # ---------------------------------------- add license file to Qt resources
        qt_add_library("${license_target}" STATIC EXCLUDE_FROM_ALL)

        qt_add_resources(
            "${license_target}"
            "${license_target}"
            PREFIX  "${ICONFONTS_RESOURCE_PREFIX}"
            BASE    "${resource_dirpath}"
            FILES    ${license_filepath}
            BIG_RESOURCES)
    endif()

    target_link_libraries("${common_font_target}" INTERFACE "${license_target}")

    iconfonts_add_generate_code_command( # -------------------------------------------------------- generate source code
        COMMON_TARGET       "${common_font_target}"
        QUICK_TARGET        "${quick_font_target}"
        RESOURCE_PREFIX     "${ICONFONTS_RESOURCE_PREFIX}"
        FONT_TAG            "${ICONFONTS_NEXT_FONT_TAG}"
        FONT_FAMILY         "${ICONFONTS_FONT_FAMILY}"
        FONT_VARIANT        "${ICONFONTS_FONT_VARIANT}"
        FONT_FILEPATH       "${opentype_filepath}"
        FONT_NAMESPACE      "${font_namespace}"
        INFO_FILEPATH       "${info_filepath}"
        INFO_FILEHASH       "${ICONFONTS_INFO_FILEHASH}"
        INFO_OPTIONS        "${ICONFONTS_INFO_OPTIONS}"
        INFO_FILETYPE       "${ICONFONTS_INFO_FILETYPE}"
        LICENSE_FILEPATH    "${ICONFONTS_LICENSE_FILEPATH}"
    )

    math(EXPR next_font_tag "${ICONFONTS_NEXT_FONT_TAG} + 1") # ----------------------- generate and store next font tag

    set_property(
        TARGET IconFonts PROPERTY
        ICONFONTS_NEXT_FONT_TAG "${next_font_tag}")

    set_property(
        TARGET IconFonts APPEND PROPERTY
        ICONFONTS_FONT_NAMESPACES "${font_namespace}")

    list(POP_BACK CMAKE_MESSAGE_INDENT)
endfunction(iconfonts_add_font)

# ----------------------------------------------------------------------------------------------------------------------
# Registers a font family. This is basically a convenience wrapper for a font family,
# which groups common properties, differentiates the font familes via `FONT_FAMILY` entries.
# ----------------------------------------------------------------------------------------------------------------------
function(iconfonts_add_font_family)
    set(options
        RECOMMENDED)            # the option for enabling this font is enabled by default for RECOMMENDED fonts

    set(mandatory_values
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
        FONT_VARIANTS)          # description of the font variants by entries of colon-separated fields:
                                # FONT_FILEPATH, FONT_FILEHASH, INFO_FILEPATH, INFO_FILEHASH, INFO_OPTIONS
                                # the INFO_OPTIONS field is optional

    set(single_values ${mandatory_values} ${optional_values}) # ------------------------------- parse function arguments
    cmake_parse_arguments(ICONFONTS "${options}" "${single_values}" "${list_values}" ${ARGN})

    iconfonts_reject_unparsed_arguments(ICONFONTS)
    iconfonts_require_mandatory_arguments(ICONFONTS ${mandatory_values} FONT_VARIANTS)
    iconfonts_unescape(ICONFONTS_VARIANT_PATTERN)

    if (ICONFONTS_RECOMMENDED)
        set(ICONFONTS_RECOMMENDED "RECOMMENDED")
    else()
        unset(ICONFONTS_RECOMMENDED)
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
            BASE_URL                "${ICONFONTS_BASE_URL}"
            RESOURCE_PREFIX         "${ICONFONTS_RESOURCE_PREFIX}"
                                    "${ICONFONTS_RECOMMENDED}"

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
            LICENSE_FILEHASH        "${ICONFONTS_LICENSE_FILEHASH}")
    endforeach()
endfunction(iconfonts_add_font_family)

# ----------------------------------------------------------------------------------------------------------------------
# Registers a system font family. System fonts are pre-installed and there are not builded for license reasons.
# The most important options here are `FONT_FAMILY`, which selects the font family,
# and `FONT_VARIANTS`, which lists all the known font variants.
# ----------------------------------------------------------------------------------------------------------------------
function(iconfonts_add_system_font_family)
    set(options
        RECOMMENDED)            # the option for enabling this font is enabled by default for RECOMMENDED fonts

    set(mandatory_values
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

    if (ICONFONTS_RECOMMENDED)
        set(ICONFONTS_RECOMMENDED "RECOMMENDED")
    else()
        unset(ICONFONTS_RECOMMENDED)
    endif()

    foreach(font_variant IN LISTS ICONFONTS_FONT_VARIANTS) # ------------------------------------- iterate font variants
        iconfonts_add_font(
            BASE_URL                "${ICONFONTS_BASE_URL}"
            RESOURCE_PREFIX         "${ICONFONTS_RESOURCE_PREFIX}"
                                    "${ICONFONTS_RECOMMENDED}"

            FONT_FAMILY             "${ICONFONTS_FONT_FAMILY}"
            FONT_VARIANT            "${font_variant}"
            FONT_FILEPATH           ""                                  # none: this is a system font
            FONT_FILEHASH           ""

            INFO_FILEPATH           "${ICONFONTS_INFO_FILEPATH}"
            INFO_FILEHASH           "${ICONFONTS_INFO_FILEHASH}"
            INFO_OPTIONS            "${ICONFONTS_INFO_OPTIONS}"
            INFO_FILETYPE           "${ICONFONTS_INFO_FILETYPE}"

            LICENSE_FILEPATH        "${ICONFONTS_LICENSE_FILEPATH}"
            LICENSE_FILEHASH        "${ICONFONTS_LICENSE_FILEHASH}")
    endforeach()
endfunction(iconfonts_add_system_font_family)

if (CMAKE_SCRIPT_MODE_FILE) # -------------------------------------------------------------------- implement script mode
    iconfonts_run_script_command()
endif()
