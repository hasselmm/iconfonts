#ifndef QUICKICONFONTS_H
#define QUICKICONFONTS_H

#include <iconfonts/iconfonts.h>

#include <QMatrix4x4>
#include <QQuickItem>

namespace QuickIconFonts {

class QUICKICONFONTS_EXPORT Symbol
{
    Q_GADGET
    Q_PROPERTY(bool  isNull READ isNull   CONSTANT FINAL)
    Q_PROPERTY(QFont   font READ font     CONSTANT FINAL)
    Q_PROPERTY(QString name READ name     CONSTANT FINAL)
    Q_PROPERTY(QString text READ toString CONSTANT FINAL)
    Q_PROPERTY(uint unicode READ unicode  CONSTANT FINAL)

    QML_CONSTRUCTIBLE_VALUE
    QML_VALUE_TYPE(symbol)

public:
    constexpr Symbol() noexcept = default;
    constexpr Symbol(const IconFonts::Symbol &symbol) noexcept : m_symbol{symbol} {}
    constexpr Symbol(IconFonts::Symbol &&symbol) noexcept : m_symbol{std::move(symbol)} {}

    Q_INVOKABLE Symbol(int taggedSymbol)
        : m_symbol{IconFonts::SymbolTag::fromValue(taggedSymbol)}
    {}

    [[nodiscard]] bool                  isNull() const { return m_symbol.isNull();   }
    [[nodiscard]] QFont                   font() const { return m_symbol.font();     }
    [[nodiscard]] QString                 name() const { return m_symbol.name();     }
    [[nodiscard]] Q_INVOKABLE QString toString() const { return m_symbol.toString(); }
    [[nodiscard]] char32_t             unicode() const { return m_symbol.unicode();  }

    constexpr operator IconFonts::Symbol() const noexcept { return m_symbol; }

private:
    IconFonts::Symbol m_symbol;
};

class QUICKICONFONTS_EXPORT FontIcon
{
    Q_GADGET
    Q_PROPERTY(bool                                  isNull READ isNull          CONSTANT FINAL)
    Q_PROPERTY(QColor                                 color READ color           CONSTANT FINAL)
    Q_PROPERTY(bool                                hasColor READ hasColor        CONSTANT FINAL)
    Q_PROPERTY(QFont                                   font READ font            CONSTANT FINAL)
    Q_PROPERTY(QString                                 name READ name            CONSTANT FINAL)
    Q_PROPERTY(QString                                 text READ toString        CONSTANT FINAL)
    Q_PROPERTY(QuickIconFonts::Symbol                symbol READ symbol          CONSTANT FINAL)
    Q_PROPERTY(QMatrix4x4                         transform READ transform       CONSTANT FINAL)
    Q_PROPERTY(IconFonts::FontIcon::Transform transformType READ transformType   CONSTANT FINAL)

    QML_CONSTRUCTIBLE_VALUE
    QML_VALUE_TYPE(fonticon)

public:
    constexpr FontIcon() noexcept = default;

    Q_INVOKABLE FontIcon(const Symbol &symbol) : m_icon{symbol} {}
    Q_INVOKABLE FontIcon(int taggedUnicode) : m_icon{Symbol{taggedUnicode}} {}

    [[nodiscard]] bool                                  isNull() const { return m_icon.isNull();          }
    [[nodiscard]] QColor                                 color() const { return m_icon.color();           }
    [[nodiscard]] bool                                hasColor() const { return m_icon.hasColor();        }
    [[nodiscard]] QFont                                   font() const { return m_icon.font();            }
    [[nodiscard]] QString                                 name() const { return m_icon.name();            }
    [[nodiscard]] QuickIconFonts::Symbol                symbol() const { return m_icon.symbol();          }
    [[nodiscard]] QMatrix4x4                         transform() const { return m_icon.transform();       }
    [[nodiscard]] IconFonts::FontIcon::Transform transformType() const { return m_icon.transformType();   }
    [[nodiscard]] Q_INVOKABLE QString                 toString() const { return m_icon.toString();        }

