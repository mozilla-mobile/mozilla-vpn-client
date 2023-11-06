function scrollToComponent(root) {
    let parentComponent = root.parent
    while(parentComponent && parentComponent !== "undefined") {
        if(typeof(parentComponent.ensureVisible) !== "undefined") {
            return parentComponent.ensureVisible(root)
        }
        parentComponent = parentComponent.parent
    }
}

function isMobile() {
    return Qt.platform.os === "android" || Qt.platform.os === "ios"
}

function isLargePhone() {
    return window.height > MZTheme.theme.largePhoneHeight
}
