import { Client } from './client'
import { GenericDispatcher } from './genericdispatcher'

class _CommandObserver extends GenericDispatcher {
  constructor () {
    super()
    Client.on('help', (r) => this.onHelpCall(r))
    this.counter = 0
    this.requests = []
  }

  onHelpCall (message) {
    const text = message.value
    const lines = text.split('\n')
    this.commands = lines.map(l => l.split('\t')[0])
    this.emit({
      type: 'commandsChanged',
      value: this.commands
    })
  }
}

export const CommandObserver = new _CommandObserver()
