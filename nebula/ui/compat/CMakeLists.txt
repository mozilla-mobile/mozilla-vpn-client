# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_library(compat STATIC)

target_link_libraries(compat PRIVATE 
    Qt6::Quick
    Qt6::Gui
)

qt_add_qml_module(compat
    VERSION 0.1
    URI compat 
    OUTPUT_DIRECTORY compat
    RESOURCE_PREFIX /nebula
    QML_FILES
        qt6/VPNColorOverlay.qml
        qt6/VPNLinearGradient.qml
        qt6/VPNRadialGradient.qml
        qt6/VPNRectangularGlow.qml
        qt6/VPNAnimatedRingsShader.qml
        qt6/VPNOpacityMask.qml
        qt6/VPNDropShadow.qml
)