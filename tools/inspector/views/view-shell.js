/* Updated component fixing initialization issue with Lit lifecycle */
import { html, css, LitElement } from 'lit';
import { Client } from '../inspector/client.js';
import { CommandObserver } from '../inspector/CommandObserver.js';

export class ViewShell extends LitElement {

  firstUpdated() {
    this.commandList = [];
    this.lastCommand = '';

    Client.connect().then((url) => {
      this.appendOutput(`Connected to mozillaVPN@${url}\n`);
      this.runCommand('help');
    });

    Client.onAny((res) => this.processEvents(res));

    CommandObserver.onAny(() => {
      this.commandList = CommandObserver.commands;
    });

    this.inputBox.addEventListener('keydown', (e) => {
      if (e.key === 'Enter') {
        e.preventDefault();
        const command = this.inputBox.value.trim();
        this.appendOutput(`$ ${command}\n`);
        this.runCommand(command);
        this.inputBox.value = '';
      }
    });
  }

  runCommand(command) {
    if (!command) return;

    if (command === 'clear') {
      this.outputBox.value = '';
      return;
    }

    this.lastCommand = command;
    Client.sendCommand(command);
  }

  processEvents(message) {
    if (['screen_capture', 'log', 'network'].includes(message.type)) return;

    if (message.type === 'help') {
      message.value = message.value.replaceAll('\t', '\n\t');
    }

    this.writeResponse(message);
  }

  writeResponse(message) {
    const { type, value, error } = message;
    let text = typeof value === 'object' ? JSON.stringify(value) : value;

    if (!text && !error) return;

    let output = `client:(${type})\n`;
    output += error ? `ERROR: ${error}` : text;
    output += '\n';

    this.appendOutput(output);
  }

  appendOutput(text) {
    this.outputBox.value += text;
    this.outputBox.scrollTop = this.outputBox.scrollHeight;
  }

  render() {
    return html`
      <div class="holder">
        <textarea class="output" rows="20" cols="80" readonly></textarea>
        <textarea class="input" rows="2" cols="80" placeholder="Enter command..."></textarea>
      </div>
    `;
  }

  static styles = css`
    :root{
      display: flex;
    }
    .holder{
      display: flex;
      flex-direction: column;
      height: 100%;


    }
    .output {
      width: 100%;
      height: 90%;
      resize: none;
      background: #2D2E2C;
      color: #F8F8F8;
      border: none;
      font-family: monospace;
      padding: 10px;
      overflow: auto;
    }

    .input {
      width: 100%;
      height: calc(1.5em +10px);
      resize: none;
      background: #1E1E1D;
      color: #F8F8F8;
      border: none;
      font-family: monospace;
      padding: 10px 10px;
      line-height: 1.5;
      box-sizing: border-box;
    }
  `;

  get outputBox() {
    return this.shadowRoot.querySelector('.output');
  }

  get inputBox() {
    return this.shadowRoot.querySelector('.input');
  }
}

customElements.define('view-shell', ViewShell);
