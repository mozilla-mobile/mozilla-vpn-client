import QtQuick 2.3
import QtTest 1.0

TestCase {
    name: "MathTests2"

    function test_math2() {
        compare(2 + 2, 4, "2 + 2 = 4")
    }

    function test_fail2() {
        compare(2 + 2, 5, "2 + 2 = 5")
    }
}