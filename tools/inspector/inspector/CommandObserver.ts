/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { Client } from './Client'
import { GenericDispatcher } from './genericdispatcher'

class _CommandObserver extends GenericDispatcher {
  ready:boolean;
  counter:number;
  requests:Array<object>;
  commands:Array<string>;


  constructor () {
    super()
    Client.on('help', (e) => this.onHelpCall(e.detail))
    Client.on('disconnected', () => {
      this.ready=false;
    })
    this.counter = 0
    this.requests = []

    this.ready=false;
  }

  onHelpCall (message) {
    if(this.ready){
      return;
    }
    const text = message.value
    const lines = text.split('\n')
    this.commands = lines.map(l => l.split('\t')[0])
    this.emit({
      type: 'commandsChanged',
      value: this.commands
    })
    this.ready=true;
  }
}

export const CommandObserver = new _CommandObserver()
