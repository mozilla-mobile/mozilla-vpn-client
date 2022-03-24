/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


// This Type is essentially the json we get from the inspector
export interface IEvent{
  type:string,
  value?:any,
  clicked?:boolean,
  error?:string,

  // For Type("notification")
  title?:string,
  message?:string,

  // For Type("network")
  request?:object,
  response?:object,
  // For Type("qml_tree")
  tree?:object,

  entry?:any,
  list?:any,
  screen?:any,
}
type AnyEventHandler = (event: IEvent) => any;

export class GenericDispatcher extends EventTarget {
  listeners:Array<Function>
  constructor () {
    super();
    this.listeners = []
  }

  // Add an listener to $event
  on (type:string, callback:any) {
    this.addEventListener(type,callback)
  }

  onAny(callback:AnyEventHandler) {
    this.listeners.push(callback)
  }

  emit (event: IEvent) {
    const custom_event = new CustomEvent(event.type, {
      detail: event
    });
    this.listeners.forEach(l => {
      try {
        l(custom_event)
      } catch (error) {
        console.error('Exception Running Eventlistener')
        console.log(error)
      }
    })
    this.dispatchEvent(custom_event)
  }
}