    operator IconFonts::FontIcon() const noexcept { return m_icon; }

private:
    IconFonts::FontIcon m_icon;
};

class QUICKICONFONTS_EXPORT IconMode : public QObject
{
    Q_OBJECT

    QML_ELEMENT
    QML_UNCREATABLE("This type only provides the IconMode enumeration")

public:
    enum Value {
        Normal      = QIcon::Normal,
        Disabled    = QIcon::Disabled,
        Active      = QIcon::Active,
        Selected    = QIcon::Selected,
    };

    Q_ENUM(Value)

    IconMode() = delete;

    static constexpr Value fromQIcon(QIcon::Mode mode)
    {
        switch (mode) {
        case QIcon::Normal:
            return IconMode::Normal;
        case QIcon::Disabled:
            return IconMode::Disabled;
        case QIcon::Active:
            return IconMode::Active;
        case QIcon::Selected:
            return IconMode::Selected;
        }

        Q_UNREACHABLE_RETURN(static_cast<Value>(-1));
    }

    static constexpr QIcon::Mode toQIcon(Value mode)
    {
        switch (mode) {
        case IconMode::Normal:
            return QIcon::Normal;
        case IconMode::Disabled:
            return QIcon::Disabled;
        case IconMode::Active:
            return QIcon::Active;
        case IconMode::Selected:
            return QIcon::Selected;
        }

        Q_UNREACHABLE_RETURN(static_cast<QIcon::Mode>(-1));
    }
};

static_assert(IconMode::fromQIcon(QIcon::Normal)  == IconMode::Normal);
static_assert(IconMode::toQIcon(IconMode::Normal) ==    QIcon::Normal);

class QUICKICONFONTS_EXPORT DrawIconOptions
{
    Q_GADGET
    Q_PROPERTY(bool                    hasPointSize READ hasPointSize CONSTANT FINAL)
    Q_PROPERTY(qreal                      pointSize READ pointSize    CONSTANT FINAL)
    Q_PROPERTY(bool                    hasPixelSize READ hasPixelSize CONSTANT FINAL)
    Q_PROPERTY(int                        pixelSize READ pixelSize    CONSTANT FINAL)
    Q_PROPERTY(bool                      applyColor READ applyColor   CONSTANT FINAL)
    Q_PROPERTY(bool                         hasMode READ hasMode      CONSTANT FINAL)
    Q_PROPERTY(QuickIconFonts::IconMode::Value mode READ mode         CONSTANT FINAL)

    QML_VALUE_TYPE(drawIconOptions)

public:
    DrawIconOptions() = default;
    DrawIconOptions(const IconFonts::DrawIconOptions &options) : m_options{options} {}

    [[nodiscard]] bool           hasPointSize() const { return m_options.pointSize.has_value(); }
    [[nodiscard]] qreal             pointSize() const { return m_options.pointSize.value_or(0); }
    [[nodiscard]] bool           hasPixelSize() const { return m_options.pixelSize.has_value(); }
    [[nodiscard]] int               pixelSize() const { return m_options.pixelSize.value_or(0); }
    [[nodiscard]] bool             applyColor() const { return m_options.applyColor;            }
    [[nodiscard]] bool                hasMode() const { return m_options.mode.has_value();      }
    [[nodiscard]] inline IconMode::Value mode() const;

    operator IconFonts::DrawIconOptions() const noexcept { return m_options; }

    [[nodiscard]] Q_INVOKABLE QColor effectiveColor(const QQuickItem *item, const QColor &iconColor) const;
    [[nodiscard]] Q_INVOKABLE QColor effectiveColor(const QQuickItem *item, const QColor &iconColor, bool active) const;

private:
    IconFonts::DrawIconOptions m_options;
};

inline IconMode::Value DrawIconOptions::mode() const
{
    return IconMode::fromQIcon(m_options.mode.value_or(QIcon::Normal));
}

} // namespace QuickIconFonts

#endif // QUICKICONFONTS_H
