# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_qml_module(components
    VERSION 0.1
    URI components
    STATIC
    RESOURCE_PREFIX nebula
    QML_FILES
         MZAlert.qml
         MZAnimatedRings.qml
         MZAvatar.qml
         MZBoldInterLabel.qml
         MZBoldLabel.qml
         MZBottomSheet.qml
         MZButtonBase.qml
         MZButtonLoader.qml
         MZButton.qml
         MZCallout.qml
         MZCancelButton.qml
         MZCheckBox.qml
         MZCheckBoxRow.qml
         MZChevron.qml
         MZClickableRow.qml
         MZComposerView.qml
         MZDropShadowWithStates.qml
         MZExternalLinkListItem.qml
         MZFlickable.qml
         MZFocusBorder.qml
         MZFocusOutline.qml
         MZFooterLink.qml
         MZTextLink.qml
         MZHeaderLink.qml
         MZHeadline.qml
         MZHelpSheet.qml
         MZLinkRow.qml
         MZLottieAnimation.qml
         MZIconAndLabel.qml
         MZIconButton.qml
         MZIcon.qml
         MZInterLabel.qml
         MZLightLabel.qml
         MZLinkButton.qml
         MZList.qml
         MZLoader.qml
         MZLogsButton.qml
         MZMenu.qml
         MZMetropolisLabel.qml
         MZMobileStatusBarModifier.qml
         MZMouseArea.qml
         MZPanel.qml
         MZPaymentMethod.qml
         MZPill.qml
         MZPopupButton.qml
         MZPopup.qml
         MZRadioButtonLabel.qml
         MZRadioDelegate.qml
         MZSettingsItem.qml
         MZSettingsToggle.qml
         MZSignOut.qml
         MZSimplePopup.qml
         MZStackView.qml
         MZSubscriptionOption.qml
         MZSubtitle.qml
         MZSystemAlert.qml
         MZTabNavigation.qml
         MZTextBlock.qml
         MZTimer.qml
         MZToolTip.qml
         MZUIStates.qml
         MZVerticalSpacer.qml
         MZSegmentedToggle.qml
         MZSegmentedNavigation.qml
         MZUserProfile.qml
         MZScreenBase.qml
         MZViewBase.qml
         MZSwipeDelegate.qml
         MZSwipeAction.qml
         MZBadge.qml
         MZPasteButton.qml
         MZInformationCard.qml
         MZStepProgressBar.qml
         MZStepProgressBarDelegate.qml
         MZStepNavigation.qml
         MZToggleRow.qml
         navigationBar/MZBottomNavigationBar.qml
         navigationBar/MZBottomNavigationBarButton.qml
         navigator/MZNavigatorLoader.qml
         navigator/navigatorLoaderInternal.qml
    SOURCES
        filterproxymodel.cpp
        filterproxymodel.h
)

add_subdirectory(forms)
add_subdirectory(inAppAuth)


set_target_properties(components PROPERTIES FOLDER "Libs")
target_link_libraries(components PRIVATE
    forms
    formsplugin
    inAppAuth
    inAppAuthplugin
)
