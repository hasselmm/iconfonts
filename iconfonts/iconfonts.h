#ifndef ICONFONTS_ICONFONTS_H
#define ICONFONTS_ICONFONTS_H

#include "iconfontsconfig.h"

#include "namedoptions.h"

#include <QColor>
#include <QFont>
#include <QIcon>
#include <QMetaType>
#include <QPalette>
#include <QTransform>

#include <source_location>

class QAction;

namespace IconFonts {

class FontIcon;
class Symbol;

// concepts // =========================================================================================================

template<typename T> struct is_symbol_enum : public std::false_type {};
template<typename T> constexpr bool is_symbol_enum_v = is_symbol_enum<T>::value;
template<typename T> concept symbol_enum = std::is_enum_v<T> && is_symbol_enum_v<T>;

template<typename T> struct is_symbol : public std::disjunction<is_symbol_enum<T>, std::is_same<Symbol, T>> {};
template<typename T> constexpr bool is_symbol_v = is_symbol<T>::value;
template<typename T> concept symbol_type = is_symbol_v<T>;

template<typename T> struct is_icon : public std::is_same<FontIcon, T> {};
template<typename T> constexpr bool is_icon_v = is_icon<T>::value;
template<typename T> concept icon_type = is_icon_v<T>;

template<typename T> struct is_icon_assignable : public std::disjunction<is_symbol<T>, is_icon<T>> {};
template<typename T> constexpr bool is_icon_initializer_v = is_icon_assignable<T>::value;
template<typename T> concept icon_initializer = is_icon_initializer_v<T>;

// a fake symbol enum for testing
namespace Private { enum class FakeSymbol {}; }
template<> struct is_symbol_enum<Private::FakeSymbol> : public std::true_type {};

// validate the concepts defined above
static_assert(!is_symbol_enum_v<FontIcon>);
static_assert(!is_symbol_enum_v<Symbol>);
static_assert( is_symbol_enum_v<Private::FakeSymbol>);
static_assert(!is_symbol_enum_v<QIcon::Mode>);

static_assert(!is_symbol_v<FontIcon>);
static_assert( is_symbol_v<Symbol>);
static_assert( is_symbol_v<Private::FakeSymbol>);
static_assert(!is_symbol_v<QIcon::Mode>);

static_assert( is_icon_v<FontIcon>);
static_assert(!is_icon_v<Symbol>);
static_assert(!is_icon_v<Private::FakeSymbol>);
static_assert(!is_icon_v<QIcon::Mode>);

static_assert( is_icon_initializer_v<FontIcon>);
static_assert( is_icon_initializer_v<Symbol>);
static_assert( is_icon_initializer_v<Private::FakeSymbol>);
static_assert(!is_icon_initializer_v<QIcon::Mode>);

// FontId struct // ====================================================================================================

struct FontId
{
    static constexpr int Invalid = -1;

    int value;

    constexpr FontId() noexcept : value{Invalid} {}
    constexpr FontId(int value) noexcept : value{value} {}

    [[nodiscard]] constexpr operator int() const noexcept { return value; }
    [[nodiscard]] constexpr bool isValid() const noexcept { return value != Invalid; }
    [[nodiscard]] constexpr explicit operator bool() const noexcept = delete;
};

static_assert(!FontId{}.isValid());
static_assert(FontId{} == FontId::Invalid);
static_assert(static_cast<FontId>(13) == FontId{13});

// FontInfo class // ===================================================================================================

class ICONFONTS_EXPORT FontInfo final
{
    Q_GADGET

public:
    enum class Type {
        Invalid,
        Application,
        System,
    };

    Q_ENUM(Type)

    constexpr FontInfo() noexcept = default;

    template<symbol_enum S>
    constexpr FontInfo(S = {})
        : d{Data::instance<S>()}
    {
        Q_ASSERT(isValid());
    }

    [[nodiscard]] Type type() const { return d ? d->type : Type::Invalid; }
    [[nodiscard]] QMetaType enumType() const { return d ? d->enumType : QMetaType{}; }
    [[nodiscard]] QFont font() const { return d && d->font ? d->font() : QFont{}; }
    [[nodiscard]] FontId fontId() const { return d && d->fontId ? d->fontId() : FontId{}; }
    [[nodiscard]] QString fontName() const { return d && d->fontName ? d->fontName() : QString{}; }
    [[nodiscard]] QString fontFamily() const { return d && d->fontFamily ? d->fontFamily() : QString{}; }
    [[nodiscard]] QString licenseFileName() const { return d && d->licenseFileName ? d->licenseFileName() : QString{}; }
    [[nodiscard]] QString licenseText() const { return d && d->licenseText ? d->licenseText() : QString{}; }

