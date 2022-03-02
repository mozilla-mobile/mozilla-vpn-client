/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { Terminal } from 'xterm'
import { FitAddon } from 'xterm-addon-fit'
import { html, css, LitElement, unsafeCSS } from 'lit'
import { Client } from '../inspector/WebSocketClient'
import { CommandObserver } from '../inspector/CommandObserver'

const RED_ESCAPE = '\x1B[0;31m'
const NO_COLOR_ESCAPE = '\x1B[0m'
const NEWLINE = '\r\n'

const baseTheme = {
  foreground: '#F8F8F8',
  background: '#2D2E2C',
  selection: '#5DA5D533',
  black: '#1E1E1D',
  brightBlack: '#262625',
  red: '#CE5C5C',
  brightRed: '#FF7272',
  green: '#5BCC5B',
  brightGreen: '#72FF72',
  yellow: '#CCCC5B',
  brightYellow: '#FFFF72',
  blue: '#5D5DD3',
  brightBlue: '#7279FF',
  magenta: '#BC5ED1',
  brightMagenta: '#E572FF',
  cyan: '#5DA5D5',
  brightCyan: '#72F0FF',
  white: '#F8F8F8',
  brightWhite: '#FFFFFF'
}

export class ViewShell extends LitElement {

  async connectedCallback () {
    super.connectedCallback()

    await new Promise(resolve => {requestAnimationFrame(resolve)})
    const terminal = new Terminal({
      fontFamily: '"Cascadia Code", Menlo, monospace',
      fontSize: '14',
      lineHeight: 1,
      theme: baseTheme,
      cursorBlink: true,
      rendererType: 'canvas'
    })
    const fitAddon = new FitAddon()
    this.terminal = terminal
    this.fitAddon = fitAddon

    terminal.loadAddon(fitAddon)
    // Let Term Match Size
    window.addEventListener('resize', () => {
      this.fitAddon.fit()
    })
    terminal.open(this.term)
    fitAddon.fit()

    Client.connect().then((url) => {
      this.terminal.writeln(`\n\rConnected to mozillaVPN@${url}`)
      this.runCommand('help')
      this.prompt()
    })
    Client.onAny((res) => this.processEvents(res))

    CommandObserver.onAny(() => {
      this.commandList = CommandObserver.commands
    })

    let command = ''
    terminal.onData(e => {
      switch (e) {
        case '\u0003': // Ctrl+C
          console.log(terminal.getSelection())

          break
        case '\r': // Enter
          this.runCommand(command)
          command = ''
          break
        case '\u007F': // Backspace (DEL)
          // Do not delete the prompt
          if (terminal._core.buffer.x > 2) {
            terminal.write('\b \b')
            if (command.length > 0) {
              command = command.substr(0, command.length - 1)
            }
          }
          break
        default: // Print all other characters for demo
          if (e >= String.fromCharCode(0x20) && e <= String.fromCharCode(0x7B)) {
            command += e
            terminal.write(e)
          }
      }
    })
    terminal.onKey((key, event) => {
      if (key.key === '\u001b[A') {
        // UP key ->
        if (this.lastCommand != '') {
          command = this.lastCommand
          this.clearLine()
          terminal.write(this.lastCommand)
        }
        return
      }
      if (key.key === '\t') {
        // Tab Pressed!
        if (command === '') {
          // No Command here!
          terminal.writeln('')
          terminal.write(this.commandList.join('  '))
          this.prompt()
          return
        }
        const closestCommands = this.commandList.filter(c => c.startsWith(command))
        if (closestCommands.length > 1) {
          terminal.writeln('')
          terminal.write(closestCommands.join('  '))
          this.prompt()
          terminal.write(command)
        } else if (closestCommands.length == 1) {
          const newCommand = closestCommands.shift()
          this.clearLine()
          terminal.write(newCommand)
          command = newCommand
        }
      }
    })
  }

  runCommand (command) {
    if (command === '') {
      this.prompt()
      return
    }
    if (command === 'clear') {
      this.terminal.write('\u001Bc')
      this.prompt()
      return
    }
    this.lastCommand = command
    Client.sendCommand(command)
    this.prompt()
  }

  prompt () {
    this.terminal.write('\n\r$ ')
  }

  clearLine () {
    this.terminal.write('\x1b[2K\r')
    this.terminal.write('$ ')
  }

