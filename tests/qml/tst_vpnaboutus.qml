import QtQuick 2.3
import QtTest 1.0

import components 0.1

// Canvas for the unit tests
Item {
    width: 600;
    height: 800;

    VPNAboutUs {
        id: vpnAboutUsTest
    }

    TestCase {
        name: "AboutUsTests"
        when: windowShown

        function test_releaseVersion() {
            verify(vpnAboutUsTest.releaseVersion === "testme", "test failed - booo" + vpnAboutUsTest.releaseVersion)
        }
    }
}