# IconFonts library # --------------------------------------------------------------------------------------------------

qt_add_library(
    IconFonts
    iconfonts.cpp
    iconfonts.h
    iconfonts_p.h
)

target_compile_definitions(
    IconFonts
    INTERFACE   ICONFONTS_EXPORT=Q_DECL_IMPORT
    PRIVATE     ICONFONTS_EXPORT=Q_DECL_EXPORT
    PRIVATE     ICONFONTS_LIBRARY
)

target_include_directories(
    IconFonts PUBLIC
        "${ICONFONTS_SOURCE_DIR}"
        "${ICONFONTS_SOURCE_DIR}/iconfonts" # FIXME copy all static headers into generated sources folder and then remove this hack
)

target_link_libraries(
    IconFonts PUBLIC
    namedoptions
    Qt::Gui
    Qt::GuiPrivate # for QFontEngine
)

# Ant Design Icons # ---------------------------------------------------------------------------------------------------

iconfonts_add_font_family(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Ant Design Icons"

    BASE_URL            "https://github.com/ant-design/ant-design-icons@d7d0627628cdf47fa79e5c7b878aa205a977681d"
    RESOURCE_PREFIX     "/ant-design/icons"
    VARIANT_PATTERN     "ant(.*)"

    ARCHIVE             "npm:@ant-design/icons-react-native/2.3.2"
    ARCHIVE_FILEHASH    "SHA1=86e412d1ce8d629534a5d9f45cd3d8f7ca6dae56"

    FONT_VARIANTS
        "package/fonts/antfill.ttf      :SHA1=56960e7721fc92b62e0f7c4d131ffe34ed042c49
        :package/lib/fill.js            :SHA1=cf02969963449b5d03127ed9cc147873effc5308
        :mapping=fillGlyphMap"

        "package/fonts/antoutline.ttf   :SHA1=66720607b7496a48f145425386b2082b73662fd1
        :package/lib/outline.js         :SHA1=64c120f2b551c409b52496a85290733422153a6e
        :mapping=outlineGlyphMap"

    LICENSE_FILEPATH    "LICENSE"
    LICENSE_FILEHASH    "SHA1=32c5f13c36b11e945b4615bbebd11483946ad883"
)

# Bootstrap Icon # -----------------------------------------------------------------------------------------------------

iconfonts_add_font(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Bootstrap"

    BASE_URL            "https://github.com/twbs/icons@v1.11.3"
    RESOURCE_PREFIX     "/bootstrap/icons"

    FONT_FILEPATH       "font/fonts/bootstrap-icons.woff2"
    FONT_FILEHASH       "SHA1=020c3c6f9280a315e8425d7f92e15bcd0cdda1b2"

    INFO_FILEPATH       "font/bootstrap-icons.json"
    INFO_FILEHASH       "SHA1=e4adb76a19cc64a1d61125bb997d286254c87be5"

    LICENSE_FILEPATH    "LICENSE"
    LICENSE_FILEHASH    "SHA1=acac50435cf9dc993396653dcc5335fc4b52efb5"
)

# Box Icons # ----------------------------------------------------------------------------------------------------------

iconfonts_add_font_family(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Boxicons"

    BASE_URL            "https://github.com/atisawd/boxicons@9ffa9136e8681886bb7bd2145cd4098717ce1c11" # 2.1.4
    RESOURCE_PREFIX     "/boxicons"

    FONT_VARIANTS
        "fonts/boxicons.ttf :SHA1=f18d97b413c192509c83c445d74b79de2f20af71
        :css/boxicons.css   :SHA1=2b357efbd1671f10c92efb21559ce0a7ddee6088
        :prefix=.bx- variant=Regular"

        "fonts/boxicons.ttf :SHA1=f18d97b413c192509c83c445d74b79de2f20af71
        :css/boxicons.css   :SHA1=2b357efbd1671f10c92efb21559ce0a7ddee6088
        :prefix=.bxs- variant=Solid"

        "fonts/boxicons.ttf :SHA1=f18d97b413c192509c83c445d74b79de2f20af71
        :css/boxicons.css   :SHA1=2b357efbd1671f10c92efb21559ce0a7ddee6088
        :prefix=.bxl- variant=Logos"

    LICENSE_FILEPATH    "LICENSE"
    LICENSE_FILEHASH    "SHA1=57b1bfac18573db22719c2d64c49e0acd516ba09"
)

