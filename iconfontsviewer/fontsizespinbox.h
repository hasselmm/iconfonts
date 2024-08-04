#ifndef ICONFONTSVIEWER_FONTSIZESPINBOX_H
#define ICONFONTSVIEWER_FONTSIZESPINBOX_H

#include <QSpinBox>

namespace IconFonts::Viewer {

class FontSizeSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    enum class Mode {
        Fill,
        Pixels,
        Points,
    };

    explicit FontSizeSpinBox(QWidget *parent = nullptr);

    void setMode(Mode newMode);
    Mode mode() const;

signals:
    void modeChanged(Mode newMode);

private:
    QAction *addLineEditAction(const QString &text, Mode mode);
    QIcon textIcon(const QString &text, const QColor &foreground, const QColor &background);

    Mode m_mode = Mode::Fill;
};

} // namespace IconFonts::Viewer

#endif // ICONFONTSVIEWER_FONTSIZESPINBOX_H
