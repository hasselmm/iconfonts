#include "iconfonts_p.h"

#include <QAction>
#include <QFile>
#include <QFontDatabase>
#include <QIconEngine>
#include <QLoggingCategory>
#include <QMetaEnum>
#include <QPainter>
#include <QPixmapCache>

#include <QtGui/private/qfontengine_p.h>

namespace IconFonts {

using namespace Private;
using namespace Qt::StringLiterals;

namespace Private {
namespace {

Q_LOGGING_CATEGORY(lcIconFonts, "iconfonts");

class FontIconEngine : public QIconEngine
{
public:
    explicit FontIconEngine(ModalFontIcon &&icon) : m_icon{std::move(icon)} {}
    explicit FontIconEngine(const ModalFontIcon &icon) : m_icon{icon} {}

    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;

    QString key() const override;
    QIconEngine *clone() const override { return new FontIconEngine{m_icon}; }

    QString iconName() override {  return m_icon.name(); }

    bool isNull() override;

private:
    ModalFontIcon m_icon;
};

template<typename T>     [[nodiscard]] QString cacheKey(const T &);
template<typename ...Ts> [[nodiscard]] QString cacheKey(const Ts &...fields);
template<typename ...Ts> [[nodiscard]] QString cacheKey(const std::tuple<Ts...> &tuple);
template<typename ...Ts> [[nodiscard]] QString cacheKey(const std::variant<Ts...> &tuple);

template<typename T> requires std::is_enum_v<T>
[[nodiscard]] QString cacheKey(const T &value)
{
    return QString::number(std::to_underlying<T>(value), 36);
}

template<typename T>
[[nodiscard]] QString cacheKey(const std::shared_ptr<T> &ptr)
{
    return ptr ? cacheKey<T>(*ptr) : u"-"_s;
}

template<> [[nodiscard]] QString cacheKey(const std::monostate &) { return u"-"_s; }
template<> [[nodiscard]] QString cacheKey(const qreal &decimal)   { return QString::number(decimal); }
template<> [[nodiscard]] QString cacheKey(const int &number)      { return QString::number(number, 36); }
template<> [[nodiscard]] QString cacheKey(const char32_t &code)   { return QString::number(code, 36); }
template<> [[nodiscard]] QString cacheKey(const FontInfo &font)   { return cacheKey(font.enumType().id()); }
template<> [[nodiscard]] QString cacheKey(const FontIcon &icon)   { return cacheKey(icon.fields()); }
template<> [[nodiscard]] QString cacheKey(const Symbol &symbol)   { return cacheKey(symbol.fields()); }

template<>
[[nodiscard]] QString cacheKey(const QColor &color)
{
    if (color.spec() == QColor::ExtendedRgb)
        return QString::number(color.rgba64(), 36);
    else if (color.spec() != QColor::Invalid)
        return QString::number(color.rgba(), 36);
    else
        return u"-"_s;
}

template<typename ...Ts>
[[nodiscard]] QString cacheKey(const Ts &...fields)
{
    return u'(' + QStringList{cacheKey(fields)...}.join(u'/') + u')';
}

template<typename ...Ts>
[[nodiscard]] QString cacheKey(const std::tuple<Ts...> &tuple)
{
    return std::apply<QString(*)(const Ts &...)>(cacheKey<Ts...>, tuple);
}

template<>
[[nodiscard]] QString cacheKey(const QTransform &transform)
{
    return cacheKey(transform.m11(), transform.m12(), transform.m13(),
                    transform.m21(), transform.m22(), transform.m23(),
                    transform.m31(), transform.m32(), transform.m33());
}

template<typename ...Ts>
[[nodiscard]] QString cacheKey(const std::variant<Ts...> &variant)
{
    return std::visit([](const auto &field) {
        return cacheKey(field);
    }, variant);
}

QPixmap FontIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    const auto effectiveSize = std::min(size.width(), size.height());
    const auto &key = cacheKey(m_icon.fields(), mode, state, effectiveSize);

    auto pixmap = QPixmap{};

    if (!key.isEmpty() && QPixmapCache::find(key, &pixmap))
        return pixmap;

    auto image = QImage{size, QImage::Format_ARGB32};
    image.fill(Qt::transparent);

    pixmap = QPixmap::fromImage(image, Qt::NoFormatConversion);

    {
        auto painter = QPainter{&pixmap};
        paint(&painter, {0, 0, size.width(), size.height()}, mode, state);
    }

