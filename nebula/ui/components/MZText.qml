import QtQuick 2.5
import Mozilla.Shared 1.0

Text {
    //Setting the type property should do most of the styling for you
    property int type: MZText.Type.Body

    enum Type {
        Header,
        HeaderBold,
        LargeHeader,
        LargeHeaderBold,
        Body,
        BodyBold,
        BodyUnderline,
        Byline,
        BylineBold,
        BylineUnderline,
        Caption,
        CaptionBold
    }

    font.family: {
        switch (type) {
        case MZText.Type.Header:
        case MZText.Type.LargeHeader:
            return MZTheme.theme.fontMetropolisFamily
        case MZText.Type.HeaderBold:
        case MZText.Type.LargeHeaderBold:
            return MZTheme.theme.fontMetropolisSemiBoldFamily
        case MZText.Type.BodyBold:
        case MZText.Type.BylineBold:
        case MZText.Type.CaptionBold:
            return MZTheme.theme.fontInterSemiBoldFamily
        case MZText.Type.Body:
        case MZText.Type.BodyUnderline:
        case MZText.Type.Byline:
        case MZText.Type.BylineUnderline:
        case MZText.Type.Caption:
        default:
            return MZTheme.theme.fontInterFamily
        }
    }

    font.pixelSize: {
        switch (type) {
        case MZText.Type.LargeHeader:
        case MZText.Type.LargeHeaderBold:
            return MZTheme.theme.fontSizeLarge
        case MZText.Type.Byline:
        case MZText.Type.BylineBold:
        case MZText.Type.BylineUnderline:
            return MZTheme.theme.fontSizeSmall
        case MZText.Type.Caption:
        case MZText.Type.CaptionBold:
            return MZTheme.theme.fontSizeSmallest
        case MZText.Type.Header:
        case MZText.Type.HeaderBold:
        case MZText.Type.Body:
        case MZText.Type.BodyBold:
        case MZText.Type.BodyUnderline:
        default:
            return MZTheme.theme.fontSize
        }
    }

    lineHeight: {
        switch (type) {
        case MZText.Type.LargeHeader:
        case MZText.Type.LargeHeaderBold:
            return MZTheme.theme.lineHeightLargeHeader
        case MZText.Type.Header:
        case MZText.Type.HeaderBold:
            return MZTheme.theme.lineHeightHeader
        case MZText.Type.Byline:
        case MZText.Type.BylineBold:
        case MZText.Type.BylineUnderline:
            return MZTheme.theme.lineHeightByline
        case MZText.Type.Caption:
        case MZText.Type.CaptionBold:
            return MZTheme.theme.lineHeightCaption
        case MZText.Type.Body:
        case MZText.Type.BodyBold:
        case MZText.Type.BodyUnderline:
        default:
            return MZTheme.theme.lineHeightBody
        }
    }

    color: MZTheme.theme.fontColorDark
    lineHeightMode: Text.FixedHeight
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    wrapMode: Text.WordWrap

    Accessible.role: Accessible.StaticText
    Accessible.name: text
    Accessible.ignored: !visible
}
