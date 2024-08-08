#include "iconpreview.h"

#include <QPaintEvent>
#include <QPainter>

namespace IconFonts::Viewer {

void IconPreview::setIcon(FontIcon newIcon)
{
    if (std::exchange(m_icon, newIcon) != newIcon) {
        update();
        emit iconChanged();
    }
}

FontIcon IconPreview::icon() const
{
    return m_icon;
}

void IconPreview::setOptions(const DrawIconOptions &newOptions)
{
    if (std::exchange(m_options, newOptions) != newOptions) {
        update();
        emit optionsChanged();
    }
}

DrawIconOptions IconPreview::options() const
{
    return m_options;
}

void IconPreview::paintEvent(QPaintEvent *event)
{
    auto painter = QPainter{this};
    painter.setClipRegion(event->region());

    using enum QIcon::Mode;
    const auto mode = isEnabled() ? isActiveWindow() ? Active : Normal : Disabled;
    m_icon.draw(&painter, size(), palette(), m_options, mode);
}

} // namespace IconFonts::Viewer
