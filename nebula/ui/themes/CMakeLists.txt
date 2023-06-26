# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_qml_module(themes
    VERSION 0.1
    URI themes 
    STATIC
    RESOURCE_PREFIX /nebula
    RESOURCES
        main/theme.js
        main/colors.js
)