# Devicons # -----------------------------------------------------------------------------------------------------------

iconfonts_add_font(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Devicons"

    BASE_URL            "https://github.com/vorillaz/devicons@1.7.0"
    RESOURCE_PREFIX     "/vorillaz/devicons"

    FONT_FILEPATH       "fonts/devicons.ttf"
    FONT_FILEHASH       "SHA1=26e5fb15f379d5332b47f1c13913d27763b1b9b0"

    INFO_FILEPATH       "css/devicons.css"
    INFO_FILEHASH       "SHA1=71ae6f9d9e792548ee65a4d1db9d9d7345ecee47"
    INFO_OPTIONS        "prefix=.devicons-"

    LICENSE_FILEPATH    "README.md" # FIXME find actual license
    LICENSE_FILEHASH    "SHA1=f52346a950a2b030b6d0bd2d2924f8287880d6f2"
)

# Font APEX  #----------------------------------------------------------------------------------------------------------

iconfonts_add_font_family(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Font APEX"

    BASE_URL            "https://github.com/oracle/font-apex@24631d9cc7ebb23ccc58bc676343d288bf453a62"
    RESOURCE_PREFIX     "/oracle.com/font-apex"
    VARIANT_PATTERN     "Font-APEX-(.*)"

    FONT_VARIANTS
        "fonts/Font-APEX-Large.ttf  :SHA1=f1334397726d162cca8b02d129e61a23bf2fae03
        :css/font-apex.css          :SHA1=2bebf1ac969ade922cb770505edb1b49649d32cf
        :prefix=.fa-"

        "fonts/Font-APEX-Small.ttf  :SHA1=8eb84d79842764441f9aa9ed894cfdff608d153d
        :css/font-apex.css          :SHA1=2bebf1ac969ade922cb770505edb1b49649d32cf
        :prefix=.fa-"


    LICENSE_FILEPATH    "LICENSE.txt"
    LICENSE_FILEHASH    "SHA1=0ff4e358902ee5ea63ae250e946879e8554145b0"
)

# Font Awesome 5 # -----------------------------------------------------------------------------------------------------

iconfonts_add_font_family(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Font Awesome 5"

    BASE_URL            "https://github.com/FortAwesome/Font-Awesome@5.15.4"
    RESOURCE_PREFIX     "/fontawesome/5"
    VARIANT_PATTERN     "Font Awesome 5 +(.*)-<5b>0-9<5d>+"

    FONT_VARIANTS
        "otfs/Font Awesome 5 Brands-Regular-400.otf :SHA1=e12fe81dca531411d1a5b92ecc9a7ad72672f7f3
        :metadata/icons.json                        :SHA1=51be69b7a0503c66dd3031d83422ccc38ac7c77c"

        "otfs/Font Awesome 5 Free-Regular-400.otf   :SHA1=7c250889ba46e1ad033fb1822d0df2dbbe7b801c
        :metadata/icons.json                        :SHA1=51be69b7a0503c66dd3031d83422ccc38ac7c77c"

        "otfs/Font Awesome 5 Free-Solid-900.otf     :SHA1=6c18555a587ded41634d4229b6b63a8406f7c786
        :metadata/icons.json                        :SHA1=51be69b7a0503c66dd3031d83422ccc38ac7c77c"

    LICENSE_FILEPATH    "LICENSE.txt"
    LICENSE_FILEHASH    "SHA1=9907c55306fffb685ce0e06c9e469475035d5fa6"
)

# Font Awesome 6 # -----------------------------------------------------------------------------------------------------

