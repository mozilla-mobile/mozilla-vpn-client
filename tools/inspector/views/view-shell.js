/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { Terminal } from 'xterm'
import { FitAddon } from 'xterm-addon-fit'
import { html, css, LitElement } from 'lit'

import { signal } from '@lit-labs/preact-signals'
import { currentClient } from '../globalstate'


const NEWLINE = '\r\n'

export class ViewShell extends LitElement {

  constructor(){
    super()
    currentClient.subscribe( c => c.qWebChannel.subscribe((q)=>this.clientChanged(q)))
  }
  clientChanged(client){
    console.log(client)
    if(!client){
      return;
    }

    this.cli = client.objects.inspector_cli;
    this.cli.onSend.connect(bytes => this.onMessage(bytes));
  }

  firstUpdated(){
    const terminal = new Terminal({
      fontFamily: '"Cascadia Code", Menlo, monospace',
      fontSize: '14',
      lineHeight: 1,
      cursorBlink: false,
      disableStdin: true,
      rendererType: 'dom'
    })
    const fitAddon = new FitAddon()
    this.terminal = terminal
    this.fitAddon = fitAddon

    terminal.loadAddon(fitAddon)
    // Let Term Match Size
    window.addEventListener('resize', () => {
      this.fitAddon.fit()
    })

    const root = this.renderRoot.querySelector("#TerminalHost");
    terminal.open(root)
    fitAddon.fit()

    this.command = ''
  }
  runCommand (e) {
    const command = e.target.value;
    this.terminal.write('\u001Bc')
    this.terminal.write(command)
    this.terminal.write(NEWLINE)
    
    if (command === 'clear') {
      this.prompt()
      return
    }
    this.lastCommand = command
    this.cli.recv(command);
  }

  onMessage (message) {
    try{
      // If we get a "value" object. use that
      // otherwise return the raw thing.
      const msg_obj = JSON.parse(message);
      if(msg_obj.value){
        this.write(msg_obj.value + NEWLINE)
        this.prompt()
        return;
      }
    }catch(err){}
    this.write(message + NEWLINE)
    this.prompt()
  }
  write(text){
    this.terminal.write(text.replaceAll('\n', '\r\n'))
  }

  render () {
    return html`
      <link href='/node_modules/xterm/css/xterm.css' rel='stylesheet'>
      <div id="TerminalHost"></div>
      <input type="text" placeholder="command" @change=${this.runCommand}>
    `
  }
  static styles = css`
  :host{
    padding: 20px;
  }

  .terminal{
    overflow: auto;
    height 100%;
  }
  .xterm-helper-textarea{
    opacity: 0;
  }
  input{
    position: fixed;
    bottom: 0px;
    width: 100%;
    color: white;
    background: black;
    padding: 15px;
    font-size: 15px;
    font-family: monospace;
  }
  `
}
customElements.define('view-shell', ViewShell)
