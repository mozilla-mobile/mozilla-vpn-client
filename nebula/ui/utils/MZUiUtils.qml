pragma Singleton

import QtQuick 2.15

QtObject {
    function scrollToComponent(root) {
        let parentComponent = root.parent
        while(parentComponent && parentComponent !== "undefined") {
            if(typeof(parentComponent.ensureVisible) !== "undefined") {
                return parentComponent.ensureVisible(root)
            }
            parentComponent = parentComponent.parent
        }
    }
}