iconfonts_add_font_family(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Font Awesome 6"

    BASE_URL            "https://github.com/FortAwesome/Font-Awesome@6.6.0"
    RESOURCE_PREFIX     "/fontawesome/6"
    VARIANT_PATTERN     "Font Awesome 6 +(.*)-<5b>0-9<5d>+"

    FONT_VARIANTS
        "otfs/Font Awesome 6 Brands-Regular-400.otf :SHA1=7ffa3886f8e76c173b879bb50bc31aa6d084b6f7
        :metadata/icons.json                        :SHA1=4a3f7d5c1c2363be5ce3661efabe1d8850d8255b"

        "otfs/Font Awesome 6 Free-Regular-400.otf   :SHA1=d1137ee9799056add56de0a22a374055476999ba
        :metadata/icons.json                        :SHA1=4a3f7d5c1c2363be5ce3661efabe1d8850d8255b"

        "otfs/Font Awesome 6 Free-Solid-900.otf     :SHA1=0849458b151191d4834558dbe74e45ac59564d30
        :metadata/icons.json                        :SHA1=4a3f7d5c1c2363be5ce3661efabe1d8850d8255b"

    LICENSE_FILEPATH    "LICENSE.txt"
    LICENSE_FILEHASH    "SHA1=fa002248a93b21332d78e1e6b3b4a9992a8e8bdb"
)

# Foundation Icons Fonts # ---------------------------------------------------------------------------------------------

iconfonts_add_font_family(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Foundicons"

    BASE_URL            "https://github.com/zurb/foundation-icons@74f63dc35d569a8b796e86add76a0a535f96f734"
    RESOURCE_PREFIX     "/zurb/foundation-icons"
    VARIANT_PATTERN     "^(.*)_foundicons\$"

    FONT_VARIANTS
        "foundation_icons_accessibility/fonts/accessibility_foundicons.ttf
        :SHA1=2f23242c1efe696067a262944dd94f5af808fdf3
        :foundation_icons_accessibility/sass/accessibility_foundicons.scss
        :SHA1=1fa38e5653213b0c7672509ae17edf42d9e3c2ff"

        "foundation_icons_general/fonts/general_foundicons.ttf
        :SHA1=5da5d3c56b8731b084c004a83b92c4380d7c0e33
        :foundation_icons_general/sass/general_foundicons.scss
        :SHA1=6d60de3e965a8bd0d63ebcbfba5e8cea5fddbaa9"

        "foundation_icons_general_enclosed/fonts/general_enclosed_foundicons.ttf
        :SHA1=6eb93ccf276db5317a203d6549d9cec298eab3c1
        :foundation_icons_general_enclosed/sass/general_enclosed_foundicons.scss
        :SHA1=6d60de3e965a8bd0d63ebcbfba5e8cea5fddbaa9"

        "foundation_icons_social/fonts/social_foundicons.ttf
        :SHA1=3f9410223626aedfa0fa83a420ad63d136dcc465
        :foundation_icons_social/sass/social_foundicons.scss
        :SHA1=49e9b890c3f4d001c11eefff20192c80c8e07a6a"

    LICENSE_FILEPATH    "MIT-LICENSE.txt"
    LICENSE_FILEHASH    "SHA1=f1165010f9277435194a4d9db4ab1b362244e208"
)

# IcoMoon Free # -------------------------------------------------------------------------------------------------------

iconfonts_add_font(
    TARGET IconFonts OPTIONAL FONT_FAMILY "IcoMoon Free"

    BASE_URL            "https://github.com/Keyamoon/IcoMoon-Free@d006795ede82361e1bac1ee76f215cf1dc51e4ca"
    RESOURCE_PREFIX     "/icomoon/free"

    FONT_FILEPATH       "Font/IcoMoon-Free.ttf"
    FONT_FILEHASH       "SHA1=e40e2a5cb3d3aeee855a0b12ee49eac30bdcf202"

    INFO_FILEPATH       "Font/selection.json"
    INFO_FILEHASH       "SHA1=4e4670a960c49f0e9f4cad9cafe36b05e15b6d9a"

    LICENSE_FILEPATH    "License.txt"
    LICENSE_FILEHASH    "SHA1=5d8e6749d2d2fb3c669c7e14ddad66d384d79f09"
)

