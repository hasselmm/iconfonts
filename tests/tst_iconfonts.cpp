#include "iconfonts/iconfonts.h"
#include "iconfonts/iconfontsconfig.h"
#include "iconfonts/segoefluenticons.h"
#include "iconfonts/segoemdl2assets.h"

#include <QFontMetrics>
#include <QTest>

using namespace Qt::StringLiterals;

namespace IconFonts::Tests {
namespace {

QByteArray removeByteOrderMarker(const QByteArray &text)
{
    static const auto s_bomUtf8 = "\xEF\xBB\xBF"_ba;

    if (text.startsWith(s_bomUtf8))
        return text.mid(s_bomUtf8.length());

    return text;
}

class IconFontsTest : public QObject
{
    Q_OBJECT

private:
    enum class DrawIconOptional
    {
        None,
        FillBox,
        PixelSize,
        PointSize,
        Mode,
    };

    using enum SegoeFluentIcons;

    void collectFontInfoData()
    {
        QTest::addColumn<FontInfo>("font");

        for (const auto &font : FontInfo::knownFonts())
            QTest::newRow(font.enumType().name()) << font;
    }

    void ignoreFontLoadingMessage(const FontInfo &font)
    {
        if (font.type() == FontInfo::Type::Application
                && !std::exchange(m_fontSeen[font.enumType()], true)) {
            const auto typeName = QString::fromLatin1(font.enumType().name());
            auto message = typeName + uR"( is available via font id \d+)"_s;
            QTest::ignoreMessage(QtDebugMsg, QRegularExpression{message});
        }
    }

    QHash<QMetaType, bool> m_fontSeen;

private slots:
    void testSymbolConstructors()
    {
        const auto null1 = Symbol{};
        const auto null2 = Symbol{};

        QVERIFY(null1.isNull());
        QCOMPARE(null1.unicode(), 0);
        QVERIFY(null1.fontInfo().isNull());

        QVERIFY(null2.isNull());
        QCOMPARE(null2.unicode(), 0);
        QVERIFY(null2.fontInfo().isNull());

        QCOMPARE(null1, null2);

        const auto addFriend1 = Symbol{AddFriend};
        const auto addFriend2 = Symbol{AddFriend};

        QVERIFY(!addFriend1.isNull());
        QCOMPARE(addFriend1.unicode(), unicode(AddFriend));
        QCOMPARE(addFriend1.fontInfo(), FontInfo::instance<SegoeFluentIcons>());

        QVERIFY(!addFriend2.isNull());
        QCOMPARE(addFriend2.unicode(), unicode(AddFriend));
        QCOMPARE(addFriend2.fontInfo(), FontInfo::instance<SegoeFluentIcons>());

        QVERIFY(addFriend1 != null1);
        QVERIFY(addFriend1 != null2);

        QVERIFY(addFriend2 != null1);
        QVERIFY(addFriend2 != null2);

        QCOMPARE(addFriend1, addFriend2);

        const auto solidStar = Symbol{SolidStar};

        QVERIFY(!solidStar.isNull());
        QCOMPARE(solidStar.unicode(), unicode(SolidStar));
        QCOMPARE(solidStar.fontInfo(), FontInfo::instance<SegoeFluentIcons>());

        QVERIFY(solidStar != null1);
        QVERIFY(solidStar != null2);
        QVERIFY(solidStar != addFriend1);
        QVERIFY(solidStar != addFriend2);
    }

