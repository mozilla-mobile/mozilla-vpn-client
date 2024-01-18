# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_qml_module(inAppAuth
    VERSION 0.1
    URI components.inAppAuth
    STATIC
    RESOURCE_PREFIX /nebula
    QML_FILES
        MZInAppAuthenticationBase.qml
        MZInAppAuthenticationErrorPopup.qml
        MZInAppAuthenticationInputs.qml
        MZInAppAuthenticationLegalDisclaimer.qml
        MZInAppAuthenticationPasswordCondition.qml
        MZInAppAuthenticationThirdParty.qml
)
set_target_properties(inAppAuth PROPERTIES FOLDER "Libs")