# Line Awesome # -------------------------------------------------------------------------------------------------------

iconfonts_add_font_family(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Line Awesome"

    BASE_URL            "https://github.com/icons8/line-awesome@78a101217707c9b1c4dcf2a821be75684e36307f" # 1.3.1
    RESOURCE_PREFIX     "/icons8/line-awesome"
    VARIANT_PATTERN     "la-?(.*)"

    FONT_VARIANTS
        "dist/line-awesome/fonts/la-brands-400.ttf  :SHA1=10a04af3d80f9a83ef2412dedd6b76be7a0c0a66
        :dist/line-awesome/css/line-awesome.css     :SHA1=37910d7fb9a06ab2c721ce19909a71c8a3cdbf31
        :prefix=.la-"

        "dist/line-awesome/fonts/la-regular-400.ttf :SHA1=63ef86b861a7d95d11f544dab477807d90f73e3a
        :dist/line-awesome/css/line-awesome.css     :SHA1=37910d7fb9a06ab2c721ce19909a71c8a3cdbf31
        :prefix=.la-"

        "dist/line-awesome/fonts/la-solid-900.ttf   :SHA1=dcdd439c2373daebe42ee0a3978be75b280c8318
        :dist/line-awesome/css/line-awesome.css     :SHA1=37910d7fb9a06ab2c721ce19909a71c8a3cdbf31
        :prefix=.la-"

    LICENSE_FILEPATH    "LICENSE.md"
    LICENSE_FILEHASH    "SHA1=b0b7a448b606326beb40a3132523dc67d0689ad4"
)

# Lucide # -------------------------------------------------------------------------------------------------------------

iconfonts_add_font(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Lucide"

    BASE_URL            "https://github.com/lucide-icons/lucide@0.424.0"
    RESOURCE_PREFIX     "/lucide-icons/lucide"

    ARCHIVE             "releases/lucide-font-0.424.0.zip"
    ARCHIVE_FILEHASH    "SHA1=8e5e0d790a6b4a5b51f9459b04207509e0c1838a"

    FONT_FILEPATH       "lucide-font/lucide.ttf"
    FONT_FILEHASH       "SHA1=02ae813dabbebc7329924783250826ea55c50005"

    INFO_FILEPATH       "lucide-font/lucide.css"
    INFO_FILEHASH       "SHA1=7c72d2e949e39b9c40af6d6b3f8861ef9930901b"
    INFO_OPTIONS        "prefix=.icon-"

    LICENSE_FILEPATH    "LICENSE"
    LICENSE_FILEHASH    "SHA1=2129ece9971e5274e05fea88f4eac6421fc97cd8"
)

# Material Design Icons # ----------------------------------------------------------------------------------------------

iconfonts_add_font_family(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Material Icons"

    BASE_URL            "https://github.com/google/material-design-icons@f2c8df1afe6f755459d30ebe3abeec7d4346ea85"
    RESOURCE_PREFIX     "/google/materialicons"
    VARIANT_PATTERN     "MaterialIcons-?(.*)"

    FONT_VARIANTS
        "font/MaterialIcons-Regular.ttf
        :SHA1=7e02c3f005532ff4d24148567c84089756a7848a
        :font/MaterialIcons-Regular.codepoints
        :SHA1=991b00a423a57d118ef77e02c6a6aa1d04075ba7"

        "font/MaterialIconsOutlined-Regular.otf
        :SHA1=31947883f5954ddf9277150b2710b43c9e5a3523
        :font/MaterialIconsOutlined-Regular.codepoints
        :SHA1=485a7944aa55072e2708e75bb3d6f2fe41144180"

        "font/MaterialIconsRound-Regular.otf
        :SHA1=ac81e06519a047577408ee050c9a1cc903ca18b7
        :font/MaterialIconsRound-Regular.codepoints
        :SHA1=9bee3c54dfd72eb629d9d7f7fc09f961e218513a"

        "font/MaterialIconsSharp-Regular.otf
        :SHA1=5f96ef05c8f362aa2af71f16a08be79e71eea890
        :font/MaterialIconsSharp-Regular.codepoints
        :SHA1=9bee3c54dfd72eb629d9d7f7fc09f961e218513a"

        "font/MaterialIconsTwoTone-Regular.otf
        :SHA1=485bdfbb088e29138b6e63cb1864e263e144bda3
        :font/MaterialIconsTwoTone-Regular.codepoints
        :SHA1=fc884c3a358cf50fc92188865d78c7750539172f"

    LICENSE_FILEPATH    "LICENSE"
    LICENSE_FILEHASH    "SHA1=1128f8f91104ba9ef98d37eea6523a888dcfa5de"
)