    QPixmapCache::insert(key, pixmap);
    return pixmap;
}

void FontIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    painter->setBackground(Qt::transparent);
    painter->eraseRect(rect);

    // FIXME: get access to palette
    m_icon.draw(painter, rect, state, {}, {.fillBox = true, .mode = mode});
}

QString FontIconEngine::key() const
{
    return cacheKey(m_icon.fields());
}

bool FontIconEngine::isNull()
{
    return m_icon.isNull();
}

template<FontIcon::Transform> QTransform init();

template<> QTransform init<FontIcon::Transform::None>()             { return QTransform{}; }
template<> QTransform init<FontIcon::Transform::HorizontalFlip>()   { return QTransform::fromScale(-1, 1); }
template<> QTransform init<FontIcon::Transform::VerticalFlip>()     { return QTransform::fromScale(1, -1); }
template<> QTransform init<FontIcon::Transform::Rotate90>()         { return QTransform{}.rotate(90); }
template<> QTransform init<FontIcon::Transform::Rotate180>()        { return QTransform{}.rotate(180); }
template<> QTransform init<FontIcon::Transform::Rotate270>()        { return QTransform{}.rotate(270); }

template<FontIcon::Transform Transform>
[[nodiscard]] std::shared_ptr<QTransform> instance()
{
    thread_local static const auto s_instance
            = std::make_shared<QTransform>(init<Transform>());

    return s_instance;
}

[[nodiscard]] QChar::Script script(const Symbol &symbol)
{
    if (const auto &text = symbol.toString(); !text.isEmpty())
        return text.at(0).script();

    return QChar::Script_Unknown;
}

[[nodiscard]] QFontEngine::GlyphFormat glyphFormat(const QFont &font, const Symbol &symbol)
{
    if (const auto fontPrivate = QFontPrivate::get(font)) {
        if (const auto fontEngine = fontPrivate->engineForScript(script(symbol))) {
            if (const auto multiEngine = dynamic_cast<const QFontEngineMulti *>(fontEngine)) {
                if (const auto preferedEngine = multiEngine->engine(0))
                    return preferedEngine->glyphFormat;
            } else {
                return fontEngine->glyphFormat;
            }
        }
    }

    return QFontEngine::Format_None;
}

[[nodiscard]] constexpr bool isEnumeration(const QMetaType &type) noexcept
{
    return type.flags().testFlag(QMetaType::IsEnumeration);
}

[[nodiscard]] auto makeColorResolver(const QPalette &palette)
{
    return [palette](QIcon::Mode mode, QPalette::ColorRole role)
    {
        switch (mode) {
        case QIcon::Mode::Disabled:
            return palette.color(QPalette::Disabled, role);
        case QIcon::Mode::Normal:
            return palette.color(QPalette::Inactive, role);
        case QIcon::Mode::Active:
            return palette.color(QPalette::Active,   role);
        case QIcon::Mode::Selected:
            return palette.color(QPalette::Active,   QPalette::HighlightedText);
        }

        Q_UNREACHABLE_RETURN(QColor{});
    };
}

auto knownFonts()
{
    static auto s_fontRegistry = QHash<FontTag, FontInfo>{};
    return &s_fontRegistry;
}

} // namespace

FontId loadApplicationFont(const QMetaType &font, const QString &fileName)
{
    if (!QFile::exists(fileName)) {
        qCWarning(lcIconFonts,
                  R"(Cannot find embedded resource file for %s: "%ls")",
                  font.name(), qUtf16Printable(fileName));

        return FontId::Invalid;
    }

    const auto fontId = QFontDatabase::addApplicationFont(fileName);

    if (fontId < 0) {
        qCWarning(lcIconFonts,
                  R"(Cannot load font %s from embedded resource: "%ls")",
                  font.name(), qUtf16Printable(fileName));

        return FontId::Invalid;
    }

    qCDebug(lcIconFonts, "%s is available via font id %d", font.name(), fontId);
    return fontId;
}

QFont loadApplicationFont(FontId fontId)
{
    return QFont{QFontDatabase::applicationFontFamilies(fontId)};
}

QString readText(const QString &filePath)
{
    auto file = QFile{filePath};

    if (Q_UNLIKELY(!file.open(QFile::ReadOnly))) {
        qCWarning(lcIconFonts,
                  R"(Cannot read from "%ls": %ls)",
                  qUtf16Printable(file.fileName()),
                  qUtf16Printable(file.errorString()));

        return {};
    }

    return QString::fromUtf8(file.readAll());
}

} // namespace Private

