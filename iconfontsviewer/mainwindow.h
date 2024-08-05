#ifndef ICONFONTSVIEWER_MAINWINDOW_H
#define ICONFONTSVIEWER_MAINWINDOW_H

#include "fontsizespinbox.h"

class QGridLayout;
class QLabel;
class QQuickWidget;

namespace IconFonts {
class FontIcon;
}

namespace IconFonts::Viewer {

class FontListWidget;
class IconPreview;
class SymbolListWidget;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    enum class PreviewType {
        Painter,
        Widget,
        Quick,
    };

    Q_ENUM(PreviewType)

    explicit MainWindow(QWidget *parent = nullptr);

    void setIcon(const FontIcon &icon);
    [[nodiscard]] FontIcon icon() const;

private:
    enum PreviewColumn { Left, Right };
    enum PreviewRow { ToolBar, Preview, Caption, Selector };

    QLayout *createFontListLayout();
    QLayout *createSymbolListLayout();
    QLayout *createPreviewLayout();

    void setupPreviewSelector(QGridLayout *layout, PreviewColumn column);

    void onSelectedFontChanged();
    void onTransformActionTriggered(QAction *action);
    void onIconModeActionTriggered(QAction *action);
    void onColorActionTriggered();

    void setFontSizeMode(FontSizeSpinBox::Mode newMode);
    void onSetFontSize(int newFontSize);
    void setIgnoreColor(bool newIgnoreColor);

    void updateTextualPreview();

    FontListWidget      *const m_fontList;
    SymbolListWidget    *const m_symbolList;
    FontSizeSpinBox     *const m_fontSize;
    QAction             *const m_colorAction;
    IconPreview         *const m_graphicalPreview;
    QLabel              *const m_textualPreview;
    QQuickWidget        *const m_quickPreview;

    QActionGroup        *const m_leftPreviewGroup;
    QActionGroup        *const m_rightPreviewGroup;
};

} // namespace IconFonts::Viewer

#endif // ICONFONTSVIEWER_MAINWINDOW_H
