/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const express = require('express')
const bodyParser = require('body-parser')
const cors = require('cors');

class Server {
  constructor(name, port, endpoints) {
    this._name = name;
    this._endpoints = endpoints;
    this._exceptions = [];

    const app = express()
    app.use(bodyParser.json());
    app.use(cors());
    app.use((req, res, next) => {
      switch (req.method) {
        case 'GET':
          this.processRequest(
              req, res, this._endpoints.GETs, this._overrideEndpoints?.GETs);
          return;

        case 'POST':
          this.processRequest(
              req, res, this._endpoints.POSTs, this._overrideEndpoints?.POSTs);
          return;

        case 'DELETE':
          this.processRequest(
              req, res, this._endpoints.DELETEs,
              this._overrideEndpoints?.DELETEs);
          return;

        default:
          this._addException(
              `Server ${this._name} - Unsupported method: ${req.method}`);
          return;
      }
    });

    this._server = app.listen(port);
  }

  stop() {
    if (this._server) {
      this._server.close();
      this._server = null;
    }
  }

  processRequest(req, res, paths, overriddenPaths) {
    function findPath(path, paths) {
      if (path in paths) {
        return paths[path];
      }

      const key = Object.keys(paths).find(
          key => paths[key].match === 'startWith' && path.startsWith(key));

      if (key) {
        return paths[key];
      }

      return null;
    }

    let responseData;
    if (typeof overriddenPaths === 'object') {
      responseData = findPath(req.path, overriddenPaths);
    }

    if (!responseData) {
      responseData = findPath(req.path, paths);
    }

    if (!responseData) {
      this._addException(
          `Server ${this._name} - Unsupported path: ${req.path} - method: ${
              req.method} - query: ${JSON.stringify(req.query)}`);
      return;
    }

    if (responseData.callback) responseData.callback(req);

    res.status(responseData.status);
    res.json(responseData.body);
  }

  throwExceptionsIfAny() {
    for (const exception of this._exceptions) {
      throw new Error(exception);
    }
  }

  _addException(exception) {
    console.log('Exception!', exception);
    this._exceptions.push(exception);
  }

  get overrideEndpoints() {
    return this._overrideEndpoints || null;
  }

  set overrideEndpoints(value) {
    this._overrideEndpoints = value;
  }
};

module.exports = Server;
