# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_library(components STATIC)


add_subdirectory(forms)
add_subdirectory(inAppAuth)

set_target_properties(components PROPERTIES FOLDER "Libs")
target_link_libraries(components PRIVATE 
    Qt6::Quick
    Qt6::Gui
    Qt6::Widgets
    formsplugin
    inAppAuthplugin
    lottieplugin
)

qt_add_qml_module(components
    VERSION 0.1
    URI components
    RESOURCE_PREFIX /nebula
    IMPORTS
        QtQuick/auto
        QtQuick.Layouts/auto
        QtQuick.Controls
        lottie/0.1
        compat/0.1
    QML_FILES
        VPNViewBase.qml
        VPNLottieAnimation.qml
        VPNTabNavigation.qml
        VPNToggleCard.qml
        VPNButton.qml
        VPNToggle.qml
        VPNIcon.qml
        VPNIconAndLabel.qml
        VPNSettingsToggle.qml
        VPNGraphLegendMarker.qml
        VPNIPAddress.qml
        VPNPanel.qml
        VPNScreenBase.qml
        VPNFocusOutline.qml
        VPNCard.qml
        VPNCollapsibleCard.qml
        VPNCheckBoxAlert.qml
        VPNComposerView.qml
        VPNSegmentedToggle.qml
        VPNSubscriptionOption.qml
        VPNServerListToggle.qml
        VPNTutorialList.qml
        VPNConnectionInfoScreen.qml
        VPNMouseArea.qml
        VPNLogoLoader.qml
        VPNRecentConnections.qml
        VPNStackView.qml
        VPNCheckmarkList.qml
        VPNPopup.qml
        VPNMainImage.qml
        VPNTimer.qml
        VPNConnectionInfoItem.qml
        VPNConnectionInfoContent.qml
        VPNHeaderLink.qml
        VPNHeadline.qml
        VPNConnectionInfoError.qml
        VPNTutorialCard.qml
        VPNBadge.qml
        VPNTipsAndTricksSection.qml
        VPNConnectionStability.qml
        VPNRadioDelegate.qml
        VPNSignOut.qml
        VPNList.qml
        VPNExternalLinkListItem.qml
        VPNCheckBox.qml
        VPNIconButton.qml
        VPNCallout.qml
        VPNButtonLoader.qml
        VPNLogsButton.qml
        VPNSwipeAction.qml
        VPNFooterMargin.qml
        VPNNavigationBarClearance.qml
        VPNAvatar.qml
        VPNBoldInterLabel.qml
        VPNInterLabel.qml
        VPNGuideList.qml
        VPNVerticalSpacer.qml
        VPNDropShadowWithStates.qml
        VPNPasteButton.qml
        VPNControllerView.qml
        VPNMenu.qml
        VPNGreyLink.qml
        VPNUIStates.qml
        VPNFooterLink.qml
        VPNClickableRow.qml
        VPNSimplePopup.qml
        VPNAnimatedRings.qml
        VPNServerUnavailablePopup.qml
        VPNLinkButton.qml
        VPNPill.qml
        VPNSwipeDelegate.qml
        VPNRadioButtonLabel.qml
        VPNTutorialPopups.qml
        VPNServerList.qml
        VPNServerLatencyIndicator.qml
        VPNButtonBase.qml
        VPNCheckBoxRow.qml
        VPNControllerNav.qml
        VPNSegmentedNavigation.qml
        VPNPaymentMethod.qml
        VPNSubtitle.qml
        VPNBoldLabel.qml
        VPNChevron.qml
        VPNTextBlock.qml
        VPNFocusBorder.qml
        VPNUserProfile.qml
        VPNFlickable.qml
        VPNServerCountry.qml
        VPNIPInfoPanel.qml
        VPNMetropolisLabel.qml
        VPNAlert.qml
        VPNCancelButton.qml
        VPNPopupButton.qml
        VPNServerLabel.qml
        VPNSystemAlert.qml
        VPNLoader.qml
        VPNSettingsItem.qml
        VPNMobileStatusBarModifier.qml
        VPNGuideCard.qml
        VPNCheckmark.qml
        VPNSemiMonoLabel.qml
        VPNToolTip.qml
        VPNLightLabel.qml
)