    void testSymbolAssignment()
    {
        auto symbol = Symbol{};
        auto addFriend = Symbol{SegoeFluentIcons::AddFriend};
        auto solidStar = Symbol{SegoeMdl2Assets::SolidStar};

        QVERIFY(symbol.isNull());

        QCOMPARE(addFriend.unicode(), unicode(SegoeFluentIcons::AddFriend));
        QCOMPARE(addFriend.fontInfo(), FontInfo::instance<SegoeFluentIcons>());

        QCOMPARE(solidStar.unicode(), unicode(SegoeMdl2Assets::SolidStar));
        QCOMPARE(solidStar.fontInfo(), FontInfo::instance<SegoeMdl2Assets>());

        {
            const auto oldSymbol = std::exchange(symbol, addFriend);
            QVERIFY(oldSymbol.isNull());
        }

        QCOMPARE(symbol.unicode(), unicode(SegoeFluentIcons::AddFriend));
        QCOMPARE(symbol.fontInfo(), FontInfo::instance<SegoeFluentIcons>());

        QCOMPARE(addFriend.unicode(), unicode(SegoeFluentIcons::AddFriend));
        QCOMPARE(addFriend.fontInfo(), FontInfo::instance<SegoeFluentIcons>());

        QCOMPARE(solidStar.unicode(), unicode(SegoeMdl2Assets::SolidStar));
        QCOMPARE(solidStar.fontInfo(), FontInfo::instance<SegoeMdl2Assets>());

        {
            const auto oldSymbol = std::exchange(symbol, solidStar);

            QCOMPARE(oldSymbol.unicode(), unicode(SegoeFluentIcons::AddFriend));
            QCOMPARE(oldSymbol.fontInfo(), FontInfo::instance<SegoeFluentIcons>());
        }

        QCOMPARE(symbol.unicode(), unicode(SegoeMdl2Assets::SolidStar));
        QCOMPARE(symbol.fontInfo(), FontInfo::instance<SegoeMdl2Assets>());

        QCOMPARE(addFriend.unicode(), unicode(SegoeFluentIcons::AddFriend));
        QCOMPARE(addFriend.fontInfo(), FontInfo::instance<SegoeFluentIcons>());

        QCOMPARE(solidStar.unicode(), unicode(SegoeMdl2Assets::SolidStar));
        QCOMPARE(solidStar.fontInfo(), FontInfo::instance<SegoeMdl2Assets>());
    }

    void testDrawIconOptions_data()
    {
        QTest::addColumn<DrawIconOptions>          ("option1");
        QTest::addColumn<DrawIconOptions>          ("option2");
        QTest::addColumn<DrawIconOptional>("expectedOptional");
        QTest::addColumn<bool>               ("expectDefault");

        QTest::newRow("fillBox")
                << DrawIconOptions{.fillBox = true}
                << DrawIconOptions{.fillBox = false}
                << DrawIconOptional::FillBox
                << false;

        QTest::newRow("pixelSize")
                << DrawIconOptions{.pixelSize = 12}
                << DrawIconOptions{.pixelSize = 16}
                << DrawIconOptional::PixelSize
                << false;

        QTest::newRow("pointSize")
                << DrawIconOptions{.pointSize = 9.5}
                << DrawIconOptions{.pointSize = 12}
                << DrawIconOptional::PointSize
                << false;

        QTest::newRow("applyColor")
                << DrawIconOptions{.applyColor = true}
                << DrawIconOptions{.applyColor = false}
                << DrawIconOptional::None
                << true;

        QTest::newRow("mode")
                << DrawIconOptions{.mode = QIcon::Normal}
                << DrawIconOptions{.mode = QIcon::Active}
                << DrawIconOptional::Mode
                << false;
    }

    void testDrawIconOptions()
    {
        const QFETCH(DrawIconOptions,           option1);
        const QFETCH(DrawIconOptions,           option2);
        const QFETCH(DrawIconOptional, expectedOptional);
        const QFETCH(bool,                expectDefault);

        QCOMPARE(option1, option1);
        QCOMPARE(option2, option2);

        QVERIFY (option1 != option2);
        QCOMPARE(option1 == DrawIconOptions{}, expectDefault);
        QVERIFY (option2 != DrawIconOptions{});

        QCOMPARE(option1.fillBox.has_value(),   expectedOptional == DrawIconOptional::FillBox);
        QCOMPARE(option1.pixelSize.has_value(), expectedOptional == DrawIconOptional::PixelSize);
        QCOMPARE(option1.pointSize.has_value(), expectedOptional == DrawIconOptional::PointSize);
        QCOMPARE(option1.mode.has_value(),      expectedOptional == DrawIconOptional::Mode);

        QCOMPARE(option2.fillBox.has_value(),   expectedOptional == DrawIconOptional::FillBox);
        QCOMPARE(option2.pixelSize.has_value(), expectedOptional == DrawIconOptional::PixelSize);
        QCOMPARE(option2.pointSize.has_value(), expectedOptional == DrawIconOptional::PointSize);
        QCOMPARE(option2.mode.has_value(),      expectedOptional == DrawIconOptional::Mode);
    }

