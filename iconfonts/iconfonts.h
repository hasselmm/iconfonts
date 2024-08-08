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

class QAction;

namespace IconFonts {

class FontIcon;
class Symbol;

// Concepts // =========================================================================================================

inline namespace Concepts {

template<typename T> struct is_symbol_enum : public std::false_type {};
template<typename T> constexpr bool is_symbol_enum_v = std::is_enum_v<T> && is_symbol_enum<T>::value;
template<typename T> concept symbol_enum = std::is_enum_v<T> && is_symbol_enum_v<T>;

template<typename T> struct is_symbol : public std::disjunction<is_symbol_enum<T>, std::is_same<Symbol, T>> {};
template<typename T> constexpr bool is_symbol_v = is_symbol<T>::value;

template<typename T> struct is_icon : public std::is_same<FontIcon, T> {};
template<typename T> constexpr bool is_icon_v = is_icon<T>::value;

template<typename T> struct is_icon_assignable : public std::disjunction<is_symbol<T>, is_icon<T>> {};
template<typename T> constexpr bool is_icon_initializer_v = is_icon_assignable<T>::value;
template<typename T> concept icon_initializer = is_icon_initializer_v<T>;

} // inline namespace Concepts

// a fake symbol enum for testing
namespace Tests { enum class TestSymbol { A = 'A', B, C }; }
template<> struct Concepts::is_symbol_enum<Tests::TestSymbol> : public std::true_type {};

// validate the concepts defined above
static_assert(!is_symbol_enum_v<FontIcon>);
static_assert(!is_symbol_enum_v<Symbol>);
static_assert( is_symbol_enum_v<Tests::TestSymbol>);
static_assert(!is_symbol_enum_v<QIcon::Mode>);

static_assert(!is_symbol_v<FontIcon>);
static_assert( is_symbol_v<Symbol>);
static_assert( is_symbol_v<Tests::TestSymbol>);
static_assert(!is_symbol_v<QIcon::Mode>);

static_assert( is_icon_v<FontIcon>);
static_assert(!is_icon_v<Symbol>);
static_assert(!is_icon_v<Tests::TestSymbol>);
static_assert(!is_icon_v<QIcon::Mode>);

static_assert( is_icon_initializer_v<FontIcon>);
static_assert( is_icon_initializer_v<Symbol>);
static_assert( is_icon_initializer_v<Tests::TestSymbol>);
static_assert(!is_icon_initializer_v<QIcon::Mode>);

// Tags // =============================================================================================================

inline namespace Tags {

template<typename T, T Minimum, T Maximum, std::size_t Shift = 0, T Invalid = T{}>
class TaggedValue
{
public:
    using value_type = T;
    value_type m_value; // FIXME figure out how to make m_value private again

    [[nodiscard]] static constexpr T         invalid() { return Invalid; }
    [[nodiscard]] static constexpr T         minimum() { return Minimum; }
    [[nodiscard]] static constexpr T         maximum() { return Maximum; }
    [[nodiscard]] static constexpr T       valueMask() { return (1 << std::bit_width(maximum())) - 1; }
    [[nodiscard]] static constexpr T         bitmask() { return valueMask() << shift(); }
    [[nodiscard]] static constexpr std::size_t shift() { return Shift; }

    static_assert((minimum() & valueMask()) == minimum());
    static_assert((maximum() & valueMask()) == maximum());
    static_assert( minimum()                <= maximum());

    [[nodiscard]] constexpr bool isValid() const noexcept { return  m_value != invalid(); }
    [[nodiscard]] constexpr T      index() const noexcept { return (m_value >> Shift) & valueMask(); }
    [[nodiscard]] constexpr T      value() const noexcept { return  m_value; }
    [[nodiscard]] constexpr operator   T() const noexcept { return  m_value; }

    template<typename U>
    requires(std::is_base_of_v<TaggedValue, U>)
    static constexpr U fromValue(value_type value) noexcept { return {value}; }

    template<typename U, auto min = U::minimum(), auto max = U::maximum(), auto shift = U::shift()>
    static constexpr bool is_compatible_tag_v = std::is_base_of_v<TaggedValue<T, min, max, shift>, U>
        && ((bitmask() & U::bitmask()) == 0);

    template<typename U, U::value_type index, typename... V>
    requires(index >= minimum() && index <= maximum()
             && std::is_base_of_v<TaggedValue, U>
             && (... && is_compatible_tag_v<std::remove_reference_t<V>>))
    static constexpr U make(V &&...otherTags) noexcept
    {
        constexpr auto value = ((index & valueMask()) << Shift);
        return {(value | ... | otherTags.value())};
    }

    constexpr TaggedValue() noexcept : m_value(invalid()) {}

private:
    constexpr TaggedValue(T value) noexcept : m_value{value} {}
};

struct FontTag : public TaggedValue<quint32, 1, 127, 24>
{
    template<quint32 index>
    static constexpr FontTag make() noexcept
    { return TaggedValue::make<FontTag, index>(); }

    static constexpr FontTag fromValue(quint32 value) noexcept
    { return TaggedValue::fromValue<FontTag>(value); }
};

struct SymbolTag : public TaggedValue<quint32, 1, 0xffffff>
{
    template<auto symbol, symbol_enum S = decltype(symbol)>
    static constexpr SymbolTag make() noexcept;

    static constexpr SymbolTag fromValue(quint32 value) noexcept
    { return TaggedValue::fromValue<SymbolTag>(value); }

    constexpr FontTag font() const noexcept
    { return FontTag::fromValue(value()); }

    constexpr char32_t unicode() const noexcept
    { return static_cast<char32_t>(index()); }
};

} // namespace Tags

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
    [[nodiscard]] FontTag tag() const { return d ? d->fontTag : FontTag{}; }
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
    [[nodiscard]] static FontInfo fromTag(FontTag tag);
    [[nodiscard]] static QList<FontInfo> knownFonts();

