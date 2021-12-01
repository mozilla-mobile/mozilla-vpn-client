/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { Client } from './client'
import { GenericDispatcher } from './genericdispatcher'

class _UIObserver extends GenericDispatcher {
  constructor () {
    super()
    Client.on('qml_tree', (r) => this.onIncomingViewTree(r))
    Client.on('view_tree', (r) => this.onIncomingViewTree(r))
    Client.on('screen_capture', (r) => this.onIncomingScreen(r))

    this.counter = 0
    this.requests = []
  }

  // Adds "parent field for each element to make stuff more ez"
  fixTree (uiTree) {
    uiTree.map(element => this.fixElement(element))

    return uiTree
  }

  fixElement (element, parent) {
    element.parent = parent
    element.__collapsed__ = false
    // X,Y,Z are always relative to the parent element, so
    // let's already note the totalX realative to display 0/0
    // so drawing highlights is more ez
    if (parent) {
      element.totalX = element.x + parent.totalX
      element.totalY = element.y + parent.totalY
    } else {
      element.totalX = element.x
      element.totalY = element.y
    }
    if (element.subItems) {
      element.subItems.forEach(i => this.fixElement(i, element))
    }
  }

  onIncomingViewTree (message) {
    this.tree = this.fixTree(message.tree)

    this.emit({ type: 'tree', list: this.tree })
  }

  onIncomingScreen (screen) {
    console.log('Incoming screenshot')
    this.screen = screen.value
    this.emit({ type: 'screenshot', screen })
  }

  refresh () {
    Client.sendCommand('view_tree')
    Client.sendCommand('screen_capture')
  }

  getScreen () {
    Client.sendCommand('screen_capture')
  }
}

export const UIObserver = new _UIObserver()
