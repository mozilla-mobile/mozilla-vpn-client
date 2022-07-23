/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const SUBSCRIPTION_DETAILS = {
  plan: {amount: 123, currency: 'usd', interval: 'year', interval_count: 1},
  payment: {
    payment_provider: 'stripe',
    payment_type: 'credit',
    last4: '1234',
    exp_month: 12,
    exp_year: 2022,
    brand: 'visa',
  },
  subscription: {
    _subscription_type: 'web',
    created: 1,
    current_period_end: 2,
    cancel_at_period_end: true,
    status: 'active'
  },
};

class MVPNWasm {
  constructor() {
    this._guardianOverrideEndpoints = {
      GETs: {
        '/api/v1/vpn/subscriptionDetails': {
          status: 200,
          body: SUBSCRIPTION_DETAILS,
        },
        '/api/v3/vpn/products': {
          status: 200,
          body: {
            products: [
              {
                platform: 'dummy',
                id: 'monthly',
                featured_product: false,
                type: 'monthly'
              },
              {
                platform: 'dummy',
                id: 'half-monthly',
                featured_product: false,
                type: 'half-yearly'
              },
              {
                platform: 'dummy',
                id: 'yearly',
                featured_product: true,
                type: 'yearly'
              },
            ]
          }
        },
      }
    };

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
      option.appendChild(document.createTextNode(p.name));
      preset.appendChild(option);
    }

    preset.onchange = () => this._loadPreset(preset.value);
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
          method, u, body, fxaEndpoints.generateEndpoints({
            GUARDIAN_URL:
                'https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net'
          }),
          this._fxaOverrideEndpoints, this._fxaOverrideEndpointsPreset);
    }

    if (!obj) {
      fetch(url)
          .then(async resp => Module.mvpnNetworkResponse(id, JSON.stringify({
            status: resp.status,
            body: btoa(String.fromCharCode.apply(
                null, new Uint8Array(await resp.arrayBuffer())))
          })))
          .catch(() => {
            console.error('Unable to fetch content for URL', url);
            Module.mvpnNetworkResponse(
                id, JSON.stringify({status: 0, body: ''}));
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
      Module.mvpnNetworkResponse(
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
};

const mvpnWasm = new MVPNWasm();

function mvpnNetworkRequest(id, method, url, body) {
  return mvpnWasm.networkRequest(id, method, url, body);
}