    [[nodiscard]] operator QFont() const { return font(); }

    friend inline bool operator==(const FontInfo &lhs, const FontInfo &rhs) noexcept;

private:
    [[nodiscard]] QMetaEnum metaEnum() const;

    struct Data final
    {
        Type        type;
        QMetaType   enumType = {};
        FontTag     fontTag;
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

// Symbol class // =====================================================================================================

class ICONFONTS_EXPORT Symbol final
{
    Q_GADGET
    Q_PROPERTY(bool    isNull  READ isNull   CONSTANT FINAL)
    Q_PROPERTY(QFont   font    READ font     CONSTANT FINAL)
    Q_PROPERTY(QString name    READ name     CONSTANT FINAL)
    Q_PROPERTY(QString text    READ toString CONSTANT FINAL)
    Q_PROPERTY(uint    unicode READ unicode  CONSTANT FINAL)

public:
    constexpr Symbol() noexcept = default;

    template<symbol_enum S>
    constexpr Symbol(S symbol) noexcept
        : m_font{FontInfo::instance<S>()}
        , m_unicode{std::to_underlying(symbol)}
    {}

    Symbol(SymbolTag symbol) noexcept
        : m_font{FontInfo::fromTag(symbol.font())}
        , m_unicode{symbol.unicode()}
    {}

    constexpr Symbol(const FontInfo &font, char32_t unicode) noexcept
        : m_font{font}
        , m_unicode{unicode}
    {}

    [[nodiscard]] inline bool         isNull() const { return m_font.isNull(); }
    [[nodiscard]] constexpr char32_t unicode() const { return m_unicode; }
    [[nodiscard]] inline FontInfo   fontInfo() const { return m_font; }
    [[nodiscard]] inline const char     *key() const { return m_font.key(m_font.indexOf(m_unicode)); }
    [[nodiscard]] inline QString        name() const { return m_font.name(m_font.indexOf(m_unicode)); }
    [[nodiscard]] inline QFont          font() const { return m_font.font(); }

