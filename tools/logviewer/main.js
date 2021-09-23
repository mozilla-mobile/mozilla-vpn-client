/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const LOG = 0;
const SEPARATOR = 1;
const SETTINGS_OR_DEVICE = 2;

const Logger = {
  _modules: [],
  _components: [],
  _contexts: {},
  _contextID: {},
  _currentContext: 'Client',

  async initialize(e) {
    if (e.target.files.length === 0) return;

    const file = e.target.files[0];
    const content = await file.text();

    let nextLine = LOG;
    for (let line of content.split('\n').map(line => line.trim())) {
      switch (nextLine) {
        case LOG:
          if (line === 'Mozilla VPN logs') {
            nextLine = SEPARATOR;
            this._currentContext = 'Client';
            break;
          }

          if (line === 'Mozilla VPN backend logs') {
            nextLine = SEPARATOR;
            this._currentContext = 'Backend';
            break;
          }

          if (line === '==== SETTINGS ====') {
            nextLine = SETTINGS_OR_DEVICE;
            this._currentContext = 'Settings';
            break;
          }

          if (line.length === 0) break;

          if (line[0] === '[') {
            this.guessLogLine(line);
            break;
          }

          this.appendLine(line);
          break;

        case SEPARATOR:
          if (line.startsWith('==')) {
            nextLine = LOG;
            break;
          }

          this.guessLogLine(line);
          break;

        case SETTINGS_OR_DEVICE:
          if (line.length === 0) break;

          if (line === '==== DEVICE ====') {
            this._currentContext = 'Device';
            break;
          }

          this.settingsOrDeviceLine(line);
          break;
      }
    }

    this.createDateRange();
    this.createContextIDs();
    this.createModuleList();
    this.createComponentList();
    this.populateLogTable();
  },

  guessLogLine(line) {
    if (line.length === 0 || line[0] !== '[') return;
    const dateEndPos = line.indexOf(']');
    if (dateEndPos === -1) return;
    const dateString = line.slice(1, dateEndPos);
    line = line.slice(dateEndPos + 1).trim();

    let categoryStartPos = line.indexOf('(');
    if (categoryStartPos === -1) return;

    line = line.slice(categoryStartPos);
    const categoryEndPos = line.indexOf(')');
    if (categoryEndPos === -1) return;
    const categoryString = line.slice(1, categoryEndPos);
    const log = line.slice(categoryEndPos + 1).trim();

    const sepPos = categoryString.indexOf(' - ');
    if (sepPos === -1) return;
    const modules = categoryString.slice(0, sepPos).split('|');
    const component = categoryString.slice(sepPos + 3);

    const entry = {
      type: 'log',
      date: this.parseDate(dateString),
      modules,
      component,
      log: [log],
    };

    if (!(this._currentContext in this._contexts)) {
      this._contexts[this._currentContext] = [];
    }

    this._contexts[this._currentContext].push(entry);

    for (let module of modules) {
      if (!this._modules.includes(module)) {
        this._modules.push(module);
      }
    }

    if (!this._components.includes(component)) {
      this._components.push(component);
    }
  },

  settingsOrDeviceLine(line) {
    const pos = line.indexOf(' -> ');
    if (pos === -1) return;
    const key = line.slice(0, pos);
    const value = line.slice(pos + 4);

    const entry = {
      type: 'keyvalue',
      key,
      value,
    };

    if (!(this._currentContext in this._contexts)) {
      this._contexts[this._currentContext] = [];
    }

    this._contexts[this._currentContext].push(entry);
  },

  appendLine(line) {
    if (!(this._currentContext in this._contexts)) {
      return;
    }

    if (this._contexts[this._currentContext].length === 0) {
      return;
    }

    const c = this._contexts[this._currentContext];
    c[c.length - 1].log.push(line);
  },

  parseDate(str) {
    const parts = str.split(' ');
    const date = parts[0].split('.');
    return new Date(`${date[2]}-${date[1]}-${date[0]}T${parts[1]}`);
  },

  createDateRange() {
    for (let context of Object.keys(this._contexts)) {
      const minDate = this._contexts[context]
                          .reduce((a, b) => a.date < b.date ? a : b)
                          .date;
      if (minDate && (!this._minDate || this._minDate > minDate))
        this._minDate = minDate;

      const maxDate = this._contexts[context]
                          .reduce((a, b) => a.date > b.date ? a : b)
                          .date;
      if (maxDate && (!this._maxDate || this._maxDate > maxDate))
        this._maxDate = maxDate;
    }

    const dateMinElm = document.getElementById('dateMinRange');
    const dateMaxElm = document.getElementById('dateMaxRange');

    const params = new URLSearchParams(window.location.search);
    if (params.has('dm')) dateMinElm.value = parseInt(params.get('dm'), 10);
    if (params.has('dM')) dateMaxElm.value = parseInt(params.get('dM'), 10);

    if (this._dateInitialized === undefined) {
      this._dateInitialized = true;
      dateMinElm.onchange = e => this.dateChanged();
      dateMaxElm.onchange = e => this.dateChanged();
    }

    this.refreshDateRange();
  },

  dateChanged() {
    this.refreshDateRange();
    this.populateLogTable();
  },

  refreshDateRange() {
    const min = this._minDate.getTime();
    const max = this._maxDate.getTime();
    const diff = max - min;
    const minValue =
        parseInt(document.getElementById('dateMinRange').value, 10) * diff /
        100;
    const maxValue =
        parseInt(document.getElementById('dateMaxRange').value, 10) * diff /
        100;
    if (minValue > maxValue) {
      document.getElementById('dateRange').textContent = `Error`;
      return;
    }

    const minDateValue = new Date((minValue + min));
    const maxDateValue = new Date((maxValue + min));
    document.getElementById('dateRange').textContent =
        `From ${minDateValue.toISOString()} to ${maxDateValue.toISOString()}`;

    this._minDateValue = minDateValue.getTime();
    this._maxDateValue = maxDateValue.getTime();
  },

  createContextIDs() {
    const params = new URLSearchParams(window.location.search);
    for (let key of params.keys()) {
      if (key.startsWith('id-')) {
        this._contextID[key.substr(3)] = parseInt(params.get(key), 10);
      }
    }
  },

  createModuleList() {
    const params = new URLSearchParams(window.location.search);
    let modules = null;
    if (params.has('m')) modules = params.getAll('m');

    const ul = document.getElementById('moduleList');
    for (let module of this._modules.sort()) {
      const li = document.createElement('li');
      const checkbox = document.createElement('input');
      checkbox.setAttribute('type', 'checkbox');
      if (modules === null || modules.includes(module)) {
        checkbox.setAttribute('checked', 'checked');
      }
      checkbox.setAttribute('id', `module-${module}`);
      checkbox.onchange = () => this.populateLogTable();
      li.appendChild(checkbox);
      li.appendChild(document.createTextNode(
          `${module} (${this.countLogInModule(module)})`));
      ul.appendChild(li);
    }
  },

  createComponentList() {
    const params = new URLSearchParams(window.location.search);
    let components = null;
    if (params.has('c')) components = params.getAll('c');

    const ul = document.getElementById('componentList');
    for (let component of this._components.sort()) {
      const li = document.createElement('li');
      const checkbox = document.createElement('input');
      checkbox.setAttribute('type', 'checkbox');
      if (components === null || components.includes(component)) {
        checkbox.setAttribute('checked', 'checked');
      }
      checkbox.setAttribute('id', `component-${component}`);
      checkbox.onchange = () => this.populateLogTable();
      li.appendChild(checkbox);
      li.appendChild(document.createTextNode(
          `${component} (${this.countLogInComponent(component)})`));
      ul.appendChild(li);
    }
  },

  moduleSelectAll() {
    this.selectAll('module', this._modules, true);
  },

  moduleUnselectAll() {
    this.selectAll('module', this._modules, false);
  },

  componentSelectAll() {
    this.selectAll('component', this._components, true);
  },

  componentUnselectAll() {
    this.selectAll('component', this._components, false);
  },

  selectAll(what, list, state) {
    for (let elm of list) {
      const e = document.getElementById(`${what}-${elm}`);
      e.checked = state;
    }
    this.populateLogTable();
  },

  countLogInModule(module) {
    let count = 0;
    for (let context of Object.keys(this._contexts)) {
      if (this._contexts[context][0].type === 'log') {
        count += this._contexts[context].reduce(
            (a, b) => b.modules.includes(module) ? a + 1 : a, 0);
      }
    }
    return count;
  },

  countLogInComponent(component) {
    let count = 0;
    for (let context of Object.keys(this._contexts)) {
      if (this._contexts[context][0].type === 'log') {
        count += this._contexts[context].reduce(
            (a, b) => b.component === component ? a + 1 : a, 0);
      }
    }
    return count;
  },

  populateLogTable() {
    const searchValue = document.getElementById('search').value.toLowerCase();

    const ulContexts = document.getElementById('contextsTabs');
    const divContexts = document.getElementById('contextsLog');
    if (ulContexts.children.length === 0) {
      for (let context of Object.keys(this._contexts)) {
        const button = document.createElement('button');
        button.setAttribute(
            'class',
            'nav-link' + (ulContexts.children.length === 0 ? ' active' : ''));
        button.setAttribute('id', `${context}-tab`);
        button.setAttribute('data-bs-toggle', 'tab');
        button.setAttribute('data-bs-target', `#${context}`);
        button.setAttribute('type', 'button');
        button.setAttribute('role', 'tab');
        button.setAttribute('aria-controls', 'home');
        button.setAttribute('aria-selected', 'true');
        button.textContent = context;

        const li = document.createElement('li');
        li.setAttribute('class', 'nav-item');
        li.setAttribute('role', 'presentation');
        li.appendChild(button);

        ulContexts.append(li);

        const div = document.createElement('div');
        div.setAttribute(
            'class',
            'tab-pane fade show' +
                (divContexts.children.length === 0 ? ' active' : ''));
        div.setAttribute('id', context);
        div.setAttribute('role', 'tabpanel');
        div.setAttribute('aria-labelledby', `${context}-tab`);

        const table = document.createElement('table');
        table.setAttribute('class', 'table table-hover table-sm');
        div.appendChild(table);

        const thead = document.createElement('thead');
        table.appendChild(thead);

        const tr = document.createElement('tr');
        thead.appendChild(tr);

        switch (this._contexts[context][0].type) {
          case 'log':
            for (let what of ['#', 'Date', 'Modules', 'Components', 'Log']) {
              const th = document.createElement('th');
              th.setAttribute('scope', 'col');
              th.textContent = what;
              tr.appendChild(th);
            }
            break;
          case 'keyvalue':
            for (let what of ['#', 'Key', 'Value']) {
              const th = document.createElement('th');
              th.setAttribute('scope', 'col');
              th.textContent = what;
              tr.appendChild(th);
            }
            break;
        }

        const tbody = document.createElement('tbody');
        tbody.setAttribute('id', `logTable-${context}`);
        table.appendChild(tbody);

        divContexts.appendChild(div);
      }
    }

    const modules = [];
    for (let module of this._modules) {
      if (document.getElementById(`module-${module}`).checked) {
        modules.push(module);
      }
    }

    const components = [];
    for (let component of this._components) {
      if (document.getElementById(`component-${component}`).checked) {
        components.push(component);
      }
    }

    const urlSearchParams = new URLSearchParams();
    for (let module of modules) urlSearchParams.append('m', module);
    for (let component of components) urlSearchParams.append('c', component);
    urlSearchParams.append('dm', document.getElementById('dateMinRange').value);
    urlSearchParams.append('dM', document.getElementById('dateMaxRange').value);
    for (let key of Object.keys(this._contextID))
      urlSearchParams.append(`id-${key}`, this._contextID[key]);
    this.updateHistory(urlSearchParams);

    for (let context of Object.keys(this._contexts)) {
      const table = document.getElementById(`logTable-${context}`);
      while (table.firstChild) table.firstChild.remove();

      let id = 0;
      for (let entry of this._contexts[context]) {
        if (entry.type === 'log') {
          if (!components.includes(entry.component)) continue;
          if (!entry.modules.map(module => modules.includes(module))
                   .includes(true))
            continue;

          if (searchValue != '' &&
              !entry.log.find(log => log.toLowerCase().includes(searchValue))) {
            continue;
          }

          const entryDateTime = entry.date.getTime();
          if (entryDateTime < this._minDateValue ||
              entryDateTime > this._maxDateValue)
            continue;

          const tr = document.createElement('tr');
          tr.setAttribute('data-id', id);

          if ((context in this._contextID) && this._contextID[context] == id)
            tr.classList.add('table-active');

          const th = document.createElement('th');
          th.setAttribute('scope', 'row');
          th.textContent = ++id;
          tr.appendChild(th);

          const tdDate = document.createElement('td');
          tdDate.textContent = entry.date.toISOString();
          tr.appendChild(tdDate);

          const tdModules = document.createElement('td');
          tdModules.textContent = entry.modules.join(', ');
          tr.appendChild(tdModules);

          const tdComponent = document.createElement('td');
          tdComponent.textContent = entry.component;
          tr.appendChild(tdComponent);

          const tdLog = document.createElement('td');
          entry.log.forEach((log, pos) => {
            if (pos !== 0) {
              tdLog.appendChild(document.createElement('br'));
            }

            const text = document.createTextNode(log);
            tdLog.appendChild(text);
          });
          tr.appendChild(tdLog);

          table.appendChild(tr);

          tr.onclick = () => this.selectRow(context, tr);
          continue
        }

        if (entry.type === 'keyvalue') {
          const tr = document.createElement('tr');
          const th = document.createElement('th');
          th.setAttribute('scope', 'row');
          th.textContent = ++id;
          tr.appendChild(th);

          const tdModules = document.createElement('td');
          tdModules.textContent = entry.key;
          tr.appendChild(tdModules);

          const tdComponent = document.createElement('td');
          tdComponent.textContent = entry.value;
          tr.appendChild(tdComponent);

          table.appendChild(tr);
        }
      }
    }
  },

  selectRow(context, row) {
    if (row.classList.contains('table-active')) {
      row.classList.remove('table-active');
      delete this._contextID[context];
    } else {
      row.classList.add('table-active');
      for (let prevRow = row.previousSibling; prevRow;
           prevRow = prevRow.previousSibling)
        prevRow.classList.remove('table-active');
      for (let nextRow = row.nextSibling; nextRow;
           nextRow = nextRow.nextSibling)
        nextRow.classList.remove('table-active');

      this._contextID[context] = parseInt(row.dataset.id, 10);
    }

    const url = new URL(window.location);
    for (let key of url.searchParams.keys()) {
      if (key.startsWith('id-')) {
        url.searchParams.delete(key);
      }
    }

    for (let key of Object.keys(this._contextID))
      url.searchParams.append(`id-${key}`, this._contextID[key]);
    this.updateHistory(url.searchParams);
  },

  keyboardSearchChanged() {
    if (this._keyboardSearchTimerId) {
      clearTimeout(this._keyboardSearchTimerId);
    }
    this._keyboardSearchTimerId =
        setTimeout(() => this.populateLogTable(), 500);
  },

  updateHistory(urlSearchParams) {
    const url = new URL(window.location);
    url.search = urlSearchParams.toString();
    window.history.pushState({}, '', url);
  }
};

document.getElementById('file').onchange = (e) => Logger.initialize(e);
document.getElementById('moduleSelectAll').onclick = () =>
    Logger.moduleSelectAll();
document.getElementById('moduleUnselectAll').onclick = () =>
    Logger.moduleUnselectAll();
document.getElementById('componentSelectAll').onclick = () =>
    Logger.componentSelectAll();
document.getElementById('componentUnselectAll').onclick = () =>
    Logger.componentUnselectAll();
document.getElementById('search').onkeypress = () =>
    Logger.keyboardSearchChanged();
