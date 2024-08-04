# ----------------------------------------------------------------------------------------------------------------------
# Condenses `VARIABLE` by recursively applying `REGEX`
# ----------------------------------------------------------------------------------------------------------------------
function(iconfonts_condense VARIABLE REGEX)
    set(old_value "${${VARIABLE}}")

    while(TRUE)
        string(REGEX REPLACE "${REGEX}" "" new_value "${old_value}")

        if ("${new_value}" STREQUAL "${old_value}")
            break()
        endif()

        set(old_value "${new_value}")
    endwhile()

    set("${VARIABLE}" "${new_value}" PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Replaces `<HEX>` tokens in `VARIABLE` by their proper ASCII character.
# ----------------------------------------------------------------------------------------------------------------------
function(iconfonts_unescape VARIABLE)
    set(string "${${VARIABLE}}")

    while(string MATCHES "<([0-9A-Fa-f]+)>")
        math(EXPR ascii "0x${CMAKE_MATCH_1}" OUTPUT_FORMAT DECIMAL)
        string(ASCII "${ascii}" char)
        string(REPLACE "${CMAKE_MATCH_0}" "${char}" string "${string}")
    endwhile()

    set("${VARIABLE}" "${string}" PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Applies some minimal regular expression encoding to `INPUT`.
# ----------------------------------------------------------------------------------------------------------------------

function(iconfonts_encode_regex INPUT OUTPUT_VARIABLE)
    string(REPLACE "." "\\." escaped "${INPUT}")
    set("${OUTPUT_VARIABLE}" "${escaped}" PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Applies some minimal URL encoding to `INPUT`.
# ----------------------------------------------------------------------------------------------------------------------
function(iconfonts_encode_url INPUT OUTPUT_VARIABLE)
    set(multiple_values SKIP)

    cmake_parse_arguments(URLENCODE "" "" "${multiple_values}" ${ARGN})
    iconfonts_reject_unparsed_arguments(URLENCODE)

    set(encoded "${INPUT}")

    # -------------------> % SP  :  /  ?  #  [  ]  @  !  $  &  '  (  )  *  +  ,  ;  =
    foreach(code IN ITEMS 37 32 58 47 63 35 91 93 64 33 36 38 39 40 41 42 43 44 59 61)
        math(EXPR hex "${code}" OUTPUT_FORMAT HEXADECIMAL)
        string(SUBSTRING "${hex}" 2 3 hex)
        string(ASCII "${code}" char)

        if (char IN_LIST URLENCODE_SKIP)
            continue()
        endif()

        string(REPLACE "${char}" "%${hex}" encoded "${encoded}")
    endforeach()

    set("${OUTPUT_VARIABLE}" "${encoded}" PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Converts a string containing an `JSON` array into a list.
# ----------------------------------------------------------------------------------------------------------------------
function(iconfonts_json_to_list OUTPUT_VARIABLE JSON)
    string(JSON count LENGTH "${JSON}")
    math(EXPR last "${count} - 1")
    set(list)

    foreach(index RANGE ${last})
        string(JSON value GET "${JSON}" ${index})
        list(APPEND list "${value}")
    endforeach()

    set("${OUTPUT_VARIABLE}" ${list} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Turns `NAME` into a C++ camelcase identifier, and returns the result in `OUTPUT_VARIABLE`.
# ----------------------------------------------------------------------------------------------------------------------
function(__iconfonts_make_symbol NAME OUTPUT_VARIABLE)
    set(alphanum  "A-Za-z0-9")
    set(separator "-_ \t.")

    # Inject separator to ensure we will start with an uppercase character.
    # Also turn the entire name lowercase, so that we just have to care about producing leading uppercase character.
    string(TOLOWER "-${NAME}" symbol)

    # Split by separators and then turn the first character of each section uppercase.

    while (symbol MATCHES "([^${separator}]*)[${separator}]([${alphanum}])(.*)")
        string(TOUPPER "${CMAKE_MATCH_2}" start)
        set(symbol "${CMAKE_MATCH_1}${start}${CMAKE_MATCH_3}")
    endwhile()

    # Remove non alphanumeric characters.
    string(REGEX REPLACE "[^${alphanum}]+" "" symbol "${symbol}")

    # Ensure the symbol doesn't start with a number.
    if (symbol MATCHES "^[0-9]")
        set(symbol "_${symbol}")
    endif()

    set("${OUTPUT_VARIABLE}" "${symbol}" PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Some minimal unit testing
# ----------------------------------------------------------------------------------------------------------------------

if (ICONFONTS_ENABLE_TESTING)
    message(VERBOSE "Testing string functions...")
    list(APPEND CMAKE_MESSAGE_INDENT "  ")

    set(test_string "ABC<D<E<F>>>GHI") # -------------------------------------------------------------------------------
    iconfonts_condense(test_string "<[DEF]>")
    iconfonts_assert(test_string STREQUAL "ABCGHI")

    set(test_string "<41>BC<44>EF<47>")
    iconfonts_unescape(test_string) # ----------------------------------------------------------------------------------
    iconfonts_assert(test_string STREQUAL "ABCDEFG")

    iconfonts_encode_regex(".bxs-" test_string)
    iconfonts_assert(test_string STREQUAL "\\.bxs-")

    iconfonts_encode_url("Hello World!" test_string) # -----------------------------------------------------------------
    iconfonts_assert(test_string STREQUAL "Hello%20World%21")

    iconfonts_encode_url("http://wikipedia.org/#top" test_string)
    iconfonts_assert(test_string STREQUAL "http%3a%2f%2fwikipedia.org%2f%23top")

    iconfonts_encode_url("http://wikipedia.org/#top" test_string SKIP / :)
    iconfonts_assert(test_string STREQUAL "http://wikipedia.org/%23top")

    iconfonts_json_to_list(test_list "[1, 2, 3]") # --------------------------------------------------------------------
    iconfonts_assert("[${test_list}]" STREQUAL "[1;2;3]")

    iconfonts_json_to_list(test_list "[\"a\", \"b\", \"c\"]")
    iconfonts_assert("[${test_list}]" STREQUAL "[a;b;c]")

    __iconfonts_make_symbol("Hello world" test_string) # ---------------------------------------------------------------
    iconfonts_assert(test_string STREQUAL "HelloWorld")

    __iconfonts_make_symbol("Hello world!" test_string)
    iconfonts_assert(test_string STREQUAL "HelloWorld")

    __iconfonts_make_symbol(".hello-world-" test_string)
    iconfonts_assert(test_string STREQUAL "HelloWorld")

    __iconfonts_make_symbol(".-hello-world-." test_string)
    iconfonts_assert(test_string STREQUAL "HelloWorld")

    __iconfonts_make_symbol("_hello_world_" test_string)
    iconfonts_assert(test_string STREQUAL "HelloWorld")

    __iconfonts_make_symbol("hello\tworld" test_string)
    iconfonts_assert(test_string STREQUAL "HelloWorld")

    __iconfonts_make_symbol("1 World!" test_string)
    iconfonts_assert(test_string STREQUAL "_1World")

    list(POP_BACK CMAKE_MESSAGE_INDENT)
endif(ICONFONTS_ENABLE_TESTING)
