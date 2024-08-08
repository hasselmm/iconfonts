#include "mainwindow.h"

#include "fontlistwidget.h"
#include "fontsizespinbox.h"
#include "iconpreview.h"
#include "symbollistwidget.h"

#include <iconfonts/iconfontsconfig.h>

#ifdef ICONFONTS_ENABLE_MATERIALSYMBOLS_ROUNDED
#include <iconfonts/materialsymbolsrounded.h>
#else
#error Font "Material Symbols Rounded" required
#endif

#include <QActionGroup>
#include <QColorDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMetaEnum>
#include <QToolBar>

#ifdef ICONFONTS_ENABLE_QUICKWIDGET

#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickWidget>

#else

class QQuickWidget : public QLabel
{
public:
    explicit QQuickWidget(QWidget *parent = nullptr)
        : QLabel{tr("not available"), parent}
    {
        setAlignment(Qt::AlignCenter);
        setEnabled(false);
    }
};

#endif // ICONFONTS_ENABLE_QUICKWIDGET

namespace IconFonts::Viewer {

using namespace Qt::Literals;
using enum MaterialSymbolsRounded;

namespace {

template<typename T> requires std::is_enum_v<T>
[[nodiscard]] FontIcon icon(T) { return {}; }

template<typename T> requires std::is_enum_v<T>
[[nodiscard]] QString enumKey(T);

template<>
[[nodiscard]] FontIcon icon(FontIcon::Transform transform)
{
    using enum FontIcon::Transform;
    using namespace QColorConstants;

    switch(transform) {
    case None:
        return CropOriginal | [] {
            auto t = QTransform{};
            t.rotate(22.5);
            return t;
        }();

    case HorizontalFlip:
        return Flip;

    case VerticalFlip:
        return Flip | Rotate90;

    case Rotate90:
        return RotateRight | Svg::crimson;

    case Rotate180:
        return RotateRight | Rotate90 | Svg::lime;

    case Rotate270:
        return RotateRight | Svg::violet | Rotate180;

    case Matrix:
        return Calculate;
    }

    return {};
}

QString enumKey(QIcon::Mode mode)
{
    switch (mode) {
    case QIcon::Normal:     return u"Normal"_s;
    case QIcon::Disabled:   return u"Disabled"_s;
    case QIcon::Active:     return u"Active"_s;
    case QIcon::Selected:   return u"Selected"_s;
    }
    return {};
}

FontIcon icon(QIcon::Mode mode)
{
    switch (mode) {
    case QIcon::Normal:     return Photo;
    case QIcon::Disabled:   return ImageNotSupported;
    case QIcon::Active:     return AddPhotoAlternate;
    case QIcon::Selected:   return PhotoSizeSelectLarge;
    }
    return {};
}

template<typename T>
requires std::is_enum_v<T>
constexpr bool hasMetaEnum_v = QtPrivate::IsQEnumHelper<T>::Value;

template<typename T> requires std::is_enum_v<T>
constexpr std::monostate enumValues;

template<> constexpr std::array enumValues<QIcon::Mode> = {QIcon::Normal, QIcon::Disabled, QIcon::Active, QIcon::Selected};

static_assert(hasMetaEnum_v<FontIcon::Transform>);
static_assert(!hasMetaEnum_v<QIcon::Mode>);

template<typename T>
requires std::is_enum_v<T>
struct EnumTrait
{
    [[nodiscard]] static int keyCount()
    {
        if constexpr (hasMetaEnum_v<T>) {
            return QMetaEnum::fromType<T>().keyCount();
        } else {
            return enumValues<T>.size();
        }
    }

    [[nodiscard]] static T value(int index)
    {
        if constexpr (hasMetaEnum_v<T>) {
            return static_cast<T>(QMetaEnum::fromType<T>().value(index));
        } else {
            if (static_cast<std::size_t>(index) < enumValues<T>.size())
                return enumValues<T>[index];

            return {};
        }
    }

