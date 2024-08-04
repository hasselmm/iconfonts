#include "fontlistwidget.h"

namespace IconFonts::Viewer {

using namespace Qt::StringLiterals;

FontListWidget::FontListWidget(QWidget *parent)
    : QListWidget{parent}
{
    for (const auto &fontInfo : FontInfo::knownFonts())
        add(fontInfo);
}

QListWidgetItem *FontListWidget::add(const FontInfo &fontInfo)
{
    const auto &fullTypeName = QString::fromLatin1(fontInfo.enumType().name());
    const auto &typeName = fullTypeName.section(u"::"_s, 2, -2);

    const auto item = new QListWidgetItem{fontInfo.fontName(), this};
    item->setData(FontInfoRole, QVariant::fromValue(fontInfo));

    item->setToolTip(QList{//tr("Font: %1").arg(fontInfo.font().family()),
                           tr("C++ Namespace: %1").arg(typeName),
                           tr("Symbol count: %1").arg(fontInfo.symbolCount())}.
                     join(u'\n'));

    return item;
}

FontInfo FontListWidget::currentFontInfo() const
{
    if (const auto item = currentItem())
        return qvariant_cast<FontInfo>(item->data(FontInfoRole));

    return {};
}

void FontListWidget::setFilter(const QString &newFilter)
{
    if (std::exchange(m_filter, newFilter) != newFilter)
        for (auto count = FontListWidget::count(), i = 0; i < count; ++i)
            applyFilter(FontListWidget::item(i));
}

QString FontListWidget::filter() const
{
    return m_filter;
}

void FontListWidget::applyFilter(QListWidgetItem *item) const
{
    if (item) {
        const auto &text = item->text();
        const auto matched = text.contains(m_filter, Qt::CaseInsensitive);
        item->setHidden(!matched);
    }
}

} // namespace IconFonts::Viewer
