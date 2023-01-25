/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

(function(exports) {

const VALIDATORS = {
  fxaStatus: {
    type: 'object',
    properties: {email: {type: 'string'}},
    required: ['email']
  },

  fxaLogin: {
    type: 'object',
    properties: {
      email: {type: 'string'},
      authPW: {type: 'string'},
      reason: {type: 'string'},
      service: {type: 'string'},
      skipErrorCase: {type: 'boolean'},
      verificationMethod: {type: 'string'},
      metricsContext: {
        type: 'object',
        properties: {
          deviceId: {type: 'string'},
          flowId: {type: 'string'},
          flowBeginTime: {type: 'integer'},
        },
        required: ['deviceId', 'flowId', 'flowBeginTime'],
      }
    },
    required: [
      'email', 'authPW', 'reason', 'service', 'skipErrorCase',
      'verificationMethod', 'metricsContext'
    ]
  },

  fxaCreate: {
    type: 'object',
    properties: {
      email: {type: 'string'},
      authPW: {type: 'string'},
      service: {type: 'string'},
      verificationMethod: {type: 'string'},
      metricsContext: {
        type: 'object',
        properties: {
          deviceId: {type: 'string'},
          flowId: {type: 'string'},
          flowBeginTime: {type: 'integer'},
        },
        required: ['deviceId', 'flowId', 'flowBeginTime'],
      }
    },
    required:
        ['email', 'authPW', 'service', 'verificationMethod', 'metricsContext']
  },

  fxaAuthorization: {
    type: 'object',
    properties: {
      client_id: {type: 'string'},
      state: {type: 'string'},
      scope: {type: 'string'},
      access_type: {type: 'string'},
    },
    required: [
      'client_id',
      'state',
      'scope',
      'access_type',
    ]
  },

  fxaVerifyCode: {
    type: 'object',
    properties: {
      code: {type: 'string'},
      service: {type: 'string'},
      scopes: {type: 'array', items: {type: 'strings'}},
    },
    required: [
      'code',
      'service',
      'scopes',
    ]
  },

  fxaVerifyTotp: {
    type: 'object',
    properties: {
      code: {type: 'string'},
      service: {type: 'string'},
      scopes: {type: 'array', items: {type: 'strings'}},
    },
    required: [
      'code',
      'service',
      'scopes',
    ]
  },
};

exports.validators = VALIDATORS;

exports.generateEndpoints = function(guardianUrl) {
  return {
    GETs: {},

    POSTs: {
      '/v1/account/status': {
        status: 200,
        bodyValidator: VALIDATORS.fxaStatus,
        body: {exists: true}
      },

      '/v1/account/login': {
        status: 200,
        bodyValidator: VALIDATORS.fxaLogin,
        body: {
          sessionToken: 'sessionToken',
          'verified': true,
          verificationMethod: ''
        }
      },

      '/v1/oauth/authorization': {
        status: 200,
        bodyValidator: VALIDATORS.fxaAuthorization,
        body: {
          code: 'the-code',
          state: '',
          redirect: `${guardianUrl}/final_redirect`
        }
      },
      '/v1/session/destroy':
          {status: 200, requiredHeaders: ['Authorization'], body: {}},
    },

    DELETEs: {},
  };
};
})(typeof exports === 'undefined' ? this['fxaEndpoints'] = {} : exports);
