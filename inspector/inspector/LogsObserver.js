import { Client } from './client'
import { GenericDispatcher } from './genericdispatcher'

const MAX_LOG_COUNT = 200

class _LogsObserver extends GenericDispatcher {
  constructor () {
    super()
    this.LOG_ELEMENTS = []
    this.LOG_MODULES = []
    this.LOG_COMPONENT = []

    Client.on('log', (message) => this.guessLogLine(message.value))
  }

  parseDate (str) {
    const parts = str.split(' ')
    const date = parts[0].split('.')
    return new Date(`${date[2]}-${date[1]}-${date[0]}T${parts[1]}`)
  }

  detectType (modules, log) {
    if (/Set state:/.exec(log) && modules.includes('main')) {
      return TYPE_VPN_STATE
    }

    if (/Setting state:/.exec(log) && modules.includes('controller')) {
      return TYPE_CONTROLLER_STATE
    }

    return null
  }

  guessLogLine (line) {
    if (line.length === 0 || line[0] !== '[') return
    const dateEndPos = line.indexOf(']')
    if (dateEndPos === -1) return
    const dateString = line.slice(1, dateEndPos)
    line = line.slice(dateEndPos + 1).trim()

    const categoryStartPos = line.indexOf('(')
    if (categoryStartPos === -1) return

    line = line.slice(categoryStartPos)
    const categoryEndPos = line.indexOf(')')
    if (categoryEndPos === -1) return
    const categoryString = line.slice(1, categoryEndPos)
    const log = line.slice(categoryEndPos + 1).trim()

    const sepPos = categoryString.indexOf(' - ')
    if (sepPos === -1) return
    const modules = categoryString.slice(0, sepPos).split('|')
    const component = categoryString.slice(sepPos + 3)

    const entry = {
      date: this.parseDate(dateString),
      detectedType: this.detectType(modules, log),
      modules,
      component,
      log: log
    }
    this.LOG_ELEMENTS.push(entry)
    if (this.LOG_ELEMENTS.length > MAX_LOG_COUNT) {
      this.LOG_ELEMENTS.shift()
    }

    for (const module of modules) {
      if (!this.LOG_MODULES.includes(module)) {
        this.LOG_MODULES.push(module)
        this.emit({
          type: 'newModule',
          entry: module,
          list: this.LOG_MODULES
        })
      }
    }

    if (!this.LOG_COMPONENT.includes(component)) {
      this.LOG_COMPONENT.push(component)
      this.emit({
        type: 'newComponent',
        entry: component,
        list: this.LOG_COMPONENT
      })
    }

    this.emit({
      type: 'newLogs',
      entry,
      list: this.LOG_ELEMENTS
    })
  }
}

export const LogsObserver = new _LogsObserver()
