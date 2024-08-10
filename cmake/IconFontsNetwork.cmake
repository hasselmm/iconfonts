# ----------------------------------------------------------------------------------------------------------------------
# Compares `FILEPATH` with `EXPECTED_HASH` and returns the result.
# ----------------------------------------------------------------------------------------------------------------------
function(iconfonts_validate_file FILEPATH EXPECTED_HASH OUTPUT_VARIABLE)
    set(result "error:unknown")

    if (EXPECTED_HASH MATCHES "^([^=]+)=([0-9A-Fa-f]+)\$")
        set(hash_algorithm "${CMAKE_MATCH_1}")
        set(expected_hash  "${CMAKE_MATCH_2}")
    else()
        message(FATAL_ERROR "Unexpected value for EXPECTED_HASH: \"${EXPECTED_HASH}\"")
    endif()

    if (NOT EXISTS "${FILEPATH}")
        set(result "error:not-found")
    else()
        file("${hash_algorithm}" "${FILEPATH}" actual_hash)

        if (NOT actual_hash STREQUAL expected_hash)
            set(result "error:bad-hash:${actual_hash}")
        else()
            set(result "ok")
        endif()
    endif()

    set("${OUTPUT_VARIABLE}" "${result}" PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Downloads from `URL` to `FILEPATH`,
# compares the downloaded file with `EXPECTED_HASH`.
# ----------------------------------------------------------------------------------------------------------------------
function(iconfonts_download URL EXPECTED_HASH FILEPATH)
    iconfonts_validate_file("${FILEPATH}" "${EXPECTED_HASH}" validation_result)

    if (validation_result STREQUAL "ok")
        message(VERBOSE "No download needed for <${URL}>")
        return()
    endif()

    message(STATUS "Downloading <${URL}>")
    message(VERBOSE "to '${FILEPATH}'")

    file(
        DOWNLOAD "${URL}" "${FILEPATH}"
        EXPECTED_HASH "${EXPECTED_HASH}"
        STATUS download_status
        LOG download_log
    )

    if (NOT download_status MATCHES "0;")
        list(POP_FRONT download_status)
        message(FATAL_ERROR "Could not download ${URL}: ${download_status}")
        message(STATUS "${download_log}")
    endif()
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Downloads an archive from `URL` to `FILENAME`,
# compares the downloaded file with `EXPECTED_HASH`,
# and then extracts the archive to `DESTINATION`.
# ----------------------------------------------------------------------------------------------------------------------
function(iconfonts_download_archive)
    set(single_values
        URL                 # the archive's URL
        EXPECTED_HASH       # the archive's checksum
        FILENAME            # local filename of the archive
        DESTINATION)        # where to extract the archive

    set(multiple_values
        PATTERNS)           # file pattern for the files to extract

    cmake_parse_arguments(DOWNLOAD "" "${single_values}" "${multiple_values}" ${ARGN})

    iconfonts_reject_unparsed_arguments(DOWNLOAD)
    iconfonts_download("${DOWNLOAD_URL}" "${DOWNLOAD_EXPECTED_HASH}" "${DOWNLOAD_FILENAME}")

    file(
        ARCHIVE_EXTRACT
        INPUT "${DOWNLOAD_FILENAME}"
        DESTINATION "${DOWNLOAD_DESTINATION}"
        PATTERNS ${DOWNLOAD_PATTERNS}
    )
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Retreives information about the NPM package `PACKAGE_NAME` and stores the JSON query results
# in variables starting with `PREFIX`.
# ----------------------------------------------------------------------------------------------------------------------
function(iconfonts_query_npm_package PREFIX PACKAGE_NAME VERSION QUERIES)
    iconfonts_assert(PREFIX)
    iconfonts_assert(PACKAGE_NAME)
    iconfonts_assert(VERSION)

    iconfonts_encode_url("${PACKAGE_NAME}" encoded_name)

    set(npminfo_url "https://registry.npmjs.org/${encoded_name}/${VERSION}")
    set(npminfo_filepath "${resource_dirpath}/${PACKAGE_NAME}/${VERSION}.json")
    string(REPLACE "@" "" npminfo_filepath "${npminfo_filepath}")

    if (NOT EXISTS "${npminfo_filepath}")
        file(DOWNLOAD "${npminfo_url}" "${npminfo_filepath}" STATUS npminfo_status)
        list(GET npminfo_status 0 npminfo_statuscode)
        iconfonts_assert(npminfo_statuscode EQUAL 0)
    endif()

    file(READ "${npminfo_filepath}" npminfo_json)
    unset(npminfo)

    foreach(query IN LISTS QUERIES ARGN)
        string(REPLACE "/" ";" query_tokens "${query}")
        string(REPLACE "/" "_" query_name "${query}")
        string(JSON result GET "${npminfo_json}" ${query_tokens})
        set("${PREFIX}_${query_name}" ${result} PARENT_SCOPE)
    endforeach()

    #set("${OUTPUT_VARIABLE}" ${npminfo} PARENT_SCOPE)
endfunction()

