/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1

MZViewBase {
    _menuTitle: "UI Testing"
    _viewContentData: ColumnLayout {
        spacing: MZTheme.theme.windowMargin
        Layout.fillWidth: true

        MZSettingsItem {
            settingTitle: "MZStepProgressBar"
            imageRightSrc: "qrc:/nebula/resources/chevron.svg"
            imageRightMirror: MZLocalizer.isRightToLeft
            onClicked: getHelpStackView.push(progressBar)
            Layout.leftMargin: MZTheme.theme.windowMargin / 2
            Layout.rightMargin: MZTheme.theme.windowMargin / 2
        }
    }

    Component {
        id: progressBar
        MZViewBase {
            _menuTitle: "MZStepProgressBar"
            _viewContentData: ColumnLayout {
                Layout.leftMargin: 36
                Layout.rightMargin: 36

                spacing: 32

                MZStepProgressBar {
                    id: stepProgress
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.maximumWidth: 500

                    model: ListModel {
                        id: stepProgressModel

                        //Hacky workaround since QML list models can't take in scripts (the MZI18n localization script) as property values
                        //This circumvents us having to make a C++ class for the progress bar delegates and a model for the progress bar as a whole
                        //(i.e. navigationbarmodel.h/cpp and navigationbarbutton.h/cpp)
                        //reference: https://stackoverflow.com/a/33161093
                        property bool completed: false
                        property string firstString: MZI18n.OnboardingProgressBarDataUse
                        property string secondString: MZI18n.OnboardingProgressBarMorePrivacy
                        property string thirdString: MZI18n.OnboardingProgressBarAddDevices
                        property string fourthString: MZI18n.OnboardingProgressBarGetStarted

                        onFirstStringChanged: {
                            if(completed) setProperty(0, "labelText", firstString);
                        }

                        onSecondStringChanged: {
                            if(completed) setProperty(0, "labelText", firstString);
                        }

                        onThirdStringChanged: {
                            if(completed) setProperty(0, "labelText", firstString);
                        }

                        onFourthStringChanged: {
                            if(completed) setProperty(0, "labelText", firstString);
                        }

                        Component.onCompleted: {
                            append({source: "qrc:/nebula/resources/lock.svg", labelText: firstString});
                            append({source: "qrc:/ui/resources/settings/privacy.svg", labelText: secondString});
                            append({source: "qrc:/ui/resources/devices.svg", labelText: thirdString});
                            append({source: "qrc:/nebula/resources/startup.svg", labelText: fourthString});
                            completed = true;
                        }

                    }
                }

                RowLayout {
                    MZButton {
                        Layout.fillWidth: true

                        text: "Back"
                        onClicked: {
                            if(stepProgress.activeIndex > 0) stepProgress.activeIndex = stepProgress.activeIndex - 1
                        }
                    }

                    MZButton {
                        Layout.fillWidth: true

                        text: "Next"
                        onClicked: {
                            if(stepProgress.activeIndex < stepProgressModel.count - 1)  stepProgress.activeIndex = stepProgress.activeIndex + 1
                        }
                    }
                }
            }
        }
    }
}

