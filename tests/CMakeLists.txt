qt_add_executable(tst_iconfonts tst_iconfonts.cpp)
target_link_libraries(tst_iconfonts PRIVATE IconFonts Qt6::Test)
add_test(NAME tst_iconfonts COMMAND $<TARGET_FILE:tst_iconfonts>)