    [[nodiscard]] bool isNull() const;
    [[nodiscard]] bool isValid() const;
    [[nodiscard]] bool isAvailable() const;
    [[nodiscard]] int symbolCount() const;
    [[nodiscard]] inline Symbol symbol(int index) const;
    [[nodiscard]] int indexOf(char32_t unicode) const;
    [[nodiscard]] char32_t unicode(int index) const;
    [[nodiscard]] const char *key(int index) const;
    [[nodiscard]] QString name(int index) const;

    template<symbol_enum S>
    [[nodiscard]] ICONFONTS_EXPORT static const FontInfo &instance() noexcept;
    [[nodiscard]] static QList<FontInfo> knownFonts();

    [[nodiscard]] operator QFont() const { return font(); }

    friend inline bool operator==(const FontInfo &lhs, const FontInfo &rhs) noexcept;

private:
    [[nodiscard]] QMetaEnum metaEnum() const;

    struct Data final
    {
        Type        type;
        QMetaType   enumType = {};
        FontId   (* fontId)() = nullptr;
        QString  (* fontName)() = nullptr;
        QString  (* fontFamily)() = nullptr;
        QString  (* licenseFileName)() = nullptr;
        QString  (* licenseText)() = nullptr;
        QFont    (* font)() = nullptr;

        template<symbol_enum S>
        [[nodiscard]] static inline const Data *instance();

    private:
        template<symbol_enum S>
        constexpr Data(S = {});
    };

    const Data *d = nullptr;
};

template<symbol_enum S> [[nodiscard]] ICONFONTS_EXPORT QFont font();
template<symbol_enum S> [[nodiscard]] ICONFONTS_EXPORT FontId fontId();
template<symbol_enum S> [[nodiscard]] constexpr FontInfo::Type type();
template<symbol_enum S> [[nodiscard]] ICONFONTS_EXPORT QString fontName();
template<symbol_enum S> [[nodiscard]] ICONFONTS_EXPORT QString fontFamily();
template<symbol_enum S> [[nodiscard]] ICONFONTS_EXPORT QString fontFileName();
template<symbol_enum S> [[nodiscard]] ICONFONTS_EXPORT QString licenseFileName();
template<symbol_enum S> [[nodiscard]] ICONFONTS_EXPORT QString licenseText();

namespace Private {
template<symbol_enum S> [[nodiscard]] ICONFONTS_EXPORT bool loadResources();
} // namespace Private

template<symbol_enum S>
constexpr FontInfo::Data::Data(S)
    : type{IconFonts::type<S>()}
    , enumType{QMetaType::fromType<S>()}
    , fontId{&IconFonts::fontId<S>}
    , fontName{&IconFonts::fontName<S>}
    , fontFamily{&IconFonts::fontFamily<S>}
    , licenseFileName{&IconFonts::licenseFileName<S>}
    , licenseText{&IconFonts::licenseText<S>}
    , font{&IconFonts::font<S>}
{}

template<symbol_enum S>
inline const FontInfo::Data *FontInfo::Data::instance()
{
    thread_local static constexpr auto s_instance = Data{S{}};
    return &s_instance;
}

// Symbol class // =====================================================================================================

class ICONFONTS_EXPORT Symbol final
{
    Q_GADGET
    Q_PROPERTY(bool    isNull  READ isNull  CONSTANT FINAL)
    Q_PROPERTY(QFont   font    READ font    CONSTANT FINAL)
    Q_PROPERTY(QString name    READ name    CONSTANT FINAL)
    Q_PROPERTY(uint    unicode READ unicode CONSTANT FINAL)

public:
    constexpr Symbol() noexcept = default;
    constexpr Symbol(const FontInfo &font, char32_t unicode) noexcept
        : m_font{font}
        , m_unicode{unicode}
    {}

    template<symbol_enum S>
    constexpr Symbol(S symbol) noexcept;

    [[nodiscard]] constexpr char32_t unicode() const { return m_unicode; }
    [[nodiscard]] inline FontInfo fontInfo() const { return m_font; }
    [[nodiscard]] inline const char *key() const;
    [[nodiscard]] inline QString name() const;
    [[nodiscard]] inline bool isNull() const;
    [[nodiscard]] inline QFont font() const;

    [[nodiscard]] QString toString() const
    {
        const auto ch = QChar::fromUcs4(unicode());
        return QString::fromUtf16(ch.begin(), ch.size());
    }

