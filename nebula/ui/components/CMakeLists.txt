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
         MZButtonBase.qml
         MZButtonLoader.qml
         MZButton.qml
         MZCallout.qml
         MZCancelButton.qml
         MZCard.qml
         MZCheckBox.qml
         MZCheckBoxRow.qml
         MZCheckBoxRowWithoutDivider.qml
         MZChevron.qml
         MZClickableRow.qml
         MZCollapsibleCard.qml
         MZDropShadowWithStates.qml
         MZExternalLinkListItem.qml
         MZFlickable.qml
         MZFocusBorder.qml
         MZFocusOutline.qml
         MZFooterLink.qml
         MZGreyLink.qml
         MZGuideCard.qml
         MZHeaderLink.qml
         MZHeadline.qml
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
         MZToggleCard.qml
         MZToolTip.qml
         MZTutorialCard.qml
         MZUIStates.qml
         MZVerticalSpacer.qml
         MZSegmentedToggle.qml
         MZSegmentedNavigation.qml
         MZUserProfile.qml
         MZTutorialPopups.qml
         MZTutorialList.qml
         MZGuideList.qml
         MZTipsAndTricksSection.qml
         MZScreenBase.qml
         MZViewBase.qml
         MZSwipeDelegate.qml
         MZSwipeAction.qml
         MZBadge.qml
         MZComposerView.qml
         MZPasteButton.qml
         MZInformationCard.qml
         MZStepProgressBar.qml
         MZStepProgressBarDelegate.qml
         MZStepNavigation.qml
         navigationBar/MZBottomNavigationBar.qml
         navigationBar/MZBottomNavigationBarButton.qml
         navigator/MZNavigatorLoader.qml
         navigator/navigatorLoaderInternal.qml
)

add_subdirectory(forms)
add_subdirectory(inAppAuth)


set_target_properties(components PROPERTIES FOLDER "Libs")
target_link_libraries(components PRIVATE 
    forms
    inAppAuth
)
