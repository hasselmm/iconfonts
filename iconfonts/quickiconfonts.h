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

} // namespace QuickIconFonts

#endif // QUICKICONFONTS_H
