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

template<symbol_enum T>
FontId fontId()
{
    static const auto s_fontId = [] {
        if (Private::loadResources<T>())
            return Private::loadApplicationFont(QMetaType::fromType<T>(), fontFileName<T>());

        return FontId{};
    }();

    return s_fontId;
}

template<symbol_enum T>
QFont font()
{
    static const auto s_font = [] {
        switch (type<T>()) {
        case FontInfo::Type::Invalid:
            return QFont{};
        case FontInfo::Type::Application:
            return Private::loadApplicationFont(fontId<T>());
        case FontInfo::Type::System:
            return QFont{fontFamily<T>()};
        }

        Q_UNREACHABLE_RETURN(QFont{});
    }();

    return s_font;
}

template<symbol_enum T>
QString licenseText()
{
    if (Private::loadResources<T>())
        return Private::readText(licenseFileName<T>());

    return {};
}

template<symbol_enum T>
const FontInfo &FontInfo::instance() noexcept
{
    static const auto s_instance = FontInfo{T{}};
    return s_instance;
}

} // namespace IconFonts

#endif // ICONFONTS_ICONFONTS_P_H
