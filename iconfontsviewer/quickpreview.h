#ifndef ICONFONTSVIEWER_QUICKPREVIEW_H
#define ICONFONTSVIEWER_QUICKPREVIEW_H

#ifdef ICONFONTS_ENABLE_QUICKWIDGET
#include <QQuickWidget>
#else
#include <QLabel>
#endif

namespace IconFonts {
class DrawIconOptions;
class FontIcon;
}

namespace IconFonts::Viewer {

#ifndef ICONFONTS_ENABLE_QUICKWIDGET

namespace Private {

class FakeQuickWidget : public QLabel
{
    Q_OBJECT

public:
    explicit FakeQuickWidget(QWidget *parent = nullptr);
    [[nodiscard]] QObject *rootObject() const { return nullptr; }
};

} // namespace Private

using QQuickWidget = Private::FakeQuickWidget;

#endif // ICONFONTS_ENABLE_QUICKWIDGET

class QuickPreview : public QQuickWidget
{
    Q_OBJECT

public:
    explicit QuickPreview(QWidget *parent = nullptr);

    void setIcon(const FontIcon &newIcon);
    [[nodiscard]] FontIcon icon() const;

    void setOptions(const DrawIconOptions &newOptions);
    [[nodiscard]] DrawIconOptions options() const;

protected:
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *target, QEvent *event) override;

    void setQuickProperty(const char *name, const QVariant &newValue);
    [[nodiscard]] QVariant quickProperty(const char *name) const;

private:
    void filterTopLevelWidget(QWidget *newTopLevelWidget);

    bool m_hasFontIcon = false;
    QPointer<QWidget> m_topLevelWidget;
};

} // namespace IconFonts::Viewer

#endif // ICONFONTSVIEWER_QUICKPREVIEW_H