    void testEffectiveColor_data()
    {
        QTest::addColumn<QColor>            ("color");
        QTest::addColumn<QPalette>        ("palette");
        QTest::addColumn<DrawIconOptions> ("options");
        QTest::addColumn<QIcon::Mode>("fallbackMode");
        QTest::addColumn<QColor>    ("expectedColor");

        const auto       normalColor = QColor{QRgb{0x11}};
        const auto       activeColor = QColor{QRgb{0x12}};
        const auto     selectedColor = QColor{QRgb{0x13}};
        const auto     disabledColor = QColor{QRgb{0x14}};
        const auto   normalRoleColor = QColor{QRgb{0x21}};
        const auto   activeRoleColor = QColor{QRgb{0x22}};
        const auto disabledRoleColor = QColor{QRgb{0x23}};
        const auto     explicitColor = QColor{QRgb{0x31}};
        const auto           noColor = QColor{};

        auto palette = QPalette{};

        palette.setColor(QPalette::Inactive, QPalette::Text,            normalColor);
        palette.setColor(QPalette::Active,   QPalette::Text,            activeColor);
        palette.setColor(QPalette::Active,   QPalette::HighlightedText, selectedColor);
        palette.setColor(QPalette::Disabled, QPalette::Text,            disabledColor);

        palette.setColor(QPalette::Inactive, QPalette::Link,          normalRoleColor);
        palette.setColor(QPalette::Active,   QPalette::Link,          activeRoleColor);
        palette.setColor(QPalette::Disabled, QPalette::Link,        disabledRoleColor);

        const auto  defaultOptions = DrawIconOptions{};
        const auto   ignoreOptions = DrawIconOptions{.applyColor = false};
        const auto selectedOptions = DrawIconOptions{.mode = QIcon::Selected};
        const auto disabledOptions = DrawIconOptions{.mode = QIcon::Disabled};
        const auto     roleOptions = DrawIconOptions{.role = QPalette::Link};

        QTest::newRow("none:default:normal")     << noColor       << palette <<  defaultOptions << QIcon::Normal   <<   normalColor;
        QTest::newRow("none:default:active")     << noColor       << palette <<  defaultOptions << QIcon::Active   <<   activeColor;
        QTest::newRow("none:default:disabled")   << noColor       << palette <<  defaultOptions << QIcon::Disabled << disabledColor;
        QTest::newRow("none:default:selected")   << noColor       << palette <<  defaultOptions << QIcon::Selected << selectedColor;

        QTest::newRow("color:default:normal")    << explicitColor << palette <<  defaultOptions << QIcon::Normal   << explicitColor;
        QTest::newRow("color:default:active")    << explicitColor << palette <<  defaultOptions << QIcon::Active   << explicitColor;
        QTest::newRow("color:default:disabled")  << explicitColor << palette <<  defaultOptions << QIcon::Disabled << disabledColor;
        QTest::newRow("color:default:selected")  << explicitColor << palette <<  defaultOptions << QIcon::Selected << explicitColor;

        QTest::newRow("none:ignore:normal")      << noColor       << palette <<   ignoreOptions << QIcon::Normal   <<   normalColor;
        QTest::newRow("none:ignore:active")      << noColor       << palette <<   ignoreOptions << QIcon::Active   <<   activeColor;
        QTest::newRow("none:ignore:disabled")    << noColor       << palette <<   ignoreOptions << QIcon::Disabled << disabledColor;
        QTest::newRow("none:ignore:selected")    << noColor       << palette <<   ignoreOptions << QIcon::Selected << selectedColor;

        QTest::newRow("color:ignore:normal")     << explicitColor << palette <<   ignoreOptions << QIcon::Normal   <<   normalColor;
        QTest::newRow("color:ignore:active")     << explicitColor << palette <<   ignoreOptions << QIcon::Active   <<   activeColor;
        QTest::newRow("color:ignore:disabled")   << explicitColor << palette <<   ignoreOptions << QIcon::Disabled << disabledColor;
        QTest::newRow("color:ignore:selected")   << explicitColor << palette <<   ignoreOptions << QIcon::Selected << selectedColor;

        QTest::newRow("none:selected:normal")    << noColor       << palette << selectedOptions << QIcon::Normal   << selectedColor;
        QTest::newRow("none:selected:active")    << noColor       << palette << selectedOptions << QIcon::Active   << selectedColor;
        QTest::newRow("none:selected:disabled")  << noColor       << palette << selectedOptions << QIcon::Disabled << selectedColor;
        QTest::newRow("none:selected:selected")  << noColor       << palette << selectedOptions << QIcon::Selected << selectedColor;

        QTest::newRow("color:selected:normal")   << explicitColor << palette << selectedOptions << QIcon::Normal   << explicitColor;
        QTest::newRow("color:selected:active")   << explicitColor << palette << selectedOptions << QIcon::Active   << explicitColor;
        QTest::newRow("color:selected:disabled") << explicitColor << palette << selectedOptions << QIcon::Disabled << explicitColor;
        QTest::newRow("color:selected:selected") << explicitColor << palette << selectedOptions << QIcon::Selected << explicitColor;

        QTest::newRow("none:disabled:normal")    << noColor       << palette << disabledOptions << QIcon::Normal   << disabledColor;
        QTest::newRow("none:disabled:active")    << noColor       << palette << disabledOptions << QIcon::Active   << disabledColor;
        QTest::newRow("none:disabled:disabled")  << noColor       << palette << disabledOptions << QIcon::Disabled << disabledColor;
        QTest::newRow("none:disabled:selected")  << noColor       << palette << disabledOptions << QIcon::Selected << disabledColor;

        QTest::newRow("color:disabled:normal")   << explicitColor << palette << disabledOptions << QIcon::Normal   << disabledColor;
        QTest::newRow("color:disabled:active")   << explicitColor << palette << disabledOptions << QIcon::Active   << disabledColor;
        QTest::newRow("color:disabled:disabled") << explicitColor << palette << disabledOptions << QIcon::Disabled << disabledColor;
        QTest::newRow("color:disabled:selected") << explicitColor << palette << disabledOptions << QIcon::Selected << disabledColor;

        QTest::newRow("none:role:normal")        << noColor       << palette <<     roleOptions << QIcon::Normal   <<   normalRoleColor;
        QTest::newRow("none:role:active")        << noColor       << palette <<     roleOptions << QIcon::Active   <<   activeRoleColor;
        QTest::newRow("none:role:disabled")      << noColor       << palette <<     roleOptions << QIcon::Disabled << disabledRoleColor;
        QTest::newRow("none:role:selected")      << noColor       << palette <<     roleOptions << QIcon::Selected <<     selectedColor;

        QTest::newRow("color:role:normal")       << explicitColor << palette <<     roleOptions << QIcon::Normal   <<     explicitColor;
        QTest::newRow("color:role:active")       << explicitColor << palette <<     roleOptions << QIcon::Active   <<     explicitColor;
        QTest::newRow("color:role:disabled")     << explicitColor << palette <<     roleOptions << QIcon::Disabled << disabledRoleColor;
        QTest::newRow("color:role:selected")     << explicitColor << palette <<     roleOptions << QIcon::Selected <<     explicitColor;
    }

