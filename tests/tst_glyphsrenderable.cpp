#include "iconfonts/iconfonts.h"

#include <QFontMetrics>
#include <QTest>

using namespace Qt::StringLiterals;

namespace IconFonts::Tests {
namespace {

class GlyphsRenderableTest : public QObject
{
    Q_OBJECT

private:
    void testFontGlyphsRenderable_data()
    {
        QTest::addColumn<FontInfo>("font");

        for (const auto &font : FontInfo::knownFonts())
            QTest::newRow(font.enumType().name()) << font;
    }

    void testFontGlyphsRenderable()
    {
        const QFETCH(FontInfo, font);

        auto message = uR"(.* is available via font id \d+)"_s;
        QTest::ignoreMessage(QtDebugMsg, QRegularExpression{message});

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
};

} // namespace
} // namespace iconfonts::tests

QTEST_MAIN(IconFonts::Tests::GlyphsRenderableTest)

#include "tst_glyphsrenderable.moc"