    [[nodiscard]] operator QFont() const { return font(); }
    [[nodiscard]] operator QString() const { return toString(); }
    [[nodiscard]] operator QStringView() const { return toString(); }

    [[nodiscard]] constexpr auto fields() const noexcept { return std::tie(m_font, m_unicode); }
    friend constexpr bool operator==(const Symbol &l, const Symbol &r) noexcept { return l.fields() == r.fields(); }

private:
    FontInfo m_font;
    char32_t m_unicode = 0;
};

// implementations // --------------------------------------------------------------------------------------------------

template<symbol_enum S>
constexpr Symbol::Symbol(S symbol) noexcept
    : m_font{FontInfo::instance<S>()}
    , m_unicode{std::to_underlying(symbol)}
{}

const char *Symbol::key() const { return m_font.key(m_font.indexOf(m_unicode)); }
QString Symbol::name() const { return m_font.name(m_font.indexOf(m_unicode)); }
QFont Symbol::font() const { return m_font.font(); }
bool Symbol::isNull() const { return m_font.isNull(); }

Symbol FontInfo::symbol(int index) const { return {*this, unicode(index)}; }

// FontIcon class // ===================================================================================================

struct ICONFONTS_EXPORT DrawIconOptions : public named_options::options<bool, int, qreal>
{
    option<0, bool>  fillRect  = {};
    option<1, int>   pixelSize = {};
    option<2, qreal> pointSize = {};

    bool applyColor      = true;
    QIcon::Mode iconMode = QIcon::Normal;

    bool operator==(const DrawIconOptions &) const = default;
};

class ICONFONTS_EXPORT FontIcon final
{
    Q_GADGET

public:
    enum class Transform {
        None,
        HorizontalFlip,
        VerticalFlip,
        Rotate90,
        Rotate180,
        Rotate270,
        Matrix,
    };

    Q_ENUM(Transform);

    // forwarding constructors

    constexpr FontIcon() noexcept = default;

    template<typename ...Args>
    constexpr FontIcon(Args &&...args, const QColor &color = {},
                       Transform transform = Transform::None) noexcept
        : m_symbol{std::forward<Args>(args)...}
        , m_transform{instance(transform)}
        , m_color{color}
    {}

    // classical constructors

    template<symbol_enum S>
    FontIcon(S symbol, const QColor &color = {},
             Transform transform = Transform::None) noexcept
        : FontIcon{Symbol{symbol}, color, TransformVariant{transform}} {}

    FontIcon(const Symbol &symbol, const QColor &color = {},
             Transform transform = Transform::None) noexcept
        : FontIcon{symbol, color, TransformVariant{transform}} {}

    // modifying constructors

    FontIcon(const FontIcon &icon, const Symbol &symbol) noexcept
        : FontIcon{symbol, icon.m_color, icon.m_transform} {}

    FontIcon(const FontIcon &icon, const QColor &color) noexcept
        : FontIcon{icon.symbol(), color, icon.m_transform} {}

    FontIcon(const FontIcon &icon, Transform transform) noexcept
        : FontIcon{icon.symbol(), icon.m_color, transform} {}

    FontIcon(const FontIcon &icon, const QTransform &transform) noexcept
        : FontIcon{icon.symbol(), icon.m_color, optimize(transform)} {}

    // observers

    [[nodiscard]] Symbol symbol() const { return m_symbol; }
    [[nodiscard]] constexpr QColor color() const { return m_color; }
    [[nodiscard]] const QTransform &transform() const;
    [[nodiscard]] Transform transformType() const;

    [[nodiscard]] static const QTransform &transform(Transform transform) { return *instance(transform); }

    [[nodiscard]] inline bool isNull() const;
    [[nodiscard]] QString name() const { return m_symbol.name(); }

    [[nodiscard]] QIcon toIcon() const;
    [[nodiscard]] operator QIcon() const { return toIcon(); }

    [[nodiscard]] constexpr auto fields() const noexcept { return std::tie(m_symbol, m_color, m_transform); }
    friend constexpr bool operator==(const FontIcon &l, const FontIcon &r) noexcept { return l.fields() == r.fields(); }

    // operations

    void draw(QPainter *painter, const QRectF &rect, const QPalette &palette, const DrawIconOptions &options = {},
              const std::source_location &source = std::source_location::current()) const;
    void draw(QPainter *painter, const QSizeF &size, const QPalette &palette, const DrawIconOptions &options = {},
              const std::source_location &source = std::source_location::current()) const;

private:
    using TransformVariant = std::variant<std::monostate, Transform, std::shared_ptr<QTransform>>;