    void testEffectiveColor()
    {
        const QFETCH(QColor,             color);
        const QFETCH(QPalette,         palette);
        const QFETCH(DrawIconOptions,  options);
        const QFETCH(QIcon::Mode, fallbackMode);
        const QFETCH(QColor,     expectedColor);

        QCOMPARE(options.effectiveColor(color, palette, fallbackMode), expectedColor);
    }

    void testPreserveTransform()
    {
        auto icon = FontIcon{};

        QCOMPARE(icon.transformType(), FontIcon::Transform::None);
        QVERIFY(!icon.color().isValid());
        QVERIFY(icon.isNull());

        icon = FontIcon{SolidStar};

        QCOMPARE(icon.transformType(), FontIcon::Transform::None);
        QCOMPARE(icon.symbol().unicode(), unicode(SolidStar));
        QVERIFY(!icon.color().isValid());

        icon = FontIcon{SolidStar, FontIcon::Transform::VerticalFlip};

        QCOMPARE(icon.transformType(), FontIcon::Transform::VerticalFlip);
        QCOMPARE(icon.symbol().unicode(), unicode(SolidStar));
        QVERIFY(!icon.color().isValid());

        icon |= FontIcon::Transform::HorizontalFlip;

        QCOMPARE(icon.transformType(), FontIcon::Transform::HorizontalFlip);
        QCOMPARE(icon.symbol().unicode(), unicode(SolidStar));
        QVERIFY(!icon.color().isValid());

        icon |= Qt::red;

        QCOMPARE(icon.transformType(), FontIcon::Transform::HorizontalFlip);
        QCOMPARE(icon.symbol().unicode(), unicode(SolidStar));
        QCOMPARE(icon.color(), Qt::red);

        icon |= AddFriend;

        QCOMPARE(icon.transformType(), FontIcon::Transform::HorizontalFlip);
        QCOMPARE(icon.symbol().unicode(), unicode(AddFriend));
        QCOMPARE(icon.color(), Qt::red);
    }

