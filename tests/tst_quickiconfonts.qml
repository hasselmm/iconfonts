import IconFonts

import QtQuick
import QtTest

TestCase {
    name: "QuickIconFonts::Tests::QmlTest"

    property font     nullFont
    property fonticon nullIcon
    property symbol   nullSymbol

    readonly property symbol   roundedSymbol:     MaterialSymbolsRounded.Bubbles
    readonly property fonticon roundedIconDirect: MaterialSymbolsRounded.ElectricBike
    readonly property fonticon roundedIconSymbol: roundedSymbol

    readonly property symbol   sharpSymbol:     MaterialSymbolsSharp.Cloud
    readonly property fonticon sharpIconDirect: MaterialSymbolsSharp.SolarPower
    readonly property fonticon sharpIconSymbol: sharpSymbol

    function test_symbolType_data() {
        return [{
                    tag:                'symbol:null',
                    symbol:             nullSymbol,
                    expectedIsNull:     true,
                    expectedName:       '',
                    expectedText:       '',
                    expectedUnicode:    0,
                    expectedFont:       nullFont,
                }, {
                    tag:                'symbol:rounded',
                    symbol:             roundedSymbol,
                    expectedIsNull:     false,
                    expectedName:       'Bubbles',
                    expectedText:       '\uf64e',
                    expectedUnicode:    0xf64e,
                    expectedFont:       MaterialSymbolsRounded.font,
                }, {
                    tag:                'symbol:sharp',
                    symbol:             sharpSymbol,
                    expectedIsNull:     false,
                    expectedName:       'Cloud',
                    expectedText:       '\uf15c',
                    expectedUnicode:    0xf15c,
                    expectedFont:       MaterialSymbolsSharp.font,
                }, {
                    tag:                'icon:null',
                    symbol:             nullIcon.symbol,
                    expectedIsNull:     true,
                    expectedName:       '',
                    expectedText:       '',
                    expectedUnicode:    0,
                    expectedFont:       nullFont,
                }, {
                    tag:                'icon:rounded:direct',
                    symbol:             roundedIconDirect.symbol,
                    expectedIsNull:     false,
                    expectedName:       'ElectricBike',
                    expectedText:       '\ueb1b',
                    expectedUnicode:    0xeb1b,
                    expectedFont:       MaterialSymbolsRounded.font,
                }, {
                    tag:                'icon:sharp:direct',
                    symbol:             sharpIconDirect.symbol,
                    expectedIsNull:     false,
                    expectedName:       'SolarPower',
                    expectedText:       '\uec0f',
                    expectedUnicode:    0xec0f,
                    expectedFont:       MaterialSymbolsSharp.font,
                }, {
                    tag:                'icon:rounded:symbol',
                    symbol:             roundedIconSymbol.symbol,
                    expectedIsNull:     false,
                    expectedName:       'Bubbles',
                    expectedText:       '\uf64e',
                    expectedUnicode:    0xf64e,
                    expectedFont:       MaterialSymbolsRounded.font,
                }, {
                    tag:                'icon:sharp:symbol',
                    symbol:             sharpIconSymbol.symbol,
                    expectedIsNull:     false,
                    expectedName:       'Cloud',
                    expectedText:       '\uf15c',
                    expectedUnicode:    0xf15c,
                    expectedFont:       MaterialSymbolsSharp.font,
                }];
    }

    function test_symbolType(data) {
        compare(data.symbol.isNull,     data.expectedIsNull);
        compare(data.symbol.name,       data.expectedName);
        compare(data.symbol.text,       data.expectedText);
        compare(data.symbol.toString(), data.expectedText);
        compare(data.symbol.unicode,    data.expectedUnicode);
        compare(data.symbol.font,       data.expectedFont);
    }
}
