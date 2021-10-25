import { GenericDispatcher } from './genericdispatcher'

class InspectorClient extends GenericDispatcher {
  constructor () {
    super()
    this.queue = []
  }

  getEventTypes () {
    return Array.from(this.listeners.keys())
  }

  connect () {
    if (this.connectionPromise) {
      return this.connectionPromise
    }
    this.connectionPromise = new Promise((resolve, reject) => {
      console.log('Doing Connection!')
      this.websocketConnection = new WebSocket('ws://localhost:8765')
      this.websocketConnection.onopen = () => {
        this.emit({ type: 'connected' })
        resolve(this.websocketConnection.url)
        console.log('Connected to ws://localhost:8765')
        if (this.queue.length > 0) {
          this.queue.forEach(c => this.websocketConnection.send(c))
          this.queue = []
        }
      }
      this.websocketConnection.onerror = () => { this.emit({ type: 'connectionFailed' }); reject() }
      this.websocketConnection.onclose = () => { this.emit({ type: 'disconnected' }) }
      this.websocketConnection.onmessage = data => this.processMessage(data.data)
    })
    return this.connectionPromise
  }

  processMessage (data) {
    const json = JSON.parse(data)
    this.emit(json)
  }

  sendCommand (command) {
    if (this.websocketConnection.readyState != 1) {
      this.queue.push(command)
      return
    }
    this.websocketConnection.send(command)
  }
}

export const Client = new InspectorClient()
window.Client = Client
