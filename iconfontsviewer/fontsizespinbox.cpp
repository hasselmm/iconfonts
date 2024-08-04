#include "fontsizespinbox.h"

#include <QActionGroup>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>

namespace IconFonts::Viewer {

FontSizeSpinBox::FontSizeSpinBox(QWidget *parent)
    : QSpinBox{parent}
{
    const auto actionGroup = new QActionGroup{this};

    connect(actionGroup, &QActionGroup::triggered,
            this, [this](QAction *action) {
        setMode(qvariant_cast<Mode>(action->data()));
    });

    actionGroup->addAction(addLineEditAction(tr("fill"), Mode::Fill));
    actionGroup->addAction(addLineEditAction(tr("px"), Mode::Pixels));
    actionGroup->addAction(addLineEditAction(tr("pt"), Mode::Points))->setChecked(true);

    Q_ASSERT(mode() != Mode::Points);
    setMode(Mode::Points);
    setRange(7, 200);
    setValue(16);
}

void FontSizeSpinBox::setMode(Mode newMode)
{
    if (std::exchange(m_mode, newMode) != newMode)
        emit modeChanged(newMode);
}

FontSizeSpinBox::Mode FontSizeSpinBox::mode() const
{
    return m_mode;
}

QAction *FontSizeSpinBox::addLineEditAction(const QString &text, Mode mode)
{
    const auto action = new QAction{text, this};

    lineEdit()->addAction(action, QLineEdit::LeadingPosition);
    action->setData(QVariant::fromValue(mode));
    action->setCheckable(true);

    const auto updateIcon = [this, action](bool checked) {
        const auto &palette   = lineEdit()->palette();
        const auto foreground = palette.color(QPalette::HighlightedText);
        const auto background = palette.color(QPalette::Highlight);

        if (checked)
            action->setIcon(textIcon(action->text(), foreground, background));
        else
            action->setIcon(textIcon(action->text(), background, foreground));
    };

    connect(action, &QAction::toggled, action, updateIcon);
    updateIcon(action->isChecked());

    return action;
}

QIcon FontSizeSpinBox::textIcon(const QString &text, const QColor &foreground, const QColor &background)
{
    const auto size = style()->pixelMetric(QStyle::PM_LineEditIconSize, nullptr, lineEdit()) * 2;
    auto pixmap = QPixmap{size, size};
    pixmap.fill(background);

    auto iconFont = font();
    iconFont.setPixelSize(size * 5/9);

    auto painter = QPainter{&pixmap};
    painter.setFont(iconFont);
    painter.setPen(foreground);
    painter.drawText(0, 0, size, size, Qt::AlignCenter, text);

    return pixmap;
}

} // namespace IconFonts::Viewer
