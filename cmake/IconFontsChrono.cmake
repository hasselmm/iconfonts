# ----------------------------------------------------------------------------------------------------------------------
# Captures a milliseconds timestamp and stores in `OUTPUT_VARIABLE`.
# ----------------------------------------------------------------------------------------------------------------------
function(iconfonts_timestamp OUTPUT_VARIABLE)
    string(TIMESTAMP microseconds "%s%f" UTC)
    math(EXPR milliseconds "${microseconds} / 1000")
    set("${OUTPUT_VARIABLE}" ${milliseconds} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Stores the milliseconds duration until `START_TIME` in `OUTPUT_VARIABLE`.
# ----------------------------------------------------------------------------------------------------------------------
function(iconfonts_duration START_TIME OUTPUT_VARIABLE)
    iconfonts_timestamp(end_time)
    math(EXPR duration "${end_time} - ${${START_TIME}}")
    set("${OUTPUT_VARIABLE}" ${duration} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Stores the duration until `START_TIME` in `OUTPUT_VARIABLE` as pretty text.
# The duration is scaled appropriately for display, and the chosen unit is added.
# ----------------------------------------------------------------------------------------------------------------------
function(iconfonts_duration_with_unit START_TIME OUTPUT_VARIABLE)
    iconfonts_duration("${START_TIME}" duration)

    if (duration LESS 1000)
        math(EXPR seconds "${duration} / 1000")
        math(EXPR milliseconds "${duration} % 1000")
        set("${OUTPUT_VARIABLE}" "${duration} ms" PARENT_SCOPE)
    elseif (duration LESS 60000)
        math(EXPR seconds "${duration} / 1000")
        math(EXPR milliseconds "${duration} % 1000")
        string(REGEX MATCH "[0-9][0-9][0-9]\$" milliseconds "000${milliseconds}")
        string(REGEX REPLACE "0+\$" "" milliseconds "${milliseconds}")
        set("${OUTPUT_VARIABLE}" "${seconds}.${milliseconds} sec" PARENT_SCOPE)
    else()
        math(EXPR minutes "${duration} / 60000")
        math(EXPR seconds "(${duration} / 1000) % 60")
        string(REGEX MATCH "[0-9][0-9]\$" seconds "00${seconds}")
        set("${OUTPUT_VARIABLE}" "${minutes}:${seconds} min" PARENT_SCOPE)
    endif()
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# Some minimal unit testing
# ----------------------------------------------------------------------------------------------------------------------

if (ICONFONTS_ENABLE_TESTING)
    message(VERBOSE "Testing chrono functions...")
    list(APPEND CMAKE_MESSAGE_INDENT "  ")

    iconfonts_timestamp(test_timestamp)

    math(EXPR test_timestamp_msec "${test_timestamp} - 500")
    math(EXPR test_timestamp_sec  "${test_timestamp} - 1500")
    math(EXPR test_timestamp_min  "${test_timestamp} - 150000")

    iconfonts_duration(test_timestamp_msec test_duration_msec)
    iconfonts_duration(test_timestamp_sec  test_duration_sec)
    iconfonts_duration(test_timestamp_min  test_duration_min)

    iconfonts_assert(test_duration_msec GREATER_EQUAL 500)
    iconfonts_assert(test_duration_msec LESS_EQUAL 505)

    iconfonts_assert(test_duration_sec GREATER_EQUAL 1500)
    iconfonts_assert(test_duration_sec LESS_EQUAL 1505)

    iconfonts_assert(test_duration_min GREATER_EQUAL 150000)
    iconfonts_assert(test_duration_min LESS_EQUAL 150005)

    iconfonts_duration_with_unit(test_timestamp_msec test_duration_msec)
    iconfonts_duration_with_unit(test_timestamp_sec  test_duration_sec)
    iconfonts_duration_with_unit(test_timestamp_min  test_duration_min)

    iconfonts_assert(test_duration_msec MATCHES "^50[0-5] ms\$")
    iconfonts_assert(test_duration_sec  MATCHES "^1\\.5(0[0-5])? sec\$")
    iconfonts_assert(test_duration_min  MATCHES "^2:30 min\$")

    list(POP_BACK CMAKE_MESSAGE_INDENT)
endif(ICONFONTS_ENABLE_TESTING)