QAction *createAction(const QFont &font, QStringView iconName, QObject *parent)
{
    const auto action = new QAction{iconName.toString(), parent};
    action->setFont(font);
    return action;
}

const QTransform &FontIcon::transform() const
{
    if (const auto transform = std::get_if<Transform>(&m_transform))
        return *instance(*transform);
    if (const auto transform = std::get_if<std::shared_ptr<QTransform>>(&m_transform))
        return *transform->get();

    return *Private::instance<Transform::None>();
}

IconFonts::FontIcon::Transform FontIcon::transformType() const
{
    const struct {
        Transform operator()(const std::shared_ptr<QTransform> &) const { return Transform::Matrix; }
        Transform operator()(const std::monostate &) const { return Transform::None; }
        Transform operator()(Transform transform) const { return transform; }
    } visitor;

    return std::visit(visitor, m_transform);
}

QIcon FontIcon::toIcon() const
{
    return QIcon{new FontIconEngine{{.on = *this, .off = *this}}};
}

void FontIcon::draw(QPainter *painter, const QRectF &rect, const QPalette &palette,
                    const DrawIconOptions &options, QIcon::Mode fallbackMode) const
{
    auto font = symbol().font();

    if (options.fillBox.value_or(false))
        font.setPixelSize(std::min(rect.width(), rect.height()));
    else if (options.pixelSize)
        font.setPixelSize(*options.pixelSize);
    else if (options.pointSize)
        font.setPointSizeF(*options.pointSize);

    painter->save();

    const auto effectiveColor = options.effectiveColor(m_color, palette, fallbackMode);

    if (!effectiveColor.isValid()) {
        drawImmediatly(painter, rect, font);
    } else if (glyphFormat(font, symbol()) == QFontEngine::Format_ARGB) {
        drawAlphaBlended(painter, rect, font, effectiveColor);
    } else {
        painter->setPen(effectiveColor);
        drawImmediatly(painter, rect, font);
    }

    painter->restore();
}

void FontIcon::draw(QPainter *painter, const QSizeF &size, const QPalette &palette,
                    const DrawIconOptions &options, QIcon::Mode fallbackMode) const
{
    const auto rect = QRectF{0, 0, size.width(), size.height()};
    return draw(painter, rect, palette, options, fallbackMode);
}

QColor DrawIconOptions::effectiveColor(const QColor &color, const QPalette &palette,
                                       DrawIconOptions::IconMode fallbackMode) const
{
    return effectiveColor(color, makeColorResolver(palette), fallbackMode);
}

QColor DrawIconOptions::effectiveColor(const QColor &color, ColorResolver resolveColor, IconMode fallbackMode) const
{
    const auto effectiveMode = mode.value_or(fallbackMode);

    if (effectiveMode != Disabled
            && applyColor
            && color.isValid())
        return color;

    if (!resolveColor)
        resolveColor = makeColorResolver(QPalette{}); // FIXME: access widget/item palette?

    return resolveColor(effectiveMode, role.value_or(Text));
}

void FontIcon::drawImmediatly(QPainter *painter, const QRectF &rect, const QFont &font) const
{
    painter->setRenderHints(QPainter::Antialiasing
                            | QPainter::TextAntialiasing
                            | QPainter::VerticalSubpixelPositioning);

    const auto cx = rect.width() / 2 + rect.x();
    const auto cy = rect.height() / 2 + rect.y();

    auto transform = FontIcon::transform();
    transform *= QTransform::fromTranslate(cx, cy);
    transform.translate(-cx, -cy);

    painter->setFont(font);
    painter->setTransform(transform);

    painter->drawText(rect, Qt::AlignCenter, symbol().toString());
}

void FontIcon::drawAlphaBlended(QPainter *painter, const QRectF &rect, const QFont &font, const QColor &color) const
{
    const auto scale = painter->device()->devicePixelRatioF();
    const auto height = qCeil(rect.height() * scale);
    const auto width = qCeil(rect.width() * scale);

    // TODO: Find way to calculate a smaller bounding box, that still respects transformation.
    // For instance rotating pushes the icon outside the box reported by QFontMetrics::boundingBox().
    auto image = QImage{width, height, QImage::Format_ARGB32};
    image.setDevicePixelRatio(scale);
    image.fill(Qt::transparent);

    auto imagePainter = QPainter{&image};

    imagePainter.setRenderHints(QPainter::Antialiasing
                                | QPainter::TextAntialiasing
                                | QPainter::VerticalSubpixelPositioning);

    drawImmediatly(&imagePainter, {0, 0, rect.width(), rect.height()}, font);
    imagePainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    imagePainter.fillRect(0, 0, image.width(), image.height(), color);

    painter->drawImage(rect.x(), rect.y(), image);
}

