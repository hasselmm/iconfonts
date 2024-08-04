#include "iconfonts/iconfonts.h"

#ifdef ICONFONTS_ENABLE_SEGOE_FLUENTICONS
#include "iconfonts/segoefluenticons.h"
#endif

#ifdef ICONFONTS_ENABLE_SEGOE_MDL2ASSETS
#include "iconfonts/segoemdl2assets.h"
#endif

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

#if   defined(ICONFONTS_ENABLE_SEGOE_FLUENTICONS)
    using enum SegoeFluentIcons;
#elif defined(ICONFONTS_ENABLE_SEGOE_MDL2ASSETS)
    using enum SegoeMdl2Assets;
#else
#error No fonts configured for testing
#endif

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
        QTest::addColumn<DrawIconOptions>("option1");
        QTest::addColumn<DrawIconOptions>("option2");

        QTest::newRow("fillRect")
                << DrawIconOptions{.fillRect = true}
                << DrawIconOptions{.fillRect = false};

        QTest::newRow("pixelSize")
                << DrawIconOptions{.pixelSize = 12}
                << DrawIconOptions{.pixelSize = 16};

        QTest::newRow("pointSize")
                << DrawIconOptions{.pointSize = 9.5}
                << DrawIconOptions{.pointSize = 12};

        QTest::newRow("applyColor")
                << DrawIconOptions{.applyColor = true}
                << DrawIconOptions{.applyColor = false};
    }

    void testDrawIconOptions()
    {
        const QFETCH(DrawIconOptions, option1);
        const QFETCH(DrawIconOptions, option2);

        QCOMPARE(option1, option1);
        QCOMPARE(option2, option2);
        QVERIFY(option1 != option2);
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

    void testFontGlyphsRenderable_data()
    {
        collectFontInfoData();
    }

    void testFontGlyphsRenderable()
    {
        const QFETCH(FontInfo, font);
        ignoreFontLoadingMessage(font);

        const auto metrics = QFontMetrics{font};
        auto badSymbols = QStringList{};

        for (auto count = font.symbolCount(), i = 0; i < count; ++i) {
            const auto &symbol = font.symbol(i);
            const auto &box = metrics.boundingRect(symbol);

            if (box.isEmpty())
                badSymbols.emplaceBack(symbol.name());
        }

        QVERIFY2(badSymbols.isEmpty(), qPrintable(badSymbols.join(u", "_s)));
    }

    void testKnownFontsCount()
    {
        QCOMPARE(FontInfo::knownFonts().count(), 52);

        const auto &knownFonts = FontInfo::knownFonts();
        const auto symbolCount = std::accumulate(knownFonts.cbegin(), knownFonts.cend(),
                                                 0, [](int count, const auto &font) {
            return count + font.symbolCount();
        });

        qInfo() << "symbol count:" << symbolCount;
    }
};

} // namespace
} // namespace iconfonts::tests

QTEST_MAIN(IconFonts::Tests::IconFontsTest)

#include "tst_iconfonts.moc"
