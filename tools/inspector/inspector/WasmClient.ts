/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 import { GenericDispatcher } from './genericdispatcher'
 import { IClient } from './IClient'
 

export class WasmClient extends GenericDispatcher implements IClient {
    target :Window;
    async connect(url: string): Promise<string> {
        if(window.opener == null || window.parent == null){
            throw new Error("Can't connect to a WasmClient client without an opener")
        }
        this.target= window.opener ? window.opener : window.parent;
        
        window.addEventListener("message", (event) => {
            this.handleMessage(event.data);
        });

      requestIdleCallback(()=>{
        this.emit({"type":"connected"})
      })
       return url;
    }

    handleMessage(message: any): void {
        this.emit(message);
    }

    sendCommand(command:string): void {
        this.target.postMessage(command, "*");
    }

    type(): string {
        return "wasm";
    }

    currentURL(): string {
        return "http://wasm-parent/"
    }
}
