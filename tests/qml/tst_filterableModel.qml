/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.3
import QtTest 1.0

import Mozilla.VPN 1.0

Item {
    width: 600
    height: 800

    ListModel {
        id: fruitModel

        ListElement {
            name: "Apple"
            cost: 2.45
        }
        ListElement {
            name: "Orange"
            cost: 3.25
        }
        ListElement {
            name: "Banana"
            cost: 1.95
        }
    }

    MZFilterProxyModel {
        id: testModel_filterAndSort
        source: fruitModel
        filterCallback: (fruit) => fruit.cost < 3
        sortCallback: (a, b) => a.cost < b.cost
    }

    MZFilterProxyModel {
        id: testModel_filter
        source: fruitModel
        filterCallback: (fruit) => fruit.cost < 3
    }

    MZFilterProxyModel {
        id: testModel_sort
        source: fruitModel
        sortCallback: (a, b) => a.cost < b.cost
    }

    TestCase {
        name: "MZFilterPRoxyModel"
        when: windowShown

        function test_source() {
            compare(fruitModel.rowCount(), 3, "FruitModel count");
            compare(fruitModel.get(0).name, "Apple");
            compare(fruitModel.get(1).name, "Orange");
            compare(fruitModel.get(2).name, "Banana");
        }

        function test_filterAndSort() {
            compare(testModel_filterAndSort.rowCount(), 2, "TestModel count");
            compare(testModel_filterAndSort.source, fruitModel, "The filter.source");
            compare(testModel_filterAndSort.get(0).name, "Banana");
            compare(testModel_filterAndSort.get(1).name, "Apple");
        }

        function test_filter() {
            compare(testModel_filter.rowCount(), 2, "TestModel count");
            compare(testModel_filter.source, fruitModel, "The filter.source");
            compare(testModel_filter.get(0).name, "Apple");
            compare(testModel_filter.get(1).name, "Banana");
        }

        function test_sort() {
            compare(testModel_sort.rowCount(), 3, "TestModel count");
            compare(testModel_sort.source, fruitModel, "The filter.source");
            compare(testModel_sort.get(0).name, "Banana");
            compare(testModel_sort.get(1).name, "Apple");
            compare(testModel_sort.get(2).name, "Orange");
        }
    }
}

