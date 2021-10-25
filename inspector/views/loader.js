import './view-shell'
import './logs'
import './view-network'
import './view-ui'

// Imports the Module that defines <tag></tag>
export async function loadView (tag) {
  /* not useful until we can have code-splitting -> see index.html
    switch(tag){
        case "view-shell":
            return import("./view-shell.js")
        case "view-logs":
            return import("./logs.js")
        case "view-network":
            return import("./view-network.js")
        case "view-ui":
            return import("./view-ui.js")
        case "view-todo":
            return import("./view-todo.js")
    } */
}
