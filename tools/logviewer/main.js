/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const Logger = {
  _entries: [],
  _modules: [],
  _components: [],

  async initialize(e) {
    if (e.target.files.length === 0) return;

    const file = e.target.files[0];
    const content = await file.text();
    for (let line of content.split('\n')) {
      this.parseLine(line);
    }

    this.createDateRange();
    this.createModuleList();
    this.createComponentList();
    this.populateLogTable();
  },

  parseLine(line) {
    line = line.trim();

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
      date: this.parseDate(dateString),
      modules,
      component,
      log,
    };

    this._entries.push(entry);

    for (let module of modules) {
      if (!this._modules.includes(module)) {
        this._modules.push(module);
      }
    }

    if (!this._components.includes(component)) {
      this._components.push(component);
    }
  },

  parseDate(str) {
    const parts = str.split(' ');
    const date = parts[0].split('.');
    return new Date(`${date[2]}-${date[1]}-${date[0]}T${parts[1]}`);
  },

  createDateRange() {
    this._minDate =
        this._entries.reduce((a, b) => a.date < b.date ? a : b).date;
    this._maxDate =
        this._entries.reduce((a, b) => a.date > b.date ? a : b).date;

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

  countLogInModule(module) {
    return this._entries.reduce(
        (a, b) => b.modules.includes(module) ? a + 1 : a, 0);
  },

  countLogInComponent(component) {
    return this._entries.reduce(
        (a, b) => b.component === component ? a + 1 : a, 0);
  },

  populateLogTable() {
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

    const url = new URL(window.location);
    url.search = urlSearchParams.toString();
    window.history.pushState({}, '', url);

    const table = document.getElementById('logTable');
    while (table.firstChild) table.firstChild.remove();

    let id = 0;
    for (let entry of this._entries) {
      if (!components.includes(entry.component)) continue;
      if (!entry.modules.map(module => modules.includes(module)).includes(true))
        continue;

      const entryDateTime = entry.date.getTime();
      if (entryDateTime < this._minDateValue ||
          entryDateTime > this._maxDateValue)
        continue;

      const tr = document.createElement('tr');
      const th = document.createElement('th');
      th.setAttribute('scope', 'row');
      th.textContent = ++id;
      tr.appendChild(th);

      const tdModules = document.createElement('td');
      tdModules.textContent = entry.modules.join(', ');
      tr.appendChild(tdModules);

      const tdComponent = document.createElement('td');
      tdComponent.textContent = entry.component;
      tr.appendChild(tdComponent);

      const tdLog = document.createElement('td');
      tdLog.textContent = entry.log;
      tr.appendChild(tdLog);

      table.appendChild(tr);
    }
  }
};

document.getElementById('file').onchange = (e) => Logger.initialize(e);
