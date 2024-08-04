#ifndef ICONFONTSVIEWER_FONTLISTWIDGET_H
#define ICONFONTSVIEWER_FONTLISTWIDGET_H

#include <iconfonts/iconfonts.h>

#include <QListWidget>

namespace IconFonts::Viewer {

class FontListWidget : public QListWidget
{
    Q_OBJECT

public:
    enum Roles {
        FontInfoRole = Qt::UserRole,
    };

    explicit FontListWidget(QWidget *parent = nullptr);

    [[nodiscard]] FontInfo currentFontInfo() const;

    void setFilter(const QString &newFilter);
    [[nodiscard]] QString filter() const;

private:
    QListWidgetItem *add(const FontInfo &fontInfo);
    void applyFilter(QListWidgetItem *item) const;

    QString m_filter;
};

} // namespace IconFonts::Viewer

#endif // ICONFONTSVIEWER_FONTLISTWIDGET_H
