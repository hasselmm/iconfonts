qt_add_executable(
    IconFontsViewer WIN32
    main.cpp

    fontlistwidget.h fontlistwidget.cpp
    fontsizespinbox.h fontsizespinbox.cpp
    iconpreview.h iconpreview.cpp
    mainwindow.h mainwindow.cpp
    quickpreview.h quickpreview.cpp
    symbollistwidget.h symbollistwidget.cpp
)

qt_add_resources(
    IconFontsViewer iconfontsviewer
    PREFIX "/IconFonts/Viewer"
    FILES qml/QuickPreview.qml
)

target_link_libraries(
    IconFontsViewer PRIVATE
    IconFonts
    Qt::Widgets
)

if (TARGET Qt::QuickWidgets)
    target_compile_definitions(
        IconFontsViewer PRIVATE
        ICONFONTS_ENABLE_QUICKWIDGET
    )

    target_link_libraries(
        IconFontsViewer PRIVATE
        Qt::QuickWidgets
        QuickIconFonts
    )
endif()
