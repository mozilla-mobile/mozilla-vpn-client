import { Terminal } from 'xterm'
import { FitAddon } from 'xterm-addon-fit'
import { html, css, LitElement } from 'lit'
import { Client } from '../inspector/client'
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
  createRenderRoot () {
    return this
  }

  connectedCallback () {
    super.connectedCallback()
    const terminal = new Terminal({
      fontFamily: '"Cascadia Code", Menlo, monospace',
      fontSize: '14',
      lineHeight: 1,
      theme: baseTheme,
      cursorBlink: true,
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
    return html``
  }

  static styles = css`
  :host{
    display: none;
  }

  .terminal{
    overflow:hidden;
  }
  `

  get term () {
    return this.parentElement
  }
}
customElements.define('view-shell', ViewShell)
