#include "quickpreview.h"

#include "iconfonts.h"

#ifdef ICONFONTS_ENABLE_QUICKWIDGET
#include <QQmlComponent>
#include <QQuickItem>
#endif // ICONFONTS_ENABLE_QUICKWIDGET

using namespace Qt::StringLiterals;

namespace IconFonts::Viewer {

#ifndef ICONFONTS_ENABLE_QUICKWIDGET

Private::FakeQuickWidget::FakeQuickWidget(QWidget *parent)
    : QLabel{tr("QQuickWidget was missing\nwhen configuring this build."), parent}
{
    setAlignment(Qt::AlignCenter);
    setEnabled(false);
}

#endif // ICONFONTS_ENABLE_QUICKWIDGET

QuickPreview::QuickPreview(QWidget *parent)
    : QQuickWidget{parent}
{
#ifdef ICONFONTS_ENABLE_QUICKWIDGET

    setClearColor(Qt::transparent);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setAttribute(Qt::WA_TranslucentBackground);
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setSource(u"qrc:/IconFonts/Viewer/qml/QuickPreview.qml"_s);

    if (status() == Error) {
        auto component = QQmlComponent{engine()};
        component.setData("import QtQuick.Controls; Label {}", QUrl{});
        setContent(component.url(), &component, component.create(rootContext()));

        setQuickProperty("horizontalAlignment", Qt::AlignHCenter);
        setQuickProperty("verticalAlignment", Qt::AlignVCenter);
        setQuickProperty("text", tr("Could not create font icon."));
    } else {
        m_hasFontIcon = true;
    }

#endif // ICONFONTS_ENABLE_QUICKWIDGET
}

void QuickPreview::setIcon(const FontIcon &newIcon)
{
    setQuickProperty("icon", QVariant::fromValue(newIcon));
}

FontIcon QuickPreview::icon() const
{
    return qvariant_cast<FontIcon>(quickProperty("icon"));
}

void QuickPreview::setOptions(const DrawIconOptions &newOptions)
{
    setQuickProperty("options", QVariant::fromValue(newOptions));
}

DrawIconOptions QuickPreview::options() const
{
    return qvariant_cast<DrawIconOptions>(quickProperty("options"));
}

void QuickPreview::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    if (event->type() == QEvent::EnabledChange)
        setQuickProperty("enabled", m_hasFontIcon && isEnabled());

    filterTopLevelWidget(topLevelWidget());
}

bool QuickPreview::eventFilter(QObject *target, QEvent *event)
{
    if (event->type() == QEvent::ActivationChange)
        setQuickProperty("active", isActiveWindow());

    return QQuickWidget::eventFilter(target, event);
}

void QuickPreview::setQuickProperty(const char *name, const QVariant &newValue)
{
    if (const auto rootItem = rootObject())
        rootItem->setProperty(name, newValue);
}

QVariant QuickPreview::quickProperty(const char *name) const
{
    if (const auto rootItem = rootObject())
        return rootItem->property(name);

    return {};
}

void QuickPreview::filterTopLevelWidget(QWidget *newTopLevelWidget)
{
    if (const auto oldTopLevelWidget = std::exchange(m_topLevelWidget, newTopLevelWidget);
            oldTopLevelWidget != newTopLevelWidget) {
        if (oldTopLevelWidget)
            oldTopLevelWidget->removeEventFilter(this);
        if (newTopLevelWidget)
            newTopLevelWidget->installEventFilter(this);
    }
}

} // namespace IconFonts::Viewer