    FontIcon(const Symbol &symbol, const QColor &color,
             const TransformVariant &transform) noexcept
        : m_symbol{symbol}
        , m_transform{transform}
        , m_color{color}
    {}

    [[nodiscard]] QColor effectiveColor(const QPalette &palette, QIcon::Mode mode) const;

    void drawImmediatly(QPainter *painter, const QRectF &rect, const QFont &font) const;
    void drawAlphaBlended(QPainter *painter, const QRectF &rect, const QFont &font, const QColor &color) const;

    [[nodiscard]] TransformVariant optimize(const QTransform &transform);
    [[nodiscard]] static std::shared_ptr<QTransform> instance(Transform transform);

    Symbol              m_symbol;
    TransformVariant    m_transform;
    QColor              m_color;
    QPalette::ColorRole m_role = QPalette::Text;
};

// implementations // --------------------------------------------------------------------------------------------------

bool FontIcon::isNull() const
{
    return m_symbol.isNull()
            && !m_color.isValid()
            && std::holds_alternative<std::monostate>(m_transform);
}

// ModalFontIcon struct // =============================================================================================

struct ICONFONTS_EXPORT ModalFontIcon final
{
    FontIcon on;
    FontIcon off;

    // observers

    [[nodiscard]] bool isNull() const { return on.isNull() && off.isNull(); }
    [[nodiscard]] QString name() const;

    [[nodiscard]] QIcon toIcon() const;
    [[nodiscard]] operator QIcon() const { return toIcon(); }

    [[nodiscard]] constexpr auto fields() const { return std::tie(on, off); }
    friend constexpr bool operator==(const ModalFontIcon &l, const ModalFontIcon &r) noexcept { return l.fields() == r.fields(); }

    // operations

    void draw(QPainter *painter, const QRectF &rect, QIcon::State state,
              const QPalette &palette, const DrawIconOptions &options = {},
              const std::source_location &source = std::source_location::current()) const;
    void draw(QPainter *painter, const QSizeF &size, QIcon::State state,
              const QPalette &palette, const DrawIconOptions &options = {},
              const std::source_location &source = std::source_location::current()) const;
};

// freestanding observers // ===========================================================================================

template<symbol_enum S>
[[nodiscard]] constexpr char32_t unicode(S symbol) noexcept
{ return std::to_underlying<S>(symbol); }

template<symbol_enum S>
[[nodiscard]] inline QString toString(S symbol)
{
    const auto ch = QChar::fromUcs4(unicode(symbol));
    return QString::fromUtf16(ch.begin(), ch.size());
}

// utility functions // ================================================================================================

[[nodiscard]] ICONFONTS_EXPORT QAction *createAction(const QFont &font, QStringView iconName, QObject *parent);

template<symbol_enum S>
[[nodiscard]] inline QAction *createAction(S symbol, QObject *parent)
{ return createAction(font<S>(), toString(symbol), parent); }

template<symbol_enum S>
[[nodiscard]] inline QAction *createAction(QStringView iconName, QObject *parent)
{ return createAction(font<S>(), iconName, parent); }

// equality operators ==================================================================================================

[[nodiscard]] inline bool operator==(const FontInfo &lhs, const FontInfo &rhs) noexcept
{
    if (lhs.d == rhs.d)
        return true;
    if (lhs.d && rhs.d)
        return lhs.d->enumType == rhs.d->enumType;

    return false;
}

// constructing operators ==============================================================================================

template<icon_initializer S, typename T>
inline FontIcon operator|(S symbol, T &&option) noexcept
{
    return {symbol, std::forward<T>(option)};
}

template<typename T>
inline FontIcon &operator|=(FontIcon &icon, T &&option) noexcept
{
    return icon = icon | std::forward<T>(option);
}

template<icon_initializer T, icon_initializer U>
inline ModalFontIcon operator^(const T &on, const U &off)
{
    return {.on = on, .off = off};
}

// stream operators ====================================================================================================

ICONFONTS_EXPORT QDebug operator<<(QDebug debug, const FontInfo &info);
ICONFONTS_EXPORT QDebug operator<<(QDebug debug, const Symbol &symbol);
ICONFONTS_EXPORT QDebug operator<<(QDebug debug, const FontIcon &icon);
ICONFONTS_EXPORT QDebug operator<<(QDebug debug, const ModalFontIcon &icon);

} // IconFonts

#endif // ICONFONTS_ICONFONTS_H