FontIcon::TransformVariant FontIcon::optimize(const QTransform &transform)
{
    constexpr auto is90Degrees = [](const QTransform &transform) {
        return transform.m11() == 0 && transform.m22() == 0;
    };

    constexpr auto isXAxisOnly = [](const QTransform &transform) {
        return transform.m33() == 1
                && transform.m31() == 0 && transform.m32() == 0
                && transform.m13() == 0 && transform.m23() == 0;
    };

    switch (transform.type()) {
    case QTransform::TxNone:
        return Transform::None;

    case QTransform::TxScale:
        if (transform.m11() == -1) {
            if (transform.m22() == 1) {
                if (transform.m33() == 1)
                    return Transform::HorizontalFlip;
            } else if (transform.m22() == -1) {
                if (transform.m33() == 1)
                    return Transform::Rotate180;
            }
        } else if (transform.m11() == 1) {
            if (transform.m22() == -1 && transform.m33() == 1)
                return Transform::VerticalFlip;
        }

        break;

    case QTransform::TxRotate:
        if (transform.m12() == 1 && transform.m21() == -1) {
            if (is90Degrees(transform) && isXAxisOnly(transform))
                return Transform::Rotate90;
        } else if (transform.m12() == -1 && transform.m21() == 1) {
            if (is90Degrees(transform) && isXAxisOnly(transform))
                return Transform::Rotate270;
        }

        break;

    case QTransform::TxTranslate:
    case QTransform::TxShear:
    case QTransform::TxProject:
        break;
    }

    return std::make_shared<QTransform>(transform);
}

std::shared_ptr<QTransform> FontIcon::instance(Transform transform)
{
    switch (transform) {
    case Transform::None:           return Private::instance<Transform::None>();
    case Transform::HorizontalFlip: return Private::instance<Transform::HorizontalFlip>();
    case Transform::VerticalFlip:   return Private::instance<Transform::VerticalFlip>();
    case Transform::Rotate90:       return Private::instance<Transform::Rotate90>();
    case Transform::Rotate180:      return Private::instance<Transform::Rotate180>();
    case Transform::Rotate270:      return Private::instance<Transform::Rotate270>();
    case Transform::Matrix:         return Private::instance<Transform::None>();
    }

    Q_UNREACHABLE_RETURN({});
}

bool FontInfo::isNull() const
{
    return d == nullptr;
}

bool FontInfo::isValid() const
{
    return isEnumeration(enumType())
            && metaEnum().isValid();
}

bool FontInfo::isAvailable() const
{
    if (Q_UNLIKELY(!isValid()))
        return false;

    switch (type()) {
    case Type::Invalid:
        return false;

    case Type::Application:
        return fontId().isValid();

    case Type::System:
        return QFontDatabase::hasFamily(fontFamily());
    };

    Q_UNREACHABLE_RETURN(false);
}

int FontInfo::symbolCount() const
{
    return metaEnum().keyCount();
}

int FontInfo::indexOf(char32_t unicode) const
{
    using UnicodeIndexMap = std::unordered_map<char32_t, int>;
    using MetaTypeUnicodeIndexMap = std::unordered_map<int, UnicodeIndexMap>;

    thread_local static auto s_indexes = MetaTypeUnicodeIndexMap{};

    if (Q_UNLIKELY(isNull()))
        return -1;

    auto mapIt = s_indexes.find(d->enumType.id());

    if (Q_UNLIKELY(mapIt == s_indexes.end())) {
        mapIt = s_indexes.emplace(d->enumType.id(), UnicodeIndexMap{}).first;

        const auto iconCount = FontInfo::symbolCount();
        auto &map = mapIt->second;
        map.reserve(iconCount);

        for (auto i = 0; i < iconCount; ++i)
            map.emplace_hint(map.end(), FontInfo::unicode(i), i);
    }

    const auto &unicodeIndexMap = mapIt->second;

    if (const auto indexIt = unicodeIndexMap.find(unicode);
            Q_LIKELY(indexIt != unicodeIndexMap.end()))
        return indexIt->second;

    return -1;
}

