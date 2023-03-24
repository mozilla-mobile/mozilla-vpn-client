/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

class MVPNWasm {
  constructor() {
    this._guardianOverrideEndpoints = {};

    this._fxaOverrideEndpoints = {};
  }

  init() {
    const qmlcanvas = document.querySelector('#qmlcanvas');
    const controlcanvas = document.querySelector('#controlcanvas');
    const status = document.querySelector('#qtstatus');
    const bodyClassList = document.body.classList;

    const qtLoader = QtLoader({
      canvasElements: [controlcanvas, qmlcanvas],
      showLoader: function(loaderStatus) {
        status.textContent = loaderStatus + '...';
      },
      showError: function(errorText) {
        bodyClassList.remove('wasm-loading');
        status.textContent = errorText;
        bodyClassList.add('wasm-loading-error');
      },
      showExit: function() {
        status.textContent = 'Application exit';
        if (qtLoader.exitCode !== undefined)
          status.textContent += ' with code ' + qtLoader.exitCode;
        if (qtLoader.exitText !== undefined)
          status.textContent += ' (' + qtLoader.exitText + ')';
      },
      showCanvas: function() {
        bodyClassList.remove('wasm-loading');
        bodyClassList.add('wasm-loaded');
        status.textContent = '';
      },
    });
    qtLoader.loadEmscriptenModule('mozillavpn');

    const preset = document.querySelector('#preset');
    for (let p of MVPNPresets) {
      const option = document.createElement('option');
      option.value = p.name;
      option.appendChild(document.createTextNode(`Preset: ${p.name}`));
      preset.appendChild(option);
    }

    preset.onchange = () => this._loadPreset(preset.value);

    document.querySelector('#backButton').onclick = () =>
        this._backButtonClicked();
  }

  networkRequest(id, method, url, body) {
    const u = new URL(url);

    let obj;
    if (u.hostname === 'stage-vpn.guardian.nonprod.cloudops.mozgcp.net') {
      obj = this._findResponse(
          method, u, body, guardianEndpoints.endpoints,
          this._guardianOverrideEndpoints,
          this._guardianOverrideEndpointsPreset);
    } else if (u.hostname === 'api-accounts.stage.mozaws.net') {
      obj = this._findResponse(
          method, u, body,
          fxaEndpoints.generateEndpoints(
              'https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net'),
          this._fxaOverrideEndpoints, this._fxaOverrideEndpointsPreset);
    } else if (
        url == 'https://archive.mozilla.org/pub/vpn/speedtest/50m.data') {
      // 50mb of data is too much to be handled in the browser.
      setTimeout(
          () => Module.mzNetworkResponse(id, JSON.stringify({
            status: 200,
            body: btoa(String.fromCharCode.apply(null, new Uint8Array(1024)))
          })),
          15000);
      return;
    }

    if (!obj) {
      fetch(url)
          .then(async resp => Module.mzNetworkResponse(id, JSON.stringify({
            status: resp.status,
            body: btoa(String.fromCharCode.apply(
                null, new Uint8Array(await resp.arrayBuffer())))
          })))
          .catch(() => {
            console.error('Unable to fetch content for URL', url);
            Module.mzNetworkResponse(id, JSON.stringify({status: 0, body: ''}));
          });
      return;
    }

    if (obj.callback) {
      let finalBody;
      try {
        finalBody = JSON.parse(body);
      } catch (e) {
        finalBody = body;
      }
      obj.callback({body: finalBody}, obj);
    }

    setTimeout(() => {
      Module.mzNetworkResponse(
          id,
          JSON.stringify(
              {status: obj.status, body: btoa(JSON.stringify(obj.body))}));
    }, 200);
  }

  _findResponse(
      method, url, body, endpoints, overrideEndpoints,
      overrideEndpointsPreset) {
    let obj = this._findResponseEndpoints(method, url, overrideEndpointsPreset);
    if (obj) return obj;

    obj = this._findResponseEndpoints(method, url, overrideEndpoints);
    if (obj) return obj;

    return this._findResponseEndpoints(method, url, endpoints);
  }

  _findResponseEndpoints(method, url, endpoints) {
    if (!endpoints) return null;

    switch (method) {
      case 'GET':
        return this._findResponseInMethod(url, endpoints.GETs);
      case 'POST':
        return this._findResponseInMethod(url, endpoints.POSTs);
      case 'DELETE':
        return this._findResponseInMethod(url, endpoints.DELETEs);
      default:
        console.error('Unsupported method', method);
        return null;
    }
  }

  _findResponseInMethod(url, endpoints) {
    if (!endpoints) {
      return null;
    }

    if (url.pathname in endpoints) {
      return endpoints[url.pathname];
    }

    const key = Object.keys(endpoints).find(
        key => endpoints[key].match === 'startWith' &&
            url.pathname.startsWith(key));

    if (key) {
      return endpoints[key];
    }

    return null;
  }

  async _loadPreset(presetName) {
    await controller.hardReset();
    await controller.reset();

    this._fxaOverrideEndpointsPreset = null;
    this._guardianOverrideEndpointsPreset = null;

    const presetInfo = document.querySelector('#presetInfo');
    while (presetInfo.firstChild) presetInfo.firstChild.remove();

    const preset = MVPNPresets.find(p => p.name === presetName);
    if (!preset) return;

    this._fxaOverrideEndpointsPreset = preset.fxaOverrideEndpoints;
    this._guardianOverrideEndpointsPreset = preset.guardianOverrideEndpoints;

    await preset.callback();
  }

  addPresetInfo(msg) {
    const p = document.createElement('p');
    p.textContent = msg;

    const presetInfo = document.querySelector('#presetInfo');
    presetInfo.appendChild(p);
  }

  updateLanguages(langs) {
    const languages = document.querySelector('#languages');
    while (languages.firstChild) languages.firstChild.remove();

    for (let p of langs) {
      const option = document.createElement('option');
      option.value = p;
      option.appendChild(document.createTextNode(`Language: ${p}`));
      languages.appendChild(option);
    }

    languages.onchange = () => this._loadLanguage(languages.value);
  }

  _loadLanguage(lang) {
    controller.setSetting('languageCode', lang);
  }

  _backButtonClicked() {
    controller.backButtonClicked();
  }
};

const mvpnWasm = new MVPNWasm();

function mzNetworkRequest(id, method, url, body) {
  return mvpnWasm.networkRequest(id, method, url, body);
}