# Material Design Symbols # --------------------------------------------------------------------------------------------

iconfonts_add_font_family(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Material Symbols"

    BASE_URL            "https://github.com/google/material-design-icons@f2c8df1afe6f755459d30ebe3abeec7d4346ea85"
    RESOURCE_PREFIX     "/google/materialsymbols"
    VARIANT_PATTERN     "MaterialSymbols(.*)\\<5b>.*"

    FONT_VARIANTS
        "variablefont/MaterialSymbolsOutlined[FILL,GRAD,opsz,wght].ttf
        :SHA1=46d9407db528b60dc6eb0f361a2091d5e4ffc2de
        :variablefont/MaterialSymbolsOutlined[FILL,GRAD,opsz,wght].codepoints
        :SHA1=eefa4c7e1e7b9de9c3aa57d847c41f4140721612"

        "variablefont/MaterialSymbolsRounded[FILL,GRAD,opsz,wght].ttf
        :SHA1=5c5424074e63b408622849c1611db233c027bcec
        :variablefont/MaterialSymbolsRounded[FILL,GRAD,opsz,wght].codepoints
        :SHA1=387373d7ded8dd2e447683cc69c53df263bf50c0"

        "variablefont/MaterialSymbolsSharp[FILL,GRAD,opsz,wght].ttf
        :SHA1=08df0a65e70ac32e89ad4e342d995c2b5f6c3191
        :variablefont/MaterialSymbolsSharp[FILL,GRAD,opsz,wght].codepoints
        :SHA1=eefa4c7e1e7b9de9c3aa57d847c41f4140721612"

    LICENSE_FILEPATH    "LICENSE"
    LICENSE_FILEHASH    "SHA1=1128f8f91104ba9ef98d37eea6523a888dcfa5de"
)

# Nonicons # -----------------------------------------------------------------------------------------------------------

iconfonts_add_font(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Nonicons"

    BASE_URL            "https://github.com/yamatsum/nonicons@8454b3b6c3ceeee18b386b7882c5a071dcf0f3af" # 0.18
    RESOURCE_PREFIX     "/yamatsum/nonicons"

    FONT_FILEPATH       "dist/nonicons.ttf"
    FONT_FILEHASH       "SHA1=ca67b95a10a827cb40667d2148c82573bf5a66e9"

    INFO_FILEPATH       "dist/nonicons.css"
    INFO_FILEHASH       "SHA1=432e5f4d8459400575c2ff0b2b4b028beba0391c"
    INFO_OPTIONS        "prefix=.icon-"

    LICENSE_FILEPATH    "LICENSE"
    LICENSE_FILEHASH    "SHA1=c911d6d16e637fb82d942fd2b745915290fdf763"
)

# Open Iconic #-------------------------------------------------------------------------------------------------------

iconfonts_add_font(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Open Iconic"

    BASE_URL            "https://github.com/iconic/open-iconic@1.1.1"
    RESOURCE_PREFIX     "/useiconic.com/open"

    FONT_FILEPATH       "font/fonts/open-iconic.ttf"
    FONT_FILEHASH       "SHA1=48a78526a08ca236bfad0ceeabd62e1113bc563e"

    INFO_FILEPATH       "font/css/open-iconic-foundation.css"
    INFO_FILEHASH       "SHA1=d1ac664fd42d70a257e376a776cfa2e633b35ee1"
    INFO_OPTIONS        "prefix=.fi-"

    LICENSE_FILEPATH    "FONT-LICENSE"
    LICENSE_FILEHASH    "SHA1=65e04e6df717865a8e3c98f9a759cd57dfdc59f2"
)

