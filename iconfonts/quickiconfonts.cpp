#include "quickiconfonts.h"

#include <QQuickWindow>

#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickpalette_p.h>

namespace QuickIconFonts {
namespace {

using ColorResolver = IconFonts::DrawIconOptions::ColorResolver;
using IconMode = IconFonts::DrawIconOptions::IconMode;

QColor color(const QQuickColorGroup *group, QPalette::ColorRole role)
{
    switch (role) {
    case QPalette::WindowText:
        return group->windowText();
    case QPalette::Button:
        return group->button();
    case QPalette::Light:
        return group->light();
    case QPalette::Midlight:
        return group->midlight();
    case QPalette::Dark:
        return group->dark();
    case QPalette::Mid:
        return group->mid();
    case QPalette::Text:
        return group->text();
    case QPalette::BrightText:
        return group->brightText();
    case QPalette::ButtonText:
        return group->buttonText();
    case QPalette::Base:
        return group->base();
    case QPalette::Window:
        return group->window();
    case QPalette::Shadow:
        return group->shadow();
    case QPalette::Highlight:
        return group->highlight();
    case QPalette::HighlightedText:
        return group->highlightedText();
    case QPalette::Link:
        return group->link();
    case QPalette::LinkVisited:
        return group->linkVisited();
    case QPalette::AlternateBase:
        return group->alternateBase();
    case QPalette::ToolTipBase:
        return group->toolTipBase();
    case QPalette::ToolTipText:
        return group->toolTipText();
    case QPalette::PlaceholderText:
        return group->placeholderText();
    case QPalette::NoRole:
        return QColor{};

    case QPalette::NColorRoles:
        break;
    }

    Q_UNREACHABLE_RETURN(QColor{});
}

ColorResolver makeColorResolver(const QQuickItem *item)
{
    if (const auto d = QQuickItemPrivate::get(item)) {
        if (const auto palette = d->palette()) {
            return [palette](QIcon::Mode mode, QPalette::ColorRole role)
            {
                switch (mode) {
                case QIcon::Mode::Disabled:
                    return color(palette->disabled(), role);
                case QIcon::Mode::Normal:
                    return color(palette->inactive(), role);
                case QIcon::Mode::Active:
                    return color(palette->active(), role);
                case QIcon::Mode::Selected:
                    return color(palette->active(), QPalette::HighlightedText);
                }

                Q_UNREACHABLE_RETURN(QColor{});
            };
        }
    }

    return ColorResolver{};
};

[[nodiscard]] IconMode iconMode(const QQuickItem *item, bool active)
{
    if (!item->isEnabled())
        return IconMode::Disabled;
    else if (!active)
        return IconMode::Normal;
    else
        return IconMode::Active;
}

[[nodiscard]] bool isActive(const QQuickItem *item)
{
    if (const auto window = item ? item->window() : nullptr)
        return window->isActive();

    return true;
}

} // namespace

QColor DrawIconOptions::effectiveColor(const QQuickItem *item, const QColor &iconColor) const
{
    return effectiveColor(item, iconColor, isActive(item));
}

QColor DrawIconOptions::effectiveColor(const QQuickItem *item, const QColor &iconColor, bool active) const
{
    return m_options.effectiveColor(iconColor, makeColorResolver(item), iconMode(item, active));
}

} // namespace QuickIconFonts
