function scrollToComponent(root) {
    let parentComponent = root.parent
    while(parentComponent && parentComponent !== "undefined") {
        if(typeof(parentComponent.ensureVisible) !== "undefined") {
            return parentComponent.ensureVisible(root)
        }
        parentComponent = parentComponent.parent
    }
}
