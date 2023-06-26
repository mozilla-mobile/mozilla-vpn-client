# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_qml_module(compat
    VERSION 0.1
    URI compat 
    RESOURCE_PREFIX /nebula
    QML_FILES
        qt6/MZColorOverlay.qml
        qt6/MZLinearGradient.qml
        qt6/MZRadialGradient.qml
        qt6/MZRectangularGlow.qml
        qt6/MZAnimatedRingsShader.qml
        qt6/MZOpacityMask.qml
        qt6/MZDropShadow.qml
)
