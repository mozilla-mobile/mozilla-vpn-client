/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 import { GenericDispatcher } from './genericdispatcher'

export interface IClient extends GenericDispatcher {
    // Connects to a socket
    connect(url:string):Promise<string>;

    sendCommand(command:string):void;

    type():string;

    currentURL():string;
    
}