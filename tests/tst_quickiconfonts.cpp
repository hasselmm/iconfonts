#include "iconfonts/quickiconfonts.h"

#include <QTest>

using namespace Qt::StringLiterals;

namespace QuickIconFonts::Tests {
namespace {

class CPlusPlusTest : public QObject
{
    Q_OBJECT

private slots:
    void testPropertiesComplete_data()
    {
        QMetaType::registerConverter<Symbol, IconFonts::Symbol>(); // FIXME: find better place
        QMetaType::registerConverter<IconFonts::Symbol, Symbol>();

        QTest::addColumn<QMetaType>("baseType");
        QTest::addColumn<QMetaType>("quickType");
        QTest::addColumn<QMetaProperty>("baseProperty");

        const auto addPropertyRows = [](const char *name, QMetaType quickType, QMetaType baseType) {
            const auto metaObject = baseType.metaObject();

            for (auto i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i) {
                const auto &property = metaObject->property(i);
                QVERIFY(property.isValid());

                QTest::addRow("%s:%s", name, property.name())
                        << baseType << quickType << property;
            }
        };

        addPropertyRows("fonticon", QMetaType::fromType<FontIcon>(),
                        QMetaType::fromType<IconFonts::FontIcon>());

        addPropertyRows("symbol", QMetaType::fromType<Symbol>(),
                        QMetaType::fromType<IconFonts::Symbol>());
    }

    void testPropertiesComplete()
    {
        const QFETCH(QMetaType, baseType);
        const QFETCH(QMetaType, quickType);
        const QFETCH(QMetaProperty, baseProperty);

        const auto quickMetaObject = quickType.metaObject();
        QVERIFY(quickMetaObject != nullptr);

        const auto quickPropertyIndex = quickMetaObject->indexOfProperty(baseProperty.name());
        const auto &quickProperty = quickMetaObject->property(quickPropertyIndex);
        QVERIFY(quickProperty.isValid());

        QCOMPARE(quickProperty.name(),       baseProperty.name());
        QCOMPARE(quickProperty.isReadable(), baseProperty.isReadable());
        QCOMPARE(quickProperty.isWritable(), baseProperty.isWritable());
        QCOMPARE(quickProperty.isConstant(), baseProperty.isConstant());
        QCOMPARE(quickProperty.isFinal(),    baseProperty.isFinal());

        if (quickProperty.metaType() != baseProperty.metaType()) {
            QVERIFY2(QMetaType::canConvert(quickProperty.metaType(), baseProperty.metaType()),
                    quickProperty.metaType().name());
            QVERIFY(QMetaType::canConvert(baseProperty.metaType(), quickProperty.metaType()));
        }
    }
};

} // namespace
} // namespace QuickIconFonts::Tests

QTEST_MAIN(QuickIconFonts::Tests::CPlusPlusTest)

#include "tst_quickiconfonts.moc"
