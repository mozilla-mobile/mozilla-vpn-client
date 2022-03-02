/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0

import Mozilla.VPN 1.0

VPNPopup {
    id: root

    anchors.centerIn: parent
    maxWidth: VPNTheme.theme.desktopAppWidth
    _popupContent: VPNFeatureTour {
        id: featureTour

        slidesModel: VPNWhatsNewModel
        onFinished: {
            root.close();
        }
        onStarted: {
            VPNWhatsNewModel.markFeaturesAsSeen();
        }
    }

    function openTour() {
        featureTour.resetTour();
        root.open();

        VPNSettings.featuresTourShown = true;
    }

    function startTour() {
        featureTour.skipStart();
        root.open();
    }
}
