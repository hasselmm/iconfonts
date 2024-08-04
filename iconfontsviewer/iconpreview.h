#ifndef ICONFONTSVIEWER_ICONPREVIEW_H
#define ICONFONTSVIEWER_ICONPREVIEW_H

#include <iconfonts/iconfonts.h>

#include <QWidget>

namespace IconFonts::Viewer {

class IconPreview : public QWidget
{
    Q_OBJECT

public:
    using QWidget::QWidget;

    void setIcon(FontIcon newIcon);
    [[nodiscard]] FontIcon icon() const;

    void setOptions(const DrawIconOptions &newOptions);
    [[nodiscard]] DrawIconOptions options() const;

signals:
    void iconChanged();
    void optionsChanged();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    FontIcon m_icon = FontIcon{} | palette().color(foregroundRole());
    DrawIconOptions m_options;

    static IconPreview *s_instance;
};

} // namespace IconFonts::Viewer

#endif // ICONFONTSVIEWER_ICONPREVIEW_H