  processEvents (message) {
    if (['screen_capture', 'log', 'network'].includes(message.type)) {
      return
    }
    if (message.type == 'help') {
      message.value = message.value.replaceAll('\t', '\n\t')
    }
    this.writeResponse(message)
  }

  writeResponse (message) {
    const type = message.type
    let text = message.value
    if (typeof text === 'object') {
      text = JSON.stringify(text)
    }

    const error = message.error
    if (!text && !error) {
      return
    }
    this.terminal.write(NEWLINE + `client:(${type})` + NEWLINE)

    if (error) {
      this.terminal.write(`${NEWLINE}${RED_ESCAPE}${error}${NO_COLOR_ESCAPE}`)
    } else {
      this.terminal.write(text.replaceAll('\n', '\r\n'))
    }
    this.terminal.write(NEWLINE)
    this.prompt()
  }

  render () {
    return html`    
      <div id="term"></div>
    `
  }

  static styles = css`
    :host{
      display: block;
      position: absolute;
      width: 100%;
      height: 100%;
    }

  #term{
    height: 100%;
background: #2d2e2c;
  }

  /**
 * Copyright (c) 2014 The xterm.js authors. All rights reserved.
 * Copyright (c) 2012-2013, Christopher Jeffrey (MIT License)
 * https://github.com/chjj/term.js
 * @license MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Originally forked from (with the author's permission):
 *   Fabrice Bellard's javascript vt100 for jslinux:
 *   http://bellard.org/jslinux/
 *   Copyright (c) 2011 Fabrice Bellard
 *   The original design remains. The terminal itself
 *   has been extended to include xterm CSI codes, among
 *   other features.
 */

/**
 *  Default styles for xterm.js
 */

.xterm {
    position: relative;
    user-select: none;
    -ms-user-select: none;
    -webkit-user-select: none;
}

.xterm.focus,
.xterm:focus {
    outline: none;
}

.xterm .xterm-helpers {
    position: absolute;
    top: 0;
    /**
     * The z-index of the helpers must be higher than the canvases in order for
     * IMEs to appear on top.
     */
    z-index: 5;
}

.xterm .xterm-helper-textarea {
    padding: 0;
    border: 0;
    margin: 0;
    /* Move textarea out of the screen to the far left, so that the cursor is not visible */
    position: absolute;
    opacity: 0;
    left: -9999em;
    top: 0;
    width: 0;
    height: 0;
    z-index: -5;
    /** Prevent wrapping so the IME appears against the textarea at the correct position */
    white-space: nowrap;
    overflow: hidden;
    resize: none;
}

.xterm .composition-view {
    /* TODO: Composition position got messed up somewhere */
    background: #000;
    color: #FFF;
    display: none;
    position: absolute;
    white-space: nowrap;
    z-index: 1;
}

.xterm .composition-view.active {
    display: block;
}

.xterm .xterm-viewport {
    /* On OS X this is required in order for the scroll bar to appear fully opaque */
    background-color: #000;
    overflow-y: scroll;
    cursor: default;
    position: absolute;
    right: 0;
    left: 0;
    top: 0;
    bottom: 0;
}

.xterm .xterm-screen {
    position: relative;
}

.xterm .xterm-screen canvas {
    position: absolute;
    left: 0;
    top: 0;
}

.xterm .xterm-scroll-area {
    visibility: hidden;
}

.xterm-char-measure-element {
    display: inline-block;
    visibility: hidden;
    position: absolute;
    top: 0;
    left: -9999em;
    line-height: normal;
}

.xterm {
    cursor: text;
}

.xterm.enable-mouse-events {
    /* When mouse events are enabled (eg. tmux), revert to the standard pointer cursor */
    cursor: default;
}

.xterm.xterm-cursor-pointer,
.xterm .xterm-cursor-pointer {
    cursor: pointer;
}

.xterm.column-select.focus {
    /* Column selection mode */
    cursor: crosshair;
}

.xterm .xterm-accessibility,
.xterm .xterm-message {
    position: absolute;
    left: 0;
    top: 0;
    bottom: 0;
    right: 0;
    z-index: 10;
    color: transparent;
}

.xterm .live-region {
    position: absolute;
    left: -9999px;
    width: 1px;
    height: 1px;
    overflow: hidden;
}

.xterm-dim {
    opacity: 0.5;
}

.xterm-underline {
    text-decoration: underline;
}

.xterm-strikethrough {
    text-decoration: line-through;
}

  `

  get term () {
    return this.shadowRoot.getElementById('term')
  }
}
customElements.define('view-shell', ViewShell)
