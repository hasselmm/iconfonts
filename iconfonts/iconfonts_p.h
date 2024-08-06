#ifndef ICONFONTS_ICONFONTS_P_H
#define ICONFONTS_ICONFONTS_P_H

#include "iconfonts.h"

#include <QFile>
#include <QFont>

namespace IconFonts {
namespace Private {
[[nodiscard]] FontId loadApplicationFont(const QMetaType &font, const QString &fileName);
[[nodiscard]] QFont loadApplicationFont(FontId fontId);
[[nodiscard]] QString readText(const QString &filePath);
} // namespace Private

template<symbol_enum S>
FontId fontId()
{
    static const auto s_fontId = [] {
        if (Private::loadResources<S>())
            return Private::loadApplicationFont(QMetaType::fromType<S>(), fontFileName<S>());

        return FontId{};
    }();

    return s_fontId;
}

template<symbol_enum S>
QFont font()
{
    static const auto s_font = [] {
        switch (type<S>()) {
        case FontInfo::Type::Invalid:
            return QFont{};
        case FontInfo::Type::Application:
            return Private::loadApplicationFont(fontId<S>());
        case FontInfo::Type::System:
            return QFont{fontFamily<S>()};
        }

        Q_UNREACHABLE_RETURN(QFont{});
    }();

    return s_font;
}

template<symbol_enum S>
QString licenseText()
{
    if (Private::loadResources<S>())
        return Private::readText(licenseFileName<S>());

    return {};
}

} // namespace IconFonts

#endif // ICONFONTS_ICONFONTS_P_H
