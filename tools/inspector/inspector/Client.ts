/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { GenericDispatcher, IEvent } from './genericdispatcher'
 import { IClient } from './IClient'
 
import { WasmClient } from './WasmClient'
import { WebSocketClient } from './WebSocketClient'

class NestedClient extends GenericDispatcher implements IClient {
    mClient: IClient = null;

    connect(location: string): Promise<string> {
        const url = new URL(location);
        if(url.protocol === 'wss:' || url.protocol === 'ws:'){
            this.mClient = new WebSocketClient();
            this.mClient.onAny(( event: CustomEvent)=>{
                this.emit(event.detail);
            })
            return this.mClient.connect(location);
        }
        else if(url.protocol === 'https:' || url.protocol === 'http:'){ 
            this.mClient = new WasmClient();
            this.mClient.onAny(( event: CustomEvent)=>{
                this.emit(event.detail);
            })
            return this.mClient.connect(location);
        }
        return new Promise((_,reject)=>{reject("Unknown protocol")});
    }
    sendCommand(command: string): void {
        if(this.mClient){
            this.mClient.sendCommand(command)
        }
    }

    type(): string {
        return this.mClient.type();
    }

    currentURL(): string {
        return this.mClient.currentURL();
    }
}

export const Client = new NestedClient()