    [[nodiscard]] QString toString() const
    {
        if (Q_UNLIKELY(isNull()))
            return {};

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

// FontIcon class // ===================================================================================================

struct ICONFONTS_EXPORT DrawIconOptions : public named_options::options<bool, int, qreal>
{
    using ColorRole = QPalette::ColorRole;
    using enum ColorRole;

    using IconMode  = QIcon::Mode;
    using enum IconMode;

    option<0, bool>  fillBox   = {};
    option<1, int>   pixelSize = {};
    option<2, qreal> pointSize = {};

    bool               applyColor = true;
    std::optional<IconMode>  mode = {};
    std::optional<ColorRole> role = {};

    bool operator==(const DrawIconOptions &) const = default;

    [[nodiscard]] QColor effectiveColor(const QColor &color, const QPalette &palette,
                                        IconMode fallbackMode = Normal) const;
};

class ICONFONTS_EXPORT FontIcon final
{
    Q_GADGET
    Q_PROPERTY(bool              isNull          READ isNull          CONSTANT FINAL)
    Q_PROPERTY(QColor            color           READ color           CONSTANT FINAL)
    Q_PROPERTY(QFont             font            READ font            CONSTANT FINAL)
    Q_PROPERTY(QString           name            READ name            CONSTANT FINAL)
    Q_PROPERTY(QString           text            READ toString        CONSTANT FINAL)
    Q_PROPERTY(IconFonts::Symbol symbol          READ symbol          CONSTANT FINAL)
    Q_PROPERTY(QTransform        transform       READ transform       CONSTANT FINAL)
    Q_PROPERTY(Transform         transformType   READ transformType   CONSTANT FINAL)

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

    [[nodiscard]] inline bool          isNull() const;
    [[nodiscard]] constexpr QColor      color() const { return m_color; }
    [[nodiscard]] QFont                  font() const { return m_symbol.font(); }
    [[nodiscard]] QString                name() const { return m_symbol.name(); }
    [[nodiscard]] Symbol               symbol() const { return m_symbol; }
    [[nodiscard]] QString            toString() const { return m_symbol.toString(); }
    [[nodiscard]] const QTransform &transform() const;
    [[nodiscard]] Transform     transformType() const;

    [[nodiscard]] QIcon toIcon() const;
    [[nodiscard]] operator QIcon() const { return toIcon(); }

    [[nodiscard]] constexpr auto fields() const noexcept { return std::tie(m_symbol, m_color, m_transform); }
    friend constexpr bool operator==(const FontIcon &l, const FontIcon &r) noexcept { return l.fields() == r.fields(); }

    [[nodiscard]] static const QTransform &transform(Transform transform) { return *instance(transform); }

    // operations

    void draw(QPainter *painter, const QRectF &rect, const QPalette &palette,
              const DrawIconOptions &options = {}, QIcon::Mode fallbackMode = QIcon::Normal) const;
    void draw(QPainter *painter, const QSizeF &size, const QPalette &palette,
              const DrawIconOptions &options = {}, QIcon::Mode fallbackMode = QIcon::Normal) const;

private:
    using TransformVariant = std::variant<std::monostate, Transform, std::shared_ptr<QTransform>>;

    FontIcon(const Symbol &symbol, const QColor &color,
             const TransformVariant &transform) noexcept
        : m_symbol{symbol}
        , m_transform{transform}
        , m_color{color}
    {}

    void drawImmediatly(QPainter *painter, const QRectF &rect, const QFont &font) const;
    void drawAlphaBlended(QPainter *painter, const QRectF &rect, const QFont &font, const QColor &color) const;

    [[nodiscard]] TransformVariant optimize(const QTransform &transform);
    [[nodiscard]] static std::shared_ptr<QTransform> instance(Transform transform);

    Symbol              m_symbol;
    TransformVariant    m_transform;
    QColor              m_color;
};

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
    friend constexpr bool operator==(const ModalFontIcon &l, const ModalFontIcon &r) noexcept
    { return l.fields() == r.fields(); }

    // operations

    void draw(QPainter *painter, const QRectF &rect, QIcon::State state,
              const QPalette &palette, const DrawIconOptions &options = {},
              QIcon::Mode fallbackMode = QIcon::Normal) const;
    void draw(QPainter *painter, const QSizeF &size, QIcon::State state,
              const QPalette &palette, const DrawIconOptions &options = {},
              QIcon::Mode fallbackMode = QIcon::Normal) const;
};

// freestanding observers // ===========================================================================================

template<symbol_enum S> [[nodiscard]] ICONFONTS_EXPORT QFont   font();
template<symbol_enum S> [[nodiscard]] ICONFONTS_EXPORT FontId  fontId();
template<symbol_enum S> [[nodiscard]] constexpr FontTag        fontTag() noexcept;
template<symbol_enum S> [[nodiscard]] ICONFONTS_EXPORT QString fontName();
template<symbol_enum S> [[nodiscard]] ICONFONTS_EXPORT QString fontFamily();
template<symbol_enum S> [[nodiscard]] ICONFONTS_EXPORT QString fontFileName();
template<symbol_enum S> [[nodiscard]] ICONFONTS_EXPORT QString licenseFileName();
template<symbol_enum S> [[nodiscard]] ICONFONTS_EXPORT QString licenseText();
template<symbol_enum S> [[nodiscard]] constexpr FontInfo::Type type() noexcept;

template<symbol_enum S>
[[nodiscard]] inline const FontInfo &fontInfo()
{
    return FontInfo::instance<S>();
}

template<symbol_enum S>
[[nodiscard]] inline QString toString(S symbol)
{
    const auto ch = QChar::fromUcs4(unicode(symbol));
    return QString::fromUtf16(ch.begin(), ch.size());
}

template<symbol_enum S>
[[nodiscard]] constexpr char32_t unicode(S symbol) noexcept
{
    return std::to_underlying<S>(symbol);
}

template<symbol_enum S>
[[nodiscard]] constexpr Symbol symbol(S symbol) noexcept
{
    return {fontInfo<S>(), unicode(symbol)};
}

// utility functions // ================================================================================================

[[nodiscard]] ICONFONTS_EXPORT QAction *createAction(const QFont &font, QStringView iconName, QObject *parent);

template<symbol_enum S>
[[nodiscard]] inline QAction *createAction(S symbol, QObject *parent)
{ return createAction(font<S>(), toString(symbol), parent); }

template<symbol_enum S>
[[nodiscard]] inline QAction *createAction(QStringView iconName, QObject *parent)
{ return createAction(font<S>(), iconName, parent); }

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

// Tag implementations // ==============================================================================================

template<auto symbol, symbol_enum S>
constexpr Tags::SymbolTag Tags::SymbolTag::make() noexcept
{
    return TaggedValue::make<SymbolTag, IconFonts::unicode(symbol)>(fontTag<S>());
}

namespace Tests { using TestTag = TaggedValue<quint32, 3, 123, 4>; }
template<> constexpr FontTag fontTag<Tests::TestSymbol>() noexcept { return FontTag::make<123>(); }

namespace Tests {

static_assert(TestTag::  minimum()          ==     3);
static_assert(TestTag::  maximum()          ==   123);
static_assert(TestTag::valueMask()          ==   127);
static_assert(TestTag::  bitmask()          == 0x7f0);
static_assert(TestTag::  make<TestTag, 3>() ==  0x30);

static_assert(std::is_base_of_v<TaggedValue<quint32, 1, 127, 24>, FontTag>);

static_assert(FontTag::  minimum() == 1);
static_assert(FontTag::  maximum() == 127);
static_assert(FontTag::valueMask() == 127);
static_assert(FontTag::  bitmask() == 0x7f'00'00'00);

static_assert(FontTag::make<1>().index() == 1);
static_assert(FontTag::make<2>().value() == 2 << 24);
static_assert(FontTag::make<3>()         == 3 << 24);

static_assert(SymbolTag::make<TestSymbol::A>().index() ==          0x41);
static_assert(SymbolTag::make<TestSymbol::B>().value() == 0x7b'00'00'42);
static_assert(SymbolTag::make<TestSymbol::C>()         == 0x7b'00'00'43);

} // namespace Tests

// FontInfo implementations // =========================================================================================

namespace Private {
template<symbol_enum S> [[nodiscard]] ICONFONTS_EXPORT bool loadResources();
} // namespace Private

template<symbol_enum S>
constexpr FontInfo::Data::Data(S)
    : type{IconFonts::type<S>()}
    , enumType{QMetaType::fromType<S>()}
    , fontTag{IconFonts::fontTag<S>()}
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

template<symbol_enum S>
inline const FontInfo &FontInfo::instance() noexcept
{
    static const auto s_instance = FontInfo{S{}};
    return s_instance;
}

inline Symbol FontInfo::symbol(int index) const
{
    return {*this, unicode(index)};
}

inline bool operator==(const FontInfo &lhs, const FontInfo &rhs) noexcept
{
    if (lhs.d == rhs.d)
        return true;
    if (lhs.d && rhs.d)
        return lhs.d->enumType == rhs.d->enumType;

    return false;
}

// FontIcon implementations // =========================================================================================

bool FontIcon::isNull() const
{
    return m_symbol.isNull()
            && !m_color.isValid()
            && std::holds_alternative<std::monostate>(m_transform);
}

} // IconFonts

#endif // ICONFONTS_ICONFONTS_H
