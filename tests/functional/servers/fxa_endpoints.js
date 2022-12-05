/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

(function(exports) {

exports.generateEndpoints = function(constants) {
  return {
    GETs: {},

    POSTs: {
      '/v1/account/status': {status: 200, body: {exists: true}},
      '/v1/account/login': {
        status: 200,
        body: {
          sessionToken: 'sessionToken',
          'verified': true,
          verificationMethod: ''
        }
      },
      '/v1/oauth/authorization': {
        status: 200,
        body: {
          code: 'the-code',
          state: '',
          redirect: `${constants.GUARDIAN_URL}/final_redirect`
        }
      },
      '/v1/session/destroy': {status: 200, body: {}},
    },

    DELETEs: {},
  };
};
})(typeof exports === 'undefined' ? this['fxaEndpoints'] = {} : exports);
