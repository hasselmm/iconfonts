#ifndef ICONFONTSVIEWER_SYMBOLLISTWIDGET_H
#define ICONFONTSVIEWER_SYMBOLLISTWIDGET_H

#include <iconfonts/iconfonts.h>

#include <QListWidget>

namespace IconFonts::Viewer {

class SymbolListWidget : public QListWidget
{
    Q_OBJECT

public:
    enum Role {
        SymbolIndexRole = Qt::UserRole,
    };

    using QListWidget::QListWidget;

    [[nodiscard]] int currentSymbolIndex() const;
    [[nodiscard]] QString currentSymbolName() const;
    [[nodiscard]] Symbol currentSymbol() const;

    [[nodiscard]] int findRow(const QString &iconName) const;

    void setFont(const FontInfo &newFont);
    [[nodiscard]] FontInfo font() const;

    void setFilter(const QString &newFilter);
    [[nodiscard]] QString filter() const;

private:
    [[nodiscard]] QString symbolName(QListWidgetItem *item) const;
    [[nodiscard]] static int symbolIndex(QListWidgetItem *item);

    void applyFilter(QListWidgetItem *item) const;

    FontInfo m_font;
    QString m_filter;
};

} // namespace IconFonts::Viewer

#endif // ICONFONTSVIEWER_SYMBOLLISTWIDGET_H