# Phosphor Icons  #-----------------------------------------------------------------------------------------------------

iconfonts_add_font_family(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Phosphor Icons"

    BASE_URL            "https://github.com/phosphor-icons/web@v2.0.2"
    RESOURCE_PREFIX     "/phosphoricons/"
    VARIANT_PATTERN     "Phosphor-(.*)"


    FONT_VARIANTS
        "src/bold/Phosphor-Bold.ttf         :SHA1=cbf3d5d020897b6ffc5db79c812505a5728107c2
        :src/bold/style.css                 :SHA1=0d58eb42df6b393eec31970df600de7dd29a561a
        :prefix=.ph-bold.ph-"

        "src/duotone/Phosphor-Duotone.ttf   :SHA1=a7b422d2fda219f39e3614f6ff516308853446e0
        :src/duotone/style.css              :SHA1=5d7deb43ef3e84fec33fa982dde52c8a4925a7c2
        :prefix=.ph-duotone.ph-"

        "src/fill/Phosphor-Fill.ttf         :SHA1=5a495f84a74072f012ecdf94fa203d3ea537839a
        :src/fill/style.css                 :SHA1=3cdecbb69b7440d8cf4269281b7fa46de97a58b7
        :prefix=.ph-fill.ph-"

        "src/light/Phosphor-Light.ttf       :SHA1=6b75c272bfc652d40320bdaf1f605f0c6ce311db
        :src/light/style.css                :SHA1=f4e75b4788aef28137fedadd9c2fd899c9685935
        :prefix=.ph-light.ph-"

        "src/regular/Phosphor.ttf           :SHA1=9ee7bc0a8d63add7a70fae8484c2203d78854d41
        :src/regular/style.css              :SHA1=1fe0a90767cc39461e0ab89fdd238beaf409ecaf
        :prefix=.ph.ph- variant=Regular"

        "src/thin/Phosphor-Thin.ttf         :SHA1=d536613be6b2bd1b589e03118f763d2b293ef030
        :src/thin/style.css                 :SHA1=492d5af38f9c060f380048dbfca379ea419a5830
        :prefix=.ph-thin.ph-"

    LICENSE_FILEPATH    "LICENSE"
    LICENSE_FILEHASH    "SHA1=f934c0521fcc6763de2eb9874fe9169f620d14e2"
)

# Remix Icon # ---------------------------------------------------------------------------------------------------------

iconfonts_add_font(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Remix Icon"

    BASE_URL            "https://github.com/Remix-Design/RemixIcon@v4.3.0"
    RESOURCE_PREFIX     "/remixicon"

    FONT_FILEPATH       "fonts/remixicon.ttf"
    FONT_FILEHASH       "SHA1=8ce74c2ce086351d0ca323cb7b41e2b9cc27f0fd"

    INFO_FILEPATH       "fonts/remixicon.css"
    INFO_FILEHASH       "SHA1=935cbc124f0a013cf5973f016349855773ea1781"
    INFO_OPTIONS        "prefix=.ri-"

    LICENSE_FILEPATH    "License"
    LICENSE_FILEHASH    "SHA1=7df059597099bb7dcf25d2a9aedfaf4465f72d8d"
)

# Segoe Symbol Fonts # -------------------------------------------------------------------------------------------------

iconfonts_add_system_font_family(
    TARGET IconFonts FONT_FAMILY "Segoe"

    BASE_URL            "https://github.com/microsoft/microsoft-ui-xaml@winui3/release/1.5.5"
    RESOURCE_PREFIX     "/microsoft/segoe"

    FONT_VARIANTS       "Fluent Icons" "MDL2 Assets"

    INFO_FILEPATH       "dxaml/xcp/dxaml/idl/winrt/controls/microsoft.ui.xaml.controls.controls2.idl"
    INFO_FILEHASH       "SHA1=561abba882e79dd484ec8546d8944b49335b2a25"

    LICENSE_FILEPATH    "LICENSE" # that's only the symbol list; the font itself is propritary
    LICENSE_FILEHASH    "SHA1=818afb6a126b1a38966a74d168fe24c3ad205fc1"
)

