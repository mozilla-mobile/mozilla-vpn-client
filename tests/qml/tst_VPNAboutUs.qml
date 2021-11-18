import QtQuick 2.3
import QtTest 1.0

import components 0.1

Item {
    width: 600;
    height: 800;

    VPNAboutUs {
        id: vpnAboutUsTest
    }

    TestCase {
        name: "VPNAboutUsTests"
        when: windowShown

        function test_releaseVersion() {
            // These values were set in qml.pro
            const expected = "QMLTest_AppVersion (QMLTest_BuildID)"
            const actual = vpnAboutUsTest.releaseVersionText
            verify(expected === actual, `releaseVersion was ${actual} not ${expected}.`)
        }
    }
}
