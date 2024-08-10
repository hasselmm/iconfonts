function(iconfonts_add_test NAME)
    cmake_parse_arguments(TEST "" "" "LIBRARIES;SOURCES" ${ARGN})

    qt_add_executable("${NAME}" ${TEST_SOURCES} ${TEST_UNPARSED_ARGUMENTS})
    target_link_libraries("${NAME}" PRIVATE Qt::Test ${TEST_LIBRARIES})
    add_test(NAME "${NAME}" COMMAND $<TARGET_FILE:${NAME}>)
endfunction()

iconfonts_add_test(
    tst_iconfonts tst_iconfonts.cpp

    LIBRARIES
        IconFontsCollection
        IconFonts_SegoeFluentIcons
        IconFonts_SegoeMdl2Assets
)

iconfonts_add_test(
    tst_glyphsrenderable tst_glyphsrenderable.cpp
    LIBRARIES IconFonts
)

if (TARGET QuickIconFonts)
    iconfonts_add_test(
        tst_quickiconfonts tst_quickiconfonts.cpp
        LIBRARIES QuickIconFonts
    )
endif()

if (TARGET Qt::QuickTest)
    file(GLOB quicktest_sources CONFIGURE_DEPENDS tst_*.qml)

    iconfonts_add_test(
        tst_quicktests tst_quicktests.cpp ${quicktest_sources}
        LIBRARIES Qt::QuickTest)

    target_compile_definitions(
        tst_quicktests PRIVATE
        QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()