# Simple Icons # -------------------------------------------------------------------------------------------------------

iconfonts_add_font(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Simple Icons"

    BASE_URL            "https://github.com/simple-icons/simple-icons-font@13.3.0"
    RESOURCE_PREFIX     "/simple-icons"

    ARCHIVE             "releases/simple-icons-font-13.3.0.zip"
    ARCHIVE_FILEHASH    "SHA1=82dafdc32820a995169e28623a9c4b13a89483a8"

    FONT_FILEPATH       "font/SimpleIcons.otf"
    FONT_FILEHASH       "SHA1=0ba81351e7f9d403b9a0a35679dc2e14430dd8f4"

    INFO_FILEPATH       "font/simple-icons.css"
    INFO_FILEHASH       "SHA1=b2b19992e670d39f10438e43fdac7c4e8e301768"
    INFO_OPTIONS        "prefix=.si-"

    LICENSE_FILEPATH    "LICENSE.md"
    LICENSE_FILEHASH    "SHA1=b0752f996462923387fcc8459955e6644bb83b00"
)

# Simple Line Icons #---------------------------------------------------------------------------------------------------

iconfonts_add_font(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Simple Line Icons"

    BASE_URL            "https://github.com/thesabbir/simple-line-icons@v2.5.4"
    RESOURCE_PREFIX     "/thesabbir/simple-line-icons"

    FONT_FILEPATH       "fonts/Simple-Line-Icons.ttf"
    FONT_FILEHASH       "SHA1=9ffb81a5a11112e292f2cc323e98486bad597599"

    INFO_FILEPATH       "css/simple-line-icons.css"
    INFO_FILEHASH       "SHA1=412b11f7c951fa64a7fbf26c83bd457b68a78740"
    INFO_OPTIONS        "prefix=.icon-"

    LICENSE_FILEPATH    "LICENSE.md"
    LICENSE_FILEHASH    "SHA1=0173099c6388d9b94b05d4fd6166781ec3110779"
)

# Tabler Icons # -------------------------------------------------------------------------------------------------------

iconfonts_add_font_family(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Tabler Icons"

    BASE_URL            "https://github.com/tabler/tabler-icons@v3.11.0"
    RESOURCE_PREFIX     "/tabler/icons"
    VARIANT_PATTERN     "tabler-icons-(.*)"

    ARCHIVE             "releases/tabler-icons-3.11.0.zip"
    ARCHIVE_FILEHASH    "SHA1=d524f53d87b6a32772e396b15ba38e399eba13db"

    FONT_VARIANTS
        "webfont/fonts/tabler-icons-filled.ttf  :SHA1=f588fb3fad466f13a26d2eb01112e85aa6993232
        :webfont/tabler-icons-filled.css        :SHA1=af40167a1a5447337ad361666dccfcac7e59f0f5
        :prefix=.ti-"

        "webfont/fonts/tabler-icons-outline.ttf :SHA1=1a5f3ed8f93616ecf6fe86069d4e8ce03c045870
        :webfont/tabler-icons-outline.css       :SHA1=0b2dece4cc60993d12f164a5e5de677895d3f12c
        :prefix=.ti-"

        "webfont/fonts/tabler-icons.ttf         :SHA1=44a85fe3389308ddebde64ff71ef1e1f30b86465
        :webfont/tabler-icons.css               :SHA1=c6e4db3e410f590e3433efe9103f0540b2fd327e
        :prefix=.ti- variant=regular"

    LICENSE_FILEPATH    "LICENSE"
    LICENSE_FILEHASH    "SHA1=a19f9f6ec8b9157f66ed407972247e277eb08b01"
)

# Themify Icons #-------------------------------------------------------------------------------------------------------

