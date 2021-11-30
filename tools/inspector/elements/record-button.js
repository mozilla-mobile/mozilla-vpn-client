/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import {html, css, LitElement} from 'lit'

import {LogsObserver} from '../inspector/LogsObserver'

export class RecordButton extends LitElement {
  static styles = css`
    :host{
        margin:2px;
    }
    :host button{
        border-radius: 8px;
        padding: 6px;
        background:blue;
        color: white;
        width: 100%;
    }
    :host(.recording) button{
        background:red;
    }
    `

  static properties = {
    recording: {type: Boolean},
    content: {type: String},
  }

  constructor() {
    super();
    this.recording = false;
    this.setContent();
  }

  setContent() {
    if (this.recording) this.content = "Stop recording";
    else this.content = "Start recording";
  }

  connectedCallback() {
    super.connectedCallback();
    this.addEventListener('click', () => {
      this.classList.toggle('recording');

      this.recording = !this.recording;
      this.setContent();

      if (!this.recording) {
        const entries = LogsObserver.stopRecording();
        this.saveLogs(entries);
        return;
      }

      LogsObserver.startRecording();
    })
  }

  render() {
    return html`<button>${this.content}</button>`
  }

  saveLogs(entries) {
    const a = document.createElement('a');
    document.body.appendChild(a);
    a.style = 'display: none';

    const blob = new Blob([entries.join('\n')], {type: 'octet/stream'});
    const url = window.URL.createObjectURL(blob);
    a.href = url;
    a.download = 'mozillavpn.txt';
    a.click();
    window.URL.revokeObjectURL(url);
  }
}

customElements.define('record-button', RecordButton)
