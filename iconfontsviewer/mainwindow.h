#ifndef ICONFONTSVIEWER_MAINWINDOW_H
#define ICONFONTSVIEWER_MAINWINDOW_H

#include "fontsizespinbox.h"

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
    explicit MainWindow(QWidget *parent = nullptr);

    void setIcon(const FontIcon &icon);
    [[nodiscard]] FontIcon icon() const;

private:
    QLayout *createFontListLayout();
    QLayout *createSymbolListLayout();
    QLayout *createPreviewLayout();

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
};

} // namespace IconFonts::Viewer

#endif // ICONFONTSVIEWER_MAINWINDOW_H