char32_t FontInfo::unicode(int index) const
{
    const auto value = metaEnum().value(index);
    return static_cast<char32_t>(value);
}

const char *FontInfo::key(int index) const
{
    return metaEnum().key(index);
}

QString FontInfo::name(int index) const
{
    if (const auto key = FontInfo::key(index); Q_LIKELY(key != nullptr))
        return QString::fromLatin1(key);

    return {};
}

FontInfo FontInfo::fromTag(FontTag tag)
{
    return Private::knownFonts()->value(tag);
}

QList<FontInfo> FontInfo::knownFonts()
{
    static auto s_knownFonts = QList<FontInfo>{};

    if (s_knownFonts.size() != Private::knownFonts()->size()) {
        auto newKnownFonts = Private::knownFonts()->values();

        std::sort(newKnownFonts.begin(), newKnownFonts.end(),
                  [](const FontInfo &l, const FontInfo &r) {
            return std::make_tuple(l.fontName(), l.tag().value())
                    < std::make_tuple(r.fontName(), r.tag().value());
        });

        s_knownFonts = std::move(newKnownFonts);
    }

    return s_knownFonts;
}

bool FontInfo::registerFont(const FontInfo &font)
{
    const auto knownFonts = Private::knownFonts();

    if (const auto it = knownFonts->constFind(font.tag()); it == knownFonts->constEnd()) {
        knownFonts->insert(font.tag(), font);
        return true;
    } else if (*it != font) {
        qCWarning(lcIconFonts,
                  R"(Other font already registered for index %d: "%ls")",
                  font.tag().index(), qUtf16Printable(it->fontName()));

        return false;
    } else {
        qCWarning(lcIconFonts,
                  R"(Font "%ls" is already registered)",
                  qUtf16Printable(it->fontName()));

        return false;
    }
}

QMetaEnum FontInfo::metaEnum() const
{
    if (d && d->enumType.isValid()) {
        if (const auto metaObject = d->enumType.metaObject()) {
            Q_ASSERT(metaObject->enumeratorCount() == 1);
            return metaObject->enumerator(0);
        }
    }

    return {};
}

QString ModalFontIcon::name() const
{
    if (on == off)
        return on.name();

    return on.name() + u'^' + off.name();
}

QIcon ModalFontIcon::toIcon() const
{
    return QIcon{new FontIconEngine{*this}};
}

void ModalFontIcon::draw(QPainter *painter, const QRectF &rect, QIcon::State state,
                         const QPalette &palette, const DrawIconOptions &options,
                         QIcon::Mode fallbackMode) const
{
    switch (state) {
    case QIcon::On:
        on.draw(painter, rect, palette, options, fallbackMode);
        break;

    case QIcon::Off:
        off.draw(painter, rect, palette, options, fallbackMode);
        break;
    }
}

void ModalFontIcon::draw(QPainter *painter, const QSizeF &size, QIcon::State state,
                         const QPalette &palette, const DrawIconOptions &options,
                         QIcon::Mode fallbackMode) const
{
    const auto rect = QRectF{0, 0, size.width(), size.height()};
    return draw(painter, rect, state, palette, options, fallbackMode);
}

// stream operators ====================================================================================================

QDebug operator<<(QDebug debug, const FontInfo &fontInfo)
{
    if (fontInfo.isValid()) {
        return debug << "(family=" << fontInfo.font().family()
                     << ", type=" << fontInfo.type()
                     << ", license=" << fontInfo.licenseFileName()
                     << ")";
    } else {
        return debug << "(invalid font)";
    }
}

QDebug operator<<(QDebug debug, const Symbol &symbol)
{
    return debug << "(name=" << symbol.name()
                 << ", unicode=" << symbol.unicode()
                 << ", font=" << symbol.fontInfo()
                 << ")";
}

QDebug operator<<(QDebug debug, const FontIcon &icon)
{
    return debug << "(symbol=" << icon.symbol()
                 << ", color=" << icon.color()
                 << ", transform=" << icon.transformType()
                 << ")";
}

QDebug operator<<(QDebug debug, const ModalFontIcon &icon)
{
    if (icon.on == icon.off)
        return debug << icon.on;

    return debug << "(on=" << icon.on
                 << ", off=" << icon.off
                 << ")";
}

} // namespace IconFonts
