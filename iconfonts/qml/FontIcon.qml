import IconFonts
import QtQuick

/// This item displays a font icon.
Item {
    id: fontIcon

    /// The font icon to display.
    property fonticon icon

    /// Additional options for displaying the icon.
    property drawIconOptions options

    /// Items embedded in QQuickWidget do not have a native window attached. Therefore their palette is inactive
    /// all the time. This property allows overriding the active state, so that a proper palette is picked.
    property bool active: Window.window ? Window.window.active : true

    Text {
        id: content

        anchors.centerIn: parent

        renderType: Text.NativeRendering
        renderTypeQuality: Text.VeryHighRenderTypeQuality
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: fontIcon.icon.text

        color: {
            return fontIcon.options.effectiveColor(fontIcon, fontIcon.icon.color, fontIcon.active);
        }

        font: {
            let font = {family: fontIcon.icon.font.family};

            if (fontIcon.options.hasPointSize)
                font.pointSize = fontIcon.options.pointSize;
            else if (fontIcon.options.hasPixelSize)
                font.pixelSize = fontIcon.options.pixelSize;
            else
                font.pixelSize = Math.min(fontIcon.width, fontIcon.height);

            return font;
        }

        transform: [
            Translate { x: -content.width/2; y: -content.height/2 },
            Matrix4x4 { id: matrix; matrix: fontIcon.icon.transform },
            Translate { x: +content.width/2; y: +content.height/2 }
        ]
    }
}