    [[nodiscard]] static QString key(int index)
    {
        if constexpr (hasMetaEnum_v<T>) {
            return QString::fromLatin1(QMetaEnum::fromType<T>().key(index));
        } else {
            return enumKey(value(index));
        }
    }
};

template<typename T> requires std::is_enum_v<T>
QActionGroup *createActionGroup(QObject *parent)
{
    const auto actionGroup = new QActionGroup{parent};
    actionGroup->setExclusive(true);

    for (auto count = EnumTrait<T>::keyCount(), i = 0; i < count; ++i) {
        const auto value = EnumTrait<T>::value(i);
        const auto action = actionGroup->addAction(icon(value), EnumTrait<T>::key(i));
        action->setData(QVariant::fromValue(value));
        action->setCheckable(true);
    }

    actionGroup->actions().constFirst()->setChecked(true);

    return actionGroup;
}

struct SizePolicy
{
    std::optional<QSizePolicy::Policy> horizontalPolicy = {};
    std::optional<QSizePolicy::Policy> verticalPolicy = {};

    std::optional<int> horizontalStretch = {};
    std::optional<int> verticalStretch = {};
};

void setSizePolicy(QWidget *widget, const SizePolicy &newPolicy)
{
    auto sizePolicy = widget->sizePolicy();

    if (newPolicy.horizontalPolicy.has_value())
        sizePolicy.setHorizontalPolicy(newPolicy.horizontalPolicy.value());
    if (newPolicy.horizontalStretch.has_value())
        sizePolicy.setHorizontalStretch(newPolicy.horizontalStretch.value());

    if (newPolicy.verticalPolicy.has_value())
        sizePolicy.setVerticalPolicy(newPolicy.verticalPolicy.value());
    if (newPolicy.verticalStretch.has_value())
        sizePolicy.setVerticalStretch(newPolicy.verticalStretch.value());

    widget->setSizePolicy(sizePolicy);
}

QLabel *createCaption(QWidget *parent)
{
    const auto label = new QLabel{parent};
    label->setStyleSheet(u"font-weight: bold"_s);
    label->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    setSizePolicy(label, {.horizontalPolicy = QSizePolicy::Expanding, .horizontalStretch = 1});
    return label;
};

QFont makeFont(const FontIcon &icon, const DrawIconOptions &options, const QSize &fillSize)
{
    auto font = icon.symbol().font();

    if (options.pixelSize)
        font.setPixelSize(options.pixelSize.value());
    else if (options.pointSize)
        font.setPointSize(options.pointSize.value());
    else
        font.setPixelSize(std::min(fillSize.width(), fillSize.height()));

    return font;
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QWidget{parent}
    , m_fontList{new FontListWidget{this}}
    , m_symbolList{new SymbolListWidget{this}}
    , m_fontSize{new FontSizeSpinBox{this}}
    , m_colorAction{new QAction{tr("Change Color"), this}}
    , m_resetColorAction{new QAction{FontIcon{FormatColorReset}, tr("Reset Color"), this}}
    , m_graphicalPreview{new IconPreview{this}}
    , m_textualPreview{new QLabel{this}}
    , m_quickPreview{new QQuickWidget{this}}
    , m_leftPreviewGroup{createActionGroup<PreviewType>(this)}
    , m_rightPreviewGroup{createActionGroup<PreviewType>(this)}
{
    const auto layout = new QHBoxLayout{this};

    layout->addLayout(createFontListLayout());
    layout->addLayout(createSymbolListLayout());
    layout->addLayout(createPreviewLayout(), 1);

    setIcon(Symbol{});
    updateTextualPreview();
}

QLayout *MainWindow::createFontListLayout()
{
    m_fontList->setSortingEnabled(true);
    m_fontList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_fontList->setFrameShape(FontListWidget::NoFrame);

    const auto fontFilter = new QLineEdit{this};

    fontFilter->setPlaceholderText(tr("Filter fonts..."));

    const auto layout = new QVBoxLayout;

    layout->addWidget(m_fontList, 1);
    layout->addWidget(fontFilter);

    connect(fontFilter, &QLineEdit::textChanged,
            m_fontList, &FontListWidget::setFilter);

    connect(m_fontList, &QListWidget::itemSelectionChanged,
            this, &MainWindow::onSelectedFontChanged);

    return layout;
}

QLayout *MainWindow::createSymbolListLayout()
{
    m_symbolList->setSortingEnabled(true);
    m_symbolList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_symbolList->setFrameShape(FontListWidget::NoFrame);

    const auto symbolFilter = new QLineEdit{this};

    symbolFilter->setPlaceholderText(tr("Filter symbols..."));

    const auto layout = new QVBoxLayout;

    layout->addWidget(m_symbolList, 1);
    layout->addWidget(symbolFilter);

    connect(symbolFilter, &QLineEdit::textChanged,
            m_symbolList, &SymbolListWidget::setFilter);

    connect(m_symbolList, &QListWidget::itemSelectionChanged, this, [this] {
        setIcon(icon() | m_symbolList->currentSymbol());
    });

    return layout;
}

QLayout *MainWindow::createPreviewLayout()
{
    m_textualPreview->setAlignment(Qt::AlignCenter);
    m_textualPreview->setMinimumSize(300, 400);
    m_textualPreview->hide();

    m_graphicalPreview->setMinimumSize(300, 400);
    m_graphicalPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_graphicalPreview->hide();

    m_quickPreview->setMinimumSize(300, 400);
    m_quickPreview->setClearColor(Qt::transparent);
    m_quickPreview->setAttribute(Qt::WA_AlwaysStackOnTop);
    m_quickPreview->setAttribute(Qt::WA_TranslucentBackground);
    m_quickPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_quickPreview->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickPreview->hide();

    auto component = QQmlComponent{m_quickPreview->engine()};
    component.setData(R"(import QtQuick; Text { renderTypeQuality: Text.VeryHighRenderTypeQuality })", QUrl{});
    m_quickPreview->setContent(component.url(), &component, component.create(m_quickPreview->rootContext()));
    m_quickPreview->hide();

    Viewer::setSizePolicy(m_textualPreview, {
                              .horizontalPolicy = QSizePolicy::Expanding,
                              .verticalPolicy = QSizePolicy::Expanding,
                              .horizontalStretch = 1,
                          });

    Viewer::setSizePolicy(m_graphicalPreview, {
                              .horizontalPolicy = QSizePolicy::Expanding,
                              .verticalPolicy = QSizePolicy::Expanding,
                              .horizontalStretch = 1,
                          });

    Viewer::setSizePolicy(m_quickPreview, {
                              .horizontalPolicy = QSizePolicy::Expanding,
                              .verticalPolicy = QSizePolicy::Expanding,
                              .horizontalStretch = 1,
                          });

    m_fontSize->setMinimumWidth(135);
    m_fontSize->setValue(200);

    m_colorAction->setIcon(Colors | m_graphicalPreview->icon().color());

    const auto ignoreColorAction = new QAction{tr("Ignore Color"), this};
    ignoreColorAction->setIcon(InvertColorsOff ^ InvertColors);
    ignoreColorAction->setCheckable(true);

    const auto  iconModeGroup = createActionGroup<QIcon::Mode>(this);
    const auto transformGroup = createActionGroup<FontIcon::Transform>(this);

    const auto toolBar = new QToolBar{this};

    toolBar->setStyleSheet(u"QToolBar::separator { background: transparent; width: 12 }"_s);
    Viewer::setSizePolicy(toolBar, {.horizontalPolicy = QSizePolicy::Expanding, .horizontalStretch = 2});

    toolBar->addWidget(m_fontSize);
    toolBar->addSeparator();
    toolBar->addAction(m_colorAction);
    toolBar->addAction(ignoreColorAction);
    toolBar->addAction(m_resetColorAction);
    toolBar->addSeparator();
    toolBar->addActions(iconModeGroup->actions());
    toolBar->addSeparator();
    toolBar->addActions(transformGroup->actions());

    const auto leftPreviewSelector = new QToolBar{this};

    leftPreviewSelector->addActions(m_leftPreviewGroup->actions());
    Viewer::setSizePolicy(leftPreviewSelector, {.horizontalPolicy = QSizePolicy::Fixed, .horizontalStretch = 1});

    const auto rightPreviewSelector = new QToolBar{this};

    rightPreviewSelector->addActions(m_rightPreviewGroup->actions());
    Viewer::setSizePolicy(rightPreviewSelector, {.horizontalPolicy = QSizePolicy::Fixed, .horizontalStretch = 1});

    const auto layout = new QGridLayout;

    layout->addWidget(toolBar, ToolBar, Left, 1, 2);
    layout->addWidget(m_textualPreview, Preview, Left);
    layout->addWidget(m_graphicalPreview, Preview, Right);
    layout->addWidget(createCaption(this), Caption, Left, Qt::AlignCenter);
    layout->addWidget(createCaption(this), Caption, Right, Qt::AlignCenter);
    layout->addWidget(leftPreviewSelector, Selector, Left, Qt::AlignCenter);
    layout->addWidget(rightPreviewSelector, Selector, Right, Qt::AlignCenter);

    connect(m_colorAction, &QAction::triggered,
            this, &MainWindow::onColorActionTriggered);
    connect(ignoreColorAction, &QAction::triggered,
            this, &MainWindow::setIgnoreColor);
    connect(m_resetColorAction, &QAction::triggered,
            this, &MainWindow::onResetColor);

    connect(m_fontSize, &FontSizeSpinBox::modeChanged,
            this, &MainWindow::setFontSizeMode);

    connect(m_fontSize, &FontSizeSpinBox::valueChanged,
            this, &MainWindow::onSetFontSize);

    connect(iconModeGroup, &QActionGroup::triggered,
            this, &MainWindow::onIconModeActionTriggered);

    connect(transformGroup, &QActionGroup::triggered,
            this, &MainWindow::onTransformActionTriggered);

    connect(m_graphicalPreview, &IconPreview::iconChanged,
            this, &MainWindow::updateTextualPreview);
    connect(m_graphicalPreview, &IconPreview::optionsChanged,
            this, &MainWindow::updateTextualPreview);

    setupPreviewSelector(layout, Left);
    setupPreviewSelector(layout, Right);
    setFontSizeMode(m_fontSize->mode());

    return layout;
}

void MainWindow::setupPreviewSelector(QGridLayout *layout,PreviewColumn column)
{
    const auto self = (column == Left ? m_leftPreviewGroup : m_rightPreviewGroup);
    const auto other = (column == Right ? m_leftPreviewGroup : m_rightPreviewGroup);

    connect(self, &QActionGroup::triggered, self, [this, self, other, layout, column](QAction *action) {
        const auto &otherActionList = other->actions();
        const auto &ownActionList = self->actions();

        for (const auto otherAction : otherActionList)
            otherAction->setEnabled(true);

        auto index = ownActionList.indexOf(action);

        if (const auto otherAction = otherActionList.value(index)) {
            if (otherAction->isChecked())
                otherActionList[(index + 1) % otherActionList.count()]->trigger();
        }

        const auto captionItem = layout->itemAtPosition(Caption, column);
        Q_ASSERT(captionItem != nullptr);

        const auto caption = dynamic_cast<QLabel *>(captionItem->widget());
        Q_ASSERT(caption != nullptr);

        auto previewWidget = static_cast<QWidget *>(nullptr);

        switch(qvariant_cast<PreviewType>(action->data())) {
        case PreviewType::Painter:
            caption->setText(tr("Icon preview via QPainter from QtGui"));
            previewWidget = m_graphicalPreview;
            break;

        case PreviewType::Widget:
            caption->setText(tr("Icon preview via QLabel from QtWidgets"));
            previewWidget = m_textualPreview;
            break;

        case PreviewType::Quick:
            caption->setText(tr("Icon preview via Text item from QtQuick"));
            previewWidget = m_quickPreview;
            break;
        }

        Q_ASSERT(previewWidget != nullptr);

        for (auto count = layout->count(), i = 0; i < count; ++i) {
            auto itemRow = -1, itemColumn = -1, dummy = -1;
            layout->getItemPosition(i, &itemRow, &itemColumn, &dummy, &dummy);

            if (itemRow != Preview || itemColumn != column)
                continue;

            if (const auto widget = layout->itemAt(i)->widget())
                widget->hide();
        }

        if (const auto index = layout->indexOf(previewWidget)) {
            if (const auto item = layout->takeAt(index)) {
                Q_ASSERT(item->widget() == previewWidget);
                delete item;
            }
        }

        layout->addWidget(previewWidget, Preview, column);
        previewWidget->show();
    });

    self->actions().constFirst()->trigger();
}

void MainWindow::onSelectedFontChanged()
{
    const auto &selectedSymbolName = m_symbolList->currentSymbolName();
    m_symbolList->setFont(m_fontList->currentFontInfo());

    // restore previously selected symbol, if possible
    const auto matchingRow = m_symbolList->findRow(selectedSymbolName);
    m_symbolList->setCurrentRow(std::max(0, matchingRow));

    setIcon(icon() | m_symbolList->currentSymbol());
}

void MainWindow::onTransformActionTriggered(QAction *action)
{
    setIcon(icon() | qvariant_cast<FontIcon::Transform>(action->data()));
}

void MainWindow::onIconModeActionTriggered(QAction *action)
{
    auto options = MainWindow::options();
    options.mode = qvariant_cast<QIcon::Mode>(action->data());
    setOptions(options);
}

void MainWindow::onColorActionTriggered()
{
    const auto dialog = new QColorDialog{this};

    dialog->setCurrentColor(icon().color());

    connect(dialog, &QColorDialog::accepted, this, [this, dialog] {
        setIcon(icon() | dialog->currentColor());
    });

    connect(dialog, &QColorDialog::finished,
            dialog, &QColorDialog::deleteLater);

    dialog->show();
}

void MainWindow::setFontSizeMode(FontSizeSpinBox::Mode newMode)
{
    auto options = MainWindow::options();

    switch (newMode) {
    case FontSizeSpinBox::Mode::Fill:
        options.fillBox = true;
        setOptions(options);
        break;

    case FontSizeSpinBox::Mode::Pixels:
        options.pixelSize = m_fontSize->value();
        setOptions(options);
        break;

    case FontSizeSpinBox::Mode::Points:
        options.pointSize = m_fontSize->value();
        setOptions(options);
        break;
    }
}

void MainWindow::onSetFontSize(int newFontSize)
{
    auto options = MainWindow::options();

    if (options.pixelSize) {
        options.pixelSize = newFontSize;
        setOptions(options);
    } else if (options.pointSize) {
        options.pointSize = newFontSize;
        setOptions(options);
    }
}

void MainWindow::setIgnoreColor(bool newIgnoreColor)
{
    auto options = MainWindow::options();
    options.applyColor = !newIgnoreColor;
    setOptions(options);

    m_colorAction->setEnabled(options.applyColor);
}

void MainWindow::onResetColor()
{
    setIcon(icon() | QColor{});
}

void MainWindow::setIcon(const FontIcon &newIcon)
{
    const auto &newColor = newIcon.color();

    if (newColor.isValid())
        m_colorAction->setIcon(Colors | newColor);
    else
        m_colorAction->setIcon(FontIcon{FormatColorReset});

    m_resetColorAction->setEnabled(newColor.isValid());
    m_graphicalPreview->setIcon(newIcon);
}

void MainWindow::updateTextualPreview()
{
    const auto &newIcon = icon();
    const auto &newFont = makeFont(newIcon, m_graphicalPreview->options(), m_textualPreview->size());
    const auto &options = m_graphicalPreview->options();
    const auto quickText = m_quickPreview->rootObject();

    if (newIcon.symbol().isNull()) {
        m_textualPreview->setFont({});
        m_textualPreview->setText(tr("no icon selected"));
        m_textualPreview->setEnabled(false);

        quickText->setProperty("font", QFont{});
        quickText->setProperty("text", tr("no icon selected"));
        m_quickPreview->setEnabled(false);
    } else {
        m_textualPreview->setFont(newFont);
        m_textualPreview->setText(newIcon.symbol());
        m_textualPreview->setEnabled(options.mode != QIcon::Disabled);

        quickText->setProperty("font", newFont);
        quickText->setProperty("text", newIcon.symbol().toString());
        m_quickPreview->setEnabled(options.mode != QIcon::Disabled);
    }

    if (options.applyColor
            && !newIcon.symbol().isNull()
            && m_textualPreview->isEnabled()) {
        const auto &newColorName = newIcon.color().name();
        const auto &styleSheet = u"color: %1"_s.arg(newColorName);
        m_textualPreview->setStyleSheet(styleSheet);
        quickText->setProperty("color", newIcon.color());
    } else {
        m_textualPreview->setStyleSheet({});

        const auto &palette = m_quickPreview->palette();
        const auto role = m_quickPreview->foregroundRole();
        quickText->setProperty("color", palette.color(role));
    }
}

FontIcon MainWindow::icon() const
{
    return m_graphicalPreview->icon();
}

void MainWindow::setOptions(const DrawIconOptions &options)
{
    m_graphicalPreview->setOptions(options);
}

DrawIconOptions MainWindow::options() const
{
    return m_graphicalPreview->options();
}

} // namespace IconFonts::Viewer