    void testDiscoverTransform_data()
    {
        QTest::addColumn<FontIcon::Transform>("transformType");
        QTest::addColumn<QTransform>("transformMatrix");
        QTest::addColumn<QTransform::TransformationType>("expectedMatrixType");

        using enum FontIcon::Transform;

        // explicitly not using FontIcon::instance<Transform>() to also test that function
        const auto matrixForTransformType = [](FontIcon::Transform transform) {
            switch (transform) {
            case None:             return QTransform{};
            case HorizontalFlip:   return QTransform{}.scale(-1, 1);
            case VerticalFlip:     return QTransform{}.scale(1, -1);
            case Rotate90:         return QTransform{}.rotate(90);
            case Rotate180:        return QTransform{}.rotate(180);
            case Rotate270:        return QTransform{}.rotate(270);
            case Matrix:           return QTransform{}.rotate(22.5).scale(0.9, 0.9);
            }

            Q_UNREACHABLE_RETURN(QTransform{});
        };

        const auto matrixTypeForTransformType = [](FontIcon::Transform transform) {
            switch (transform) {
            case None:
                return QTransform::TxNone;

            case HorizontalFlip:
            case VerticalFlip:
            case Rotate180:
                return QTransform::TxScale;

            case Rotate90:
            case Rotate270:
            case Matrix:
                return QTransform::TxRotate;
            }

            Q_UNREACHABLE_RETURN(QTransform::TxNone);
        };

        const auto metaEnum = QMetaEnum::fromType<FontIcon::Transform>();

        for (auto i = 0; i < metaEnum.keyCount(); ++i) {
            const auto type = FontIcon::Transform{metaEnum.value(i)};
            QTest::newRow(metaEnum.key(i)) << type << matrixForTransformType(type)
                                           << matrixTypeForTransformType(type);
        }

        QTest::newRow("RotateXAxis") << Matrix << QTransform{}.rotate(90, Qt::XAxis) << QTransform::TxProject;
        QTest::newRow("RotateYAxis") << Matrix << QTransform{}.rotate(90, Qt::YAxis) << QTransform::TxProject;
    }

    void testDiscoverTransform()
    {
        const QFETCH(FontIcon::Transform,            transformType);
        const QFETCH(QTransform,                     transformMatrix);
        const QFETCH(QTransform::TransformationType, expectedMatrixType);

        if (transformType != FontIcon::Transform::Matrix) { // cannot produce custom matrix from transform type
            const auto fromType = SolidStar | transformType;
            QCOMPARE(fromType.transformType(), transformType);
            QCOMPARE(fromType.transform(), transformMatrix);
        }

        const auto fromMatrix = SolidStar | transformMatrix;
        QCOMPARE(fromMatrix.transform(), transformMatrix);
        QCOMPARE(fromMatrix.transformType(), transformType);

        QCOMPARE(transformMatrix.type(), expectedMatrixType);
    }

    void testFontLoadable_data()
    {
        collectFontInfoData();
    }

