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
            const expected = "testme"
            const actual = vpnAboutUsTest.releaseVersionText
            verify(expected === actual, `releaseVersion was ${actual} not ${expected}.`)
        }
    }
}