iconfonts_add_font(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Themify Icons"

    BASE_URL            "https://github.com/lykmapipo/themify-icons@v0.1.2"
    RESOURCE_PREFIX     "/themify/icons"

    FONT_FILEPATH       "fonts/themify.ttf"
    FONT_FILEHASH       "SHA1=5bb1fe69452a484566a81076af7567728fe7e43b"

    INFO_FILEPATH       "css/themify-icons.css"
    INFO_FILEHASH       "SHA1=d1145d3b977587a7ed3a7737812fdcf2a7968b23"
    INFO_OPTIONS        "prefix=.ti- "

    LICENSE_FILEPATH    "README.md"
    LICENSE_FILEHASH    "SHA1=9dcee320f5cc58586d30caed748cded8c8245f91"
)

# Typicons # -----------------------------------------------------------------------------------------------------------

iconfonts_add_font(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Typicons"

    BASE_URL            "https://github.com/stephenhutchings/typicons.font@v2.1.2"
    RESOURCE_PREFIX     "/stephenhutchings/typicons"

    FONT_FILEPATH       "src/font/typicons.ttf"
    FONT_FILEHASH       "SHA1=58e90384707051ca3105425d1876d96ec8d88055"

    INFO_FILEPATH       "src/font/typicons.json"
    INFO_FILEHASH       "SHA1=e12ac17d701964edbb9b74626b475dfdc22e04ab"

    LICENSE_FILEPATH    "src/font/LICENCE.md"
    LICENSE_FILEHASH    "SHA1=cc6cdfb9602a44761f5cde1db7d2e12a24fc3cef"
)

# VS Code Icons #-----------------------------------------------------------------------------------------------------

iconfonts_add_font(
    TARGET IconFonts OPTIONAL FONT_FAMILY "VS Code Icons"

    BASE_URL            "https://github.com/microsoft/vscode-codicons@0.0.36"
    RESOURCE_PREFIX     "/microsoft/codicons"

    FONT_FILEPATH       "releases/codicon.ttf"
    FONT_FILEHASH       "SHA1=a98c43b6b673da90a79227ede5c3c72f6b32d645"

    INFO_FILEPATH       "src/template/mapping.json"
    INFO_FILEHASH       "SHA1=8d2f85b9fd54df58312a9264b1ddad5f7d11d274"

    LICENSE_FILEPATH    "LICENSE"
    LICENSE_FILEHASH    "SHA1=632ece91588956ff43fdca7176e63fc66850c63f"
)

# Weather Icons #-------------------------------------------------------------------------------------------------------

iconfonts_add_font(
    TARGET IconFonts OPTIONAL FONT_FAMILY "Weather Icons"

    BASE_URL            "https://github.com/erikflowers/weather-icons@2.0.10"
    RESOURCE_PREFIX     "/erikflowers/weather-icons"

    FONT_FILEPATH       "font/weathericons-regular-webfont.ttf"
    FONT_FILEHASH       "SHA1=9916e3dbc3990e3073cf5a2cb4c2e958273b7532"

    INFO_FILEPATH       "css/weather-icons.css"
    INFO_FILEHASH       "SHA1=0c2aa167f186b6d9561243dad2fce3608ef9870b"
    INFO_OPTIONS        "prefix=.wi-"

    LICENSE_FILEPATH    "README.md"
    LICENSE_FILEHASH    "SHA1=b0d6005eb93a6641613f3bb9ee71d540a1d3f809"
)

#
# The following fonts from https://react-icons.github.io/react-icons/ are not supported yet:
#
# - https://github.com/Klarr-Agency/Circum-icons    - SVG/Javascript only it seems
# - https://github.com/icons8/flat-color-icons      - SVG/PDF only it seems
# - https://github.com/game-icons/icons             - could not find icon font
# - https://github.com/primer/octicons              - fonts got deprecated -> yamatsum/nonicons
# - https://github.com/grommet/grommet-icons        - could not find icon font
# - https://github.com/tailwindlabs/heroicons       - could not find icon font; contains "Heroicons" and "Heroicons 2"
# - https://github.com/ionic-team/ionicons          - could not find icon font; contains "Ionicons 4" and "Ionicons 5"
# - https://github.com/radix-ui/icons               - could not find icon font
#