    void testFontLoadable()
    {
        const QFETCH(FontInfo, font);
        ignoreFontLoadingMessage(font);

        QVERIFY(font.enumType().isValid());
        QVERIFY(font.enumType().flags().testFlag(QMetaType::IsEnumeration));
        QVERIFY(font.isValid());

        QVERIFY(font.isAvailable());

        QVERIFY(!font.fontName().isEmpty());
        QVERIFY(!font.fontFamily().isEmpty());
    }

    void testFontLicenseAvailable_data()
    {
        collectFontInfoData();
    }

    void testFontLicenseAvailable()
    {
        const QFETCH(FontInfo, font);
        ignoreFontLoadingMessage(font);

        QVERIFY(!font.licenseFileName().isEmpty());
        QVERIFY(!font.licenseText().isEmpty());

        auto licenseFile = QFile{font.licenseFileName()};

        QVERIFY2(licenseFile.exists(), qPrintable(licenseFile.fileName()));
        QVERIFY2(licenseFile.size() > 0 , qPrintable(licenseFile.fileName()));
        QVERIFY2(licenseFile.open(QFile::ReadOnly), qPrintable(licenseFile.errorString()));

        const auto &licenseTextFromFile = licenseFile.readAll();

        QCOMPARE(licenseTextFromFile.size(), licenseFile.size());
        QCOMPARE(removeByteOrderMarker(licenseTextFromFile),
                 font.licenseText().toUtf8());
    }

    void testSymbolProperties_data()
    {
        QTest::addColumn<Symbol>("symbol");
        QTest::addColumn<bool>("expectedIsNull");
        QTest::addColumn<QFont>("expectedFont");
        QTest::addColumn<QString>("expectedName");
        QTest::addColumn<char32_t>("expectedUnicode");

        QTest::newRow("default") << Symbol{} << true << QFont{} << QString{} << char32_t{};
        QTest::newRow("SolidStar") << Symbol{SolidStar} << false << font<decltype(SolidStar)>() << "SolidStar" << unicode(SolidStar);
    }

    void testSymbolProperties()
    {
        const QFETCH(Symbol,   symbol);
        const QFETCH(bool,     expectedIsNull);
        const QFETCH(QFont,    expectedFont);
        const QFETCH(QString,  expectedName);
        const QFETCH(char32_t, expectedUnicode);

        QCOMPARE(symbol.isNull(),   expectedIsNull);
        QCOMPARE(symbol.font(),     expectedFont);
        QCOMPARE(symbol.name(),     expectedName);
        QCOMPARE(symbol.unicode(),  expectedUnicode);

        const auto &mo = Symbol::staticMetaObject;

        const auto  isNullProperty = mo.property(mo.indexOfProperty("isNull"));
        const auto    fontProperty = mo.property(mo.indexOfProperty("font"));
        const auto    nameProperty = mo.property(mo.indexOfProperty("name"));
        const auto unicodeProperty = mo.property(mo.indexOfProperty("unicode"));

        QVERIFY( isNullProperty.isValid());
        QVERIFY(   fontProperty.isValid());
        QVERIFY(   nameProperty.isValid());
        QVERIFY(unicodeProperty.isValid());

        QCOMPARE( isNullProperty.readOnGadget(&symbol), expectedIsNull);
        QCOMPARE(   fontProperty.readOnGadget(&symbol), expectedFont);
        QCOMPARE(   nameProperty.readOnGadget(&symbol), expectedName);
        QCOMPARE(unicodeProperty.readOnGadget(&symbol), expectedUnicode);
    }

    void testKnownFontsCount()
    {
#ifdef ICONFONTS_ENABLE_ALL_FONTS
        QCOMPARE(FontInfo::knownFonts().count(), 52);
#endif

        const auto &knownFonts = FontInfo::knownFonts();
        const auto symbolCount = std::accumulate(knownFonts.cbegin(), knownFonts.cend(),
                                                 0, [](int count, const auto &font) {
            return count + font.symbolCount();
        });

        qInfo() << "font count:" << knownFonts.count();
        qInfo() << "symbol count:" << symbolCount;
    }
};

} // namespace
} // namespace iconfonts::tests

QTEST_MAIN(IconFonts::Tests::IconFontsTest)

#include "tst_iconfonts.moc"
