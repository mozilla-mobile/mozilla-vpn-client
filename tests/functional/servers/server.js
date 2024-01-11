/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import express from 'express';
import { json } from 'body-parser';
import cors from 'cors';
import { Validator } from 'jsonschema';
import { register } from './ports.js';

class Server {
  constructor(name, endpoints, headerCheck) {
    this._name = name;
    this._port = -1;
    this._endpoints = endpoints;
    this._exceptions = [];
    this._headerCheck = headerCheck;
  }

  async start() {
    const app = express()

    app.use(json());
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

    await this._createServer(app);
  }

  stop() {
    if (this._server) {
      this._server.close();
      this._server = null;
    }
  }

  get port() {
    return this._port;
  }

  get url() {
    return `http://localhost:${this._port}`;
  }

  async processRequest(req, res, paths, overriddenPaths) {
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

    if (responseData.callback) await responseData.callback(req);

    if (this._headerCheck) {
      for (let header of responseData.requiredHeaders || []) {
        if (!(header.toLowerCase() in req.headers)) {
          this._addException(`Server ${this._name} - Expected header: ${
              header} for ${req.path} - method: ${req.method} - query: ${
              JSON.stringify(req.query)}`);
          return;
        }
      }
    }

    for (let queryStringParam of responseData.queryStringParams || []) {
      if (!(queryStringParam in req.query)) {
        this._addException(`Server ${this._name} - Expected query param: ${
            queryStringParam} for ${req.path} - method: ${
            req.method} - query: ${JSON.stringify(req.query)}`);
        return;
      }
    }

    if (responseData.bodyValidator) {
      const v = new Validator();
      const resp = v.validate(req.body, responseData.bodyValidator);
      if (!resp.valid) {
        this._addException(
            `Server ${this._name} - Invalid body for ${req.path} - method: ${
                req.method} - query: ${JSON.stringify(req.query)}`);
        return;
      }
    }

    res.status(responseData.status);
    if ('bodyRaw' in responseData) {
      res.send(responseData.bodyRaw);
      return;
    }

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

  _createServer(app) {
    return new Promise(r => {
      this._port = register(this._name);
      this._server = app.listen(this._port);
      this._server.on('error', err => {
        if (err.code === 'EADDRINUSE') {
          this._createServer(app).then(r);
          return;
        }

        throw new Error(err.code);
      });
      this._server.on('listening', r);
    });
  }
};

export default Server;
