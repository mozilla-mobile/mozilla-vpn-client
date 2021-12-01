/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

export class GenericDispatcher {
  constructor () {
    this.listeners = []
  }

  // Add an listener to $event
  on (type, callback) {
    this.listeners.push((event) => {
      if (event.type === type) {
        callback(event)
      }
    })
  }

  onAny (callback) {
    this.listeners.push(callback)
  }

  emit (event) {
    this.listeners.forEach(l => {
      try {
        l(event)
      } catch (error) {
        console.error('Exception Running Eventlistener')
        console.log(error)
      }
    })
  }
}
