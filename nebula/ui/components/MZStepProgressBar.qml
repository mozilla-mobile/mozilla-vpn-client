/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0

Item {
    id: progressBar

    property var model
    property int activeIndex: 0

    signal buttonClicked(index: int)

    implicitHeight: delegateLayout.implicitHeight

    Rectangle {
        id: bar

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: (MZTheme.theme.progressBarDelegateHeightWidth / 2) - (bar.height / 2)

        height: 4

        z: parent.z - 1
        color: MZTheme.colors.grey30

        Rectangle {
            implicitHeight: parent.height
            implicitWidth: (delegateLayout.spacing * progressBar.activeIndex) + (MZTheme.theme.progressBarDelegateHeightWidth * activeIndex )

            color: MZTheme.colors.purple10

            Behavior on implicitWidth {
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    Row {
        id: delegateLayout

        anchors.left: parent.left
        anchors.right: parent.right

        spacing: (parent.width - (MZTheme.theme.progressBarDelegateHeightWidth * progressBar.model.count)) / (progressBar.model.count - 1)

        Repeater {
            model: progressBar.model
            delegate:  MZStepProgressBarDelegate {
                id: progressBarDelegate
                iconSource: model.iconSource
                labelText: MZI18n[model.labelText]
                labelWidth: progressBar.width / progressBar.model.count
                currentState: {
                    if (index === activeIndex) {
                        MZStepProgressBarDelegate.State.Active
                    }
                    else if (index < activeIndex) {
                        MZStepProgressBarDelegate.State.Complete
                    }
                    else {
                        MZStepProgressBarDelegate.State.Incomplete
                    }
                }

                onClicked: {
                    activeIndex = index
                    buttonClicked(index)
                }

                accessibleIgnored: currentState === MZStepProgressBarDelegate.State.Incomplete
                accessibleName: currentState === MZStepProgressBarDelegate.State.Complete ? MZI18n.OnboardingProgressBarAccessibilityStepComplete.arg(labelText).arg(index + 1).arg(progressBar.model.count)
                                                                                          : MZI18n.OnboardingProgressBarAccessibilityStepCurrent.arg(labelText).arg(index + 1).arg(progressBar.model.count)
            }
        }
    }
}

