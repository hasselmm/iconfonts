#include "symbollistwidget.h"

namespace IconFonts::Viewer {

using namespace Qt::Literals;

int SymbolListWidget::currentSymbolIndex() const
{
    return symbolIndex(currentItem());
}

QString SymbolListWidget::currentSymbolName() const
{
    return symbolName(currentItem());
}

Symbol SymbolListWidget::currentSymbol() const
{
    return m_font.symbol(currentSymbolIndex());
}

int SymbolListWidget::findRow(const QString &iconName) const
{
    for (auto count = SymbolListWidget::count(), i = 0; i < count; ++i) {
        if (iconName == SymbolListWidget::symbolName(item(i)))
            return i;
    }

    return -1;
}

void SymbolListWidget::setFont(const FontInfo &newFont)
{
    if (std::exchange(m_font, newFont) != newFont) {
        clear();

        for (auto count = m_font.symbolCount(), i = 0; i < count; ++i) {
            const auto &codepoint = QString::number(m_font.unicode(i), 16).toUpper();
            const auto &label = u"%1 (U+%2)"_s.arg(m_font.name(i), codepoint);
            auto item = new QListWidgetItem{label, this};
            item->setIcon(FontIcon{m_font.symbol(i)}.toIcon());
            item->setData(SymbolIndexRole, i);
            applyFilter(item);
        }
    }
}

FontInfo SymbolListWidget::font() const
{
    return m_font;
}

void SymbolListWidget::setFilter(const QString &newFilter)
{
    if (std::exchange(m_filter, newFilter) != newFilter)
        for (auto count = SymbolListWidget::count(), i = 0; i < count; ++i)
            applyFilter(SymbolListWidget::item(i));
}

QString SymbolListWidget::filter() const
{
    return m_filter;
}

QString SymbolListWidget::symbolName(QListWidgetItem *item) const
{
    if (const auto index = symbolIndex(item); index >= 0)
        return m_font.name(index);

    return {};
}

int SymbolListWidget::symbolIndex(QListWidgetItem *item)
{
    if (item)
        return item->data(SymbolIndexRole).toInt();

    return -1;
}

void SymbolListWidget::applyFilter(QListWidgetItem *item) const
{
    if (item) {
        const auto &text = item->text();
        const auto matched = text.contains(m_filter, Qt::CaseInsensitive);
        item->setHidden(!matched);
    }
}

} // namespace IconFonts::Viewer
