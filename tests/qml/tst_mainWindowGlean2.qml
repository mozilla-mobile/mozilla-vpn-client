import QtQuick 2.3
import QtTest 1.0

import TestHelper 1.0
import Mozilla.VPN 1.0
import org.mozilla.Glean 0.24
import ui 0.1

/*
 * Some tests need to be isolated from others
 * because things like const properties are set once
 * so we need to split our tests in to multiple files.
 */

Item {
    Main {
        id: mainTestGlean
    }

    TestCase {
        name: "MainTestsGlean2"

        property var spyConfig

        function initTestCase() {
            function mockGleanInitialize(applicationId, uploadEnabled, config) {
                spyConfig = config;
            }
            Glean.initialize = mockGleanInitialize;
        }

        function test_onInitializeGleanSetsChannelBasedOnStagingModeFalse() {
            TestHelper.triggerInitializeGlean()
            compare(spyConfig.channel, "production")
        }
    }
}