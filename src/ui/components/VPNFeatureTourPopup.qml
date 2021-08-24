import QtQuick 2.0
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

VPNPopup {
    id: root

    topMargin: Theme.windowMargin * 2
    contentItem: VPNFeatureTour {
        id: featureTour

        slidesModel: newFeaturesModel
        onFinished: {
            root.close();
        }
        onStarted: {
            VPNSettings.newFeaturesSeen = true;
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
