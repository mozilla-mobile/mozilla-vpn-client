# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_library(forms STATIC)

set_target_properties(forms PROPERTIES FOLDER "Libs")
target_link_libraries(forms PRIVATE 
    Qt6::Quick
    Qt6::Gui
    Qt6::Widgets
)

qt_add_qml_module(forms
    VERSION 0.1
    URI components.forms 
    RESOURCE_PREFIX /nebula
    QML_FILES
        VPNContextualAlerts.qml
        VPNInputStates.qml
        VPNTextField.qml
        VPNInputBackground.qml
        VPNTextArea.qml
        VPNPasswordInput.qml
        VPNRadioButton.qml
        VPNCursorDelegate.qml
        VPNSearchBar.qml
        VPNContextualAlert.qml
        VPNComboBox.qml
    IMPORTS
        QtQuick/2
        QtQuick.Layouts/2
        components/auto
        QtQuick.Controls/2
)