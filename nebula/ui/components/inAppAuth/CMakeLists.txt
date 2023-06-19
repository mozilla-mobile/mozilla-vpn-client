# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_library(inAppAuth STATIC)

set_target_properties(inAppAuth PROPERTIES FOLDER "Libs")
target_link_libraries(inAppAuth PRIVATE 
    Qt6::Quick
    Qt6::Gui
    Qt6::Widgets
)

qt_add_qml_module(inAppAuth
    VERSION 0.1
    URI components.inAppAuth 
    RESOURCE_PREFIX /nebula
    QML_FILES
        MZInAppAuthenticationBase.qml
        MZInAppAuthenticationErrorPopup.qml
        MZInAppAuthenticationInputs.qml
        MZInAppAuthenticationPasswordCondition.qml
        MZInAppAuthenticationLegalDisclaimer.qml
)