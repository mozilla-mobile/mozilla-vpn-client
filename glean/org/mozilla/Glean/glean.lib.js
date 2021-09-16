var Glean;
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	// The require scope
/******/ 	var __webpack_require__ = {};
/******/ 	
/************************************************************************/
/******/ 	/* webpack/runtime/define property getters */
/******/ 	(() => {
/******/ 		// define getter functions for harmony exports
/******/ 		__webpack_require__.d = (exports, definition) => {
/******/ 			for(var key in definition) {
/******/ 				if(__webpack_require__.o(definition, key) && !__webpack_require__.o(exports, key)) {
/******/ 					Object.defineProperty(exports, key, { enumerable: true, get: definition[key] });
/******/ 				}
/******/ 			}
/******/ 		};
/******/ 	})();
/******/ 	
/******/ 	/* webpack/runtime/hasOwnProperty shorthand */
/******/ 	(() => {
/******/ 		__webpack_require__.o = (obj, prop) => (Object.prototype.hasOwnProperty.call(obj, prop))
/******/ 	})();
/******/ 	
/******/ 	/* webpack/runtime/make namespace object */
/******/ 	(() => {
/******/ 		// define __esModule on exports
/******/ 		__webpack_require__.r = (exports) => {
/******/ 			if(typeof Symbol !== 'undefined' && Symbol.toStringTag) {
/******/ 				Object.defineProperty(exports, Symbol.toStringTag, { value: 'Module' });
/******/ 			}
/******/ 			Object.defineProperty(exports, '__esModule', { value: true });
/******/ 		};
/******/ 	})();
/******/ 	
/************************************************************************/
var __webpack_exports__ = {};
// This entry need to be wrapped in an IIFE because it declares 'Glean' on top-level, which conflicts with the current library output.
(() => {
// ESM COMPAT FLAG
__webpack_require__.r(__webpack_exports__);

// EXPORTS
__webpack_require__.d(__webpack_exports__, {
  "default": () => (/* binding */ index_qt)
});

;// CONCATENATED MODULE: ./src/core/error/error_type.ts
var ErrorType;
(function (ErrorType) {
    ErrorType["InvalidValue"] = "invalid_value";
    ErrorType["InvalidLabel"] = "invalid_label";
    ErrorType["InvalidState"] = "invalid_state";
    ErrorType["InvalidOverflow"] = "invalid_overflow";
})(ErrorType || (ErrorType = {}));

;// CONCATENATED MODULE: ./node_modules/tslib/tslib.es6.js
/*! *****************************************************************************
Copyright (c) Microsoft Corporation.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
***************************************************************************** */
/* global Reflect, Promise */

var extendStatics = function(d, b) {
    extendStatics = Object.setPrototypeOf ||
        ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
        function (d, b) { for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p]; };
    return extendStatics(d, b);
};

function __extends(d, b) {
    extendStatics(d, b);
    function __() { this.constructor = d; }
    d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
}

var __assign = function() {
    __assign = Object.assign || function __assign(t) {
        for (var s, i = 1, n = arguments.length; i < n; i++) {
            s = arguments[i];
            for (var p in s) if (Object.prototype.hasOwnProperty.call(s, p)) t[p] = s[p];
        }
        return t;
    }
    return __assign.apply(this, arguments);
}

function __rest(s, e) {
    var t = {};
    for (var p in s) if (Object.prototype.hasOwnProperty.call(s, p) && e.indexOf(p) < 0)
        t[p] = s[p];
    if (s != null && typeof Object.getOwnPropertySymbols === "function")
        for (var i = 0, p = Object.getOwnPropertySymbols(s); i < p.length; i++) {
            if (e.indexOf(p[i]) < 0 && Object.prototype.propertyIsEnumerable.call(s, p[i]))
                t[p[i]] = s[p[i]];
        }
    return t;
}

function __decorate(decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
}

function __param(paramIndex, decorator) {
    return function (target, key) { decorator(target, key, paramIndex); }
}

function __metadata(metadataKey, metadataValue) {
    if (typeof Reflect === "object" && typeof Reflect.metadata === "function") return Reflect.metadata(metadataKey, metadataValue);
}

function __awaiter(thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
}

function __generator(thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (_) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
}

function __createBinding(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}

function __exportStar(m, exports) {
    for (var p in m) if (p !== "default" && !exports.hasOwnProperty(p)) exports[p] = m[p];
}

function __values(o) {
    var s = typeof Symbol === "function" && Symbol.iterator, m = s && o[s], i = 0;
    if (m) return m.call(o);
    if (o && typeof o.length === "number") return {
        next: function () {
            if (o && i >= o.length) o = void 0;
            return { value: o && o[i++], done: !o };
        }
    };
    throw new TypeError(s ? "Object is not iterable." : "Symbol.iterator is not defined.");
}

function __read(o, n) {
    var m = typeof Symbol === "function" && o[Symbol.iterator];
    if (!m) return o;
    var i = m.call(o), r, ar = [], e;
    try {
        while ((n === void 0 || n-- > 0) && !(r = i.next()).done) ar.push(r.value);
    }
    catch (error) { e = { error: error }; }
    finally {
        try {
            if (r && !r.done && (m = i["return"])) m.call(i);
        }
        finally { if (e) throw e.error; }
    }
    return ar;
}

function __spread() {
    for (var ar = [], i = 0; i < arguments.length; i++)
        ar = ar.concat(__read(arguments[i]));
    return ar;
}

function __spreadArrays() {
    for (var s = 0, i = 0, il = arguments.length; i < il; i++) s += arguments[i].length;
    for (var r = Array(s), k = 0, i = 0; i < il; i++)
        for (var a = arguments[i], j = 0, jl = a.length; j < jl; j++, k++)
            r[k] = a[j];
    return r;
};

function __await(v) {
    return this instanceof __await ? (this.v = v, this) : new __await(v);
}

function __asyncGenerator(thisArg, _arguments, generator) {
    if (!Symbol.asyncIterator) throw new TypeError("Symbol.asyncIterator is not defined.");
    var g = generator.apply(thisArg, _arguments || []), i, q = [];
    return i = {}, verb("next"), verb("throw"), verb("return"), i[Symbol.asyncIterator] = function () { return this; }, i;
    function verb(n) { if (g[n]) i[n] = function (v) { return new Promise(function (a, b) { q.push([n, v, a, b]) > 1 || resume(n, v); }); }; }
    function resume(n, v) { try { step(g[n](v)); } catch (e) { settle(q[0][3], e); } }
    function step(r) { r.value instanceof __await ? Promise.resolve(r.value.v).then(fulfill, reject) : settle(q[0][2], r); }
    function fulfill(value) { resume("next", value); }
    function reject(value) { resume("throw", value); }
    function settle(f, v) { if (f(v), q.shift(), q.length) resume(q[0][0], q[0][1]); }
}

function __asyncDelegator(o) {
    var i, p;
    return i = {}, verb("next"), verb("throw", function (e) { throw e; }), verb("return"), i[Symbol.iterator] = function () { return this; }, i;
    function verb(n, f) { i[n] = o[n] ? function (v) { return (p = !p) ? { value: __await(o[n](v)), done: n === "return" } : f ? f(v) : v; } : f; }
}

function __asyncValues(o) {
    if (!Symbol.asyncIterator) throw new TypeError("Symbol.asyncIterator is not defined.");
    var m = o[Symbol.asyncIterator], i;
    return m ? m.call(o) : (o = typeof __values === "function" ? __values(o) : o[Symbol.iterator](), i = {}, verb("next"), verb("throw"), verb("return"), i[Symbol.asyncIterator] = function () { return this; }, i);
    function verb(n) { i[n] = o[n] && function (v) { return new Promise(function (resolve, reject) { v = o[n](v), settle(resolve, reject, v.done, v.value); }); }; }
    function settle(resolve, reject, d, v) { Promise.resolve(v).then(function(v) { resolve({ value: v, done: d }); }, reject); }
}

function __makeTemplateObject(cooked, raw) {
    if (Object.defineProperty) { Object.defineProperty(cooked, "raw", { value: raw }); } else { cooked.raw = raw; }
    return cooked;
};

function __importStar(mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (Object.hasOwnProperty.call(mod, k)) result[k] = mod[k];
    result.default = mod;
    return result;
}

function __importDefault(mod) {
    return (mod && mod.__esModule) ? mod : { default: mod };
}

function __classPrivateFieldGet(receiver, privateMap) {
    if (!privateMap.has(receiver)) {
        throw new TypeError("attempted to get private field on non-instance");
    }
    return privateMap.get(receiver);
}

function __classPrivateFieldSet(receiver, privateMap, value) {
    if (!privateMap.has(receiver)) {
        throw new TypeError("attempted to set private field on non-instance");
    }
    privateMap.set(receiver, value);
    return value;
}

;// CONCATENATED MODULE: ./src/core/log.ts
var LoggingLevel;
(function (LoggingLevel) {
    LoggingLevel["Debug"] = "debug";
    LoggingLevel["Info"] = "info";
    LoggingLevel["Warn"] = "warn";
    LoggingLevel["Error"] = "error";
})(LoggingLevel || (LoggingLevel = {}));
function log(modulePath, message, level = LoggingLevel.Debug) {
    const prefix = `(Glean.${modulePath})`;
    if (typeof message === "string") {
        console[level](prefix, message);
    }
    else {
        console[level](prefix, ...message);
    }
}

;// CONCATENATED MODULE: ./src/core/upload/uploader.ts
const DEFAULT_UPLOAD_TIMEOUT_MS = 10000;
var UploadResultStatus;
(function (UploadResultStatus) {
    UploadResultStatus[UploadResultStatus["RecoverableFailure"] = 0] = "RecoverableFailure";
    UploadResultStatus[UploadResultStatus["UnrecoverableFailure"] = 1] = "UnrecoverableFailure";
    UploadResultStatus[UploadResultStatus["Success"] = 2] = "Success";
})(UploadResultStatus || (UploadResultStatus = {}));
class UploadResult {
    constructor(result, status) {
        this.result = result;
        this.status = status;
    }
}
class Uploader {
}
/* harmony default export */ const uploader = (Uploader);

;// CONCATENATED MODULE: ./node_modules/uuid/dist/esm-browser/rng.js
// Unique ID creation requires a high quality random # generator. In the browser we therefore
// require the crypto API and do not support built-in fallback to lower quality random number
// generators (like Math.random()).
var getRandomValues;
var rnds8 = new Uint8Array(16);
function rng() {
  // lazy load so that environments that need to polyfill have a chance to do so
  if (!getRandomValues) {
    // getRandomValues needs to be invoked in a context where "this" is a Crypto implementation. Also,
    // find the complete implementation of crypto (msCrypto) on IE11.
    getRandomValues = typeof crypto !== 'undefined' && crypto.getRandomValues && crypto.getRandomValues.bind(crypto) || typeof msCrypto !== 'undefined' && typeof msCrypto.getRandomValues === 'function' && msCrypto.getRandomValues.bind(msCrypto);

    if (!getRandomValues) {
      throw new Error('crypto.getRandomValues() not supported. See https://github.com/uuidjs/uuid#getrandomvalues-not-supported');
    }
  }

  return getRandomValues(rnds8);
}
;// CONCATENATED MODULE: ./node_modules/uuid/dist/esm-browser/regex.js
/* harmony default export */ const regex = (/^(?:[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}|00000000-0000-0000-0000-000000000000)$/i);
;// CONCATENATED MODULE: ./node_modules/uuid/dist/esm-browser/validate.js


function validate(uuid) {
  return typeof uuid === 'string' && regex.test(uuid);
}

/* harmony default export */ const esm_browser_validate = (validate);
;// CONCATENATED MODULE: ./node_modules/uuid/dist/esm-browser/stringify.js

/**
 * Convert array of 16 byte values to UUID string format of the form:
 * XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
 */

var byteToHex = [];

for (var i = 0; i < 256; ++i) {
  byteToHex.push((i + 0x100).toString(16).substr(1));
}

function stringify(arr) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  // Note: Be careful editing this code!  It's been tuned for performance
  // and works in ways you may not expect. See https://github.com/uuidjs/uuid/pull/434
  var uuid = (byteToHex[arr[offset + 0]] + byteToHex[arr[offset + 1]] + byteToHex[arr[offset + 2]] + byteToHex[arr[offset + 3]] + '-' + byteToHex[arr[offset + 4]] + byteToHex[arr[offset + 5]] + '-' + byteToHex[arr[offset + 6]] + byteToHex[arr[offset + 7]] + '-' + byteToHex[arr[offset + 8]] + byteToHex[arr[offset + 9]] + '-' + byteToHex[arr[offset + 10]] + byteToHex[arr[offset + 11]] + byteToHex[arr[offset + 12]] + byteToHex[arr[offset + 13]] + byteToHex[arr[offset + 14]] + byteToHex[arr[offset + 15]]).toLowerCase(); // Consistency check for valid UUID.  If this throws, it's likely due to one
  // of the following:
  // - One or more input array values don't map to a hex octet (leading to
  // "undefined" in the uuid)
  // - Invalid input values for the RFC `version` or `variant` fields

  if (!esm_browser_validate(uuid)) {
    throw TypeError('Stringified UUID is invalid');
  }

  return uuid;
}

/* harmony default export */ const esm_browser_stringify = (stringify);
;// CONCATENATED MODULE: ./node_modules/uuid/dist/esm-browser/v4.js



function v4(options, buf, offset) {
  options = options || {};
  var rnds = options.random || (options.rng || rng)(); // Per 4.4, set bits for version and `clock_seq_hi_and_reserved`

  rnds[6] = rnds[6] & 0x0f | 0x40;
  rnds[8] = rnds[8] & 0x3f | 0x80; // Copy bytes to buffer, if provided

  if (buf) {
    offset = offset || 0;

    for (var i = 0; i < 16; ++i) {
      buf[offset + i] = rnds[i];
    }

    return buf;
  }

  return esm_browser_stringify(rnds);
}

/* harmony default export */ const esm_browser_v4 = (v4);
;// CONCATENATED MODULE: ./src/core/dispatcher.ts


const LOG_TAG = "core.Dispatcher";
var DispatcherState;
(function (DispatcherState) {
    DispatcherState["Uninitialized"] = "Uninitialized";
    DispatcherState["Idle"] = "Idle";
    DispatcherState["Processing"] = "Processing";
    DispatcherState["Stopped"] = "Stopped";
    DispatcherState["Shutdown"] = "Shutdown";
})(DispatcherState || (DispatcherState = {}));
var Commands;
(function (Commands) {
    Commands["Task"] = "Task";
    Commands["PersistentTask"] = "PersistentTask";
    Commands["Stop"] = "Stop";
    Commands["Clear"] = "Clear";
    Commands["Shutdown"] = "Shutdown";
    Commands["TestTask"] = "TestTask";
})(Commands || (Commands = {}));
class Dispatcher {
    constructor(maxPreInitQueueSize = 100, logTag = LOG_TAG) {
        this.maxPreInitQueueSize = maxPreInitQueueSize;
        this.logTag = logTag;
        this.shuttingDown = false;
        this.currentJob = Promise.resolve();
        this.queue = [];
        this.state = "Uninitialized";
    }
    getNextCommand() {
        return this.queue.shift();
    }
    executeTask(task, debugTag) {
        return __awaiter(this, void 0, void 0, function* () {
            try {
                yield task();
                log(this.logTag, [
                    "Done executing task in Task command:",
                    debugTag ? `[${debugTag}]` : "[unidentified]"
                ]);
            }
            catch (e) {
                log(this.logTag, ["Error executing task:", JSON.stringify(e)], LoggingLevel.Error);
            }
        });
    }
    unblockTestResolvers() {
        this.queue.forEach(c => {
            if (c.command === "TestTask") {
                c.resolver();
            }
        });
    }
    execute() {
        return __awaiter(this, void 0, void 0, function* () {
            let nextCommand = this.getNextCommand();
            while (nextCommand) {
                log(this.logTag, [
                    `Executing dispatched ${nextCommand.command} command:`,
                    nextCommand.debugTag ? `[${nextCommand.debugTag}]` : "[unidentified]"
                ]);
                switch (nextCommand.command) {
                    case ("Stop"):
                        this.state = "Stopped";
                        return;
                    case ("Shutdown"):
                        this.unblockTestResolvers();
                        this.queue = [];
                        this.state = "Shutdown";
                        this.shuttingDown = false;
                        return;
                    case ("Clear"):
                        this.unblockTestResolvers();
                        this.queue = this.queue.filter(c => ["PersistentTask", "Shutdown"].includes(c.command));
                        nextCommand = this.getNextCommand();
                        continue;
                    case ("TestTask"):
                        yield this.executeTask(nextCommand.task, nextCommand.debugTag);
                        nextCommand.resolver();
                        nextCommand = this.getNextCommand();
                        continue;
                    case ("PersistentTask"):
                    case ("Task"):
                        yield this.executeTask(nextCommand.task, nextCommand.debugTag);
                        nextCommand = this.getNextCommand();
                }
                if (nextCommand) {
                    log(this.logTag, ["Getting the next command...", nextCommand.command]);
                }
            }
        });
    }
    triggerExecution() {
        if (this.state === "Idle" && this.queue.length > 0) {
            this.state = "Processing";
            this.currentJob = this.execute();
            this.currentJob
                .then(() => {
                const that = this;
                if (this.state === "Processing") {
                    that.state = "Idle";
                }
                log(this.logTag, `Done executing tasks, the dispatcher is now in the ${this.state} state.`);
            })
                .catch(error => {
                log(this.logTag, [
                    "IMPOSSIBLE: Something went wrong while the dispatcher was executing the tasks queue.",
                    error
                ], LoggingLevel.Error);
            });
        }
    }
    launchInternal(command, priorityTask = false) {
        if (this.state === "Shutdown") {
            log(this.logTag, "Attempted to enqueue a new task but the dispatcher is shutdown. Ignoring.", LoggingLevel.Warn);
            return false;
        }
        if (!priorityTask && this.state === "Uninitialized") {
            if (this.queue.length >= this.maxPreInitQueueSize) {
                log(this.logTag, "Unable to enqueue task, pre init queue is full.", LoggingLevel.Warn);
                return false;
            }
        }
        if (priorityTask) {
            this.queue.unshift(command);
        }
        else {
            this.queue.push(command);
        }
        this.triggerExecution();
        return true;
    }
    launch(task, debugTag) {
        this.launchInternal({
            task,
            debugTag,
            command: "Task"
        });
    }
    launchPersistent(task, debugTag) {
        this.launchInternal({
            task,
            debugTag,
            command: "PersistentTask"
        });
    }
    flushInit(task, debugTag) {
        if (this.state !== "Uninitialized") {
            log(this.logTag, "Attempted to initialize the Dispatcher, but it is already initialized. Ignoring.", LoggingLevel.Warn);
            return;
        }
        if (task) {
            this.launchInternal({
                task,
                debugTag,
                command: "Task"
            }, true);
        }
        this.state = "Idle";
        this.triggerExecution();
    }
    clear(priorityTask = true) {
        this.launchInternal({ command: "Clear" }, priorityTask);
        this.resume();
    }
    stop(priorityTask = true) {
        if (this.shuttingDown) {
            this.clear(priorityTask);
        }
        else {
            this.launchInternal({ command: "Stop" }, priorityTask);
        }
    }
    resume() {
        if (this.state === "Stopped") {
            this.state = "Idle";
            this.triggerExecution();
        }
    }
    shutdown() {
        this.shuttingDown = true;
        this.launchInternal({ command: "Shutdown" });
        this.resume();
        return this.currentJob;
    }
    testBlockOnQueue() {
        return __awaiter(this, void 0, void 0, function* () {
            return yield this.currentJob;
        });
    }
    testUninitialize() {
        return __awaiter(this, void 0, void 0, function* () {
            if (this.state === "Uninitialized") {
                return;
            }
            this.clear();
            yield this.shutdown();
            this.state = "Uninitialized";
        });
    }
    testLaunch(task) {
        return new Promise((resolver, reject) => {
            this.resume();
            const wasLaunched = this.launchInternal({
                resolver,
                task,
                command: "TestTask"
            });
            if (!wasLaunched) {
                reject();
            }
        });
    }
}
/* harmony default export */ const core_dispatcher = (Dispatcher);

;// CONCATENATED MODULE: ./src/core/context.ts


const context_LOG_TAG = "core.Context";
class Context {
    constructor() {
        this._initialized = false;
        this._startTime = new Date();
        this._dispatcher = new core_dispatcher();
    }
    static get instance() {
        if (!Context._instance) {
            Context._instance = new Context();
        }
        return Context._instance;
    }
    static testUninitialize() {
        Context._instance = undefined;
    }
    static get dispatcher() {
        return Context.instance._dispatcher;
    }
    static get uploadEnabled() {
        if (typeof Context.instance._uploadEnabled === "undefined") {
            log(context_LOG_TAG, [
                "Attempted to access Context.uploadEnabled before it was set. This may cause unexpected behaviour.",
            ], LoggingLevel.Error);
        }
        return Context.instance._uploadEnabled;
    }
    static set uploadEnabled(upload) {
        Context.instance._uploadEnabled = upload;
    }
    static get metricsDatabase() {
        if (typeof Context.instance._metricsDatabase === "undefined") {
            log(context_LOG_TAG, [
                "Attempted to access Context.metricsDatabase before it was set. This may cause unexpected behaviour.",
            ], LoggingLevel.Error);
        }
        return Context.instance._metricsDatabase;
    }
    static set metricsDatabase(db) {
        Context.instance._metricsDatabase = db;
    }
    static get eventsDatabase() {
        if (typeof Context.instance._eventsDatabase === "undefined") {
            log(context_LOG_TAG, [
                "Attempted to access Context.eventsDatabase before it was set. This may cause unexpected behaviour.",
            ], LoggingLevel.Error);
        }
        return Context.instance._eventsDatabase;
    }
    static set eventsDatabase(db) {
        Context.instance._eventsDatabase = db;
    }
    static get pingsDatabase() {
        if (typeof Context.instance._pingsDatabase === "undefined") {
            log(context_LOG_TAG, [
                "Attempted to access Context.pingsDatabase before it was set. This may cause unexpected behaviour.",
            ], LoggingLevel.Error);
        }
        return Context.instance._pingsDatabase;
    }
    static set pingsDatabase(db) {
        Context.instance._pingsDatabase = db;
    }
    static get errorManager() {
        if (typeof Context.instance._errorManager === "undefined") {
            log(context_LOG_TAG, [
                "Attempted to access Context.errorManager before it was set. This may cause unexpected behaviour.",
            ], LoggingLevel.Error);
        }
        return Context.instance._errorManager;
    }
    static set errorManager(db) {
        Context.instance._errorManager = db;
    }
    static get applicationId() {
        if (typeof Context.instance._applicationId === "undefined") {
            log(context_LOG_TAG, [
                "Attempted to access Context.applicationId before it was set. This may cause unexpected behaviour.",
            ], LoggingLevel.Error);
        }
        return Context.instance._applicationId;
    }
    static set applicationId(id) {
        Context.instance._applicationId = id;
    }
    static get initialized() {
        return Context.instance._initialized;
    }
    static set initialized(init) {
        Context.instance._initialized = init;
    }
    static get debugOptions() {
        if (typeof Context.instance._debugOptions === "undefined") {
            log(context_LOG_TAG, [
                "Attempted to access Context.debugOptions before it was set. This may cause unexpected behaviour.",
            ], LoggingLevel.Error);
        }
        return Context.instance._debugOptions;
    }
    static set debugOptions(options) {
        Context.instance._debugOptions = options;
    }
    static get startTime() {
        return Context.instance._startTime;
    }
}

;// CONCATENATED MODULE: ./src/core/utils.ts




function isJSONValue(v) {
    if (isString(v) || isBoolean(v) || isNumber(v)) {
        return true;
    }
    if (isObject(v)) {
        if (Object.keys(v).length === 0) {
            return true;
        }
        for (const key in v) {
            return isJSONValue(v[key]);
        }
    }
    if (Array.isArray(v)) {
        return v.every((e) => isJSONValue(e));
    }
    return false;
}
function isObject(v) {
    return (typeof v === "object" && v !== null && v.constructor === Object);
}
function isUndefined(v) {
    return typeof v === "undefined";
}
function isString(v) {
    return typeof v === "string";
}
function isBoolean(v) {
    return typeof v === "boolean";
}
function isNumber(v) {
    return typeof v === "number" && !isNaN(v);
}
function isInteger(v) {
    return isNumber(v) && Number.isInteger(v);
}
function sanitizeApplicationId(applicationId) {
    return applicationId.replace(/[^a-z0-9]+/gi, "-").toLowerCase();
}
function validateURL(v) {
    const urlPattern = /^(http|https):\/\/[a-zA-Z0-9._-]+(:\d+){0,1}(\/{0,1})$/i;
    return urlPattern.test(v);
}
function validateHeader(v) {
    return /^[a-z0-9-]{1,20}$/i.test(v);
}
function generateUUIDv4() {
    if (typeof crypto !== "undefined") {
        return esm_browser_v4();
    }
    else {
        return "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx".replace(/[xy]/g, function (c) {
            const r = Math.random() * 16 | 0, v = c == "x" ? r : (r & 0x3 | 0x8);
            return v.toString(16);
        });
    }
}
const TIME_ORIGIN = Date.now();
function getMonotonicNow() {
    return typeof performance === "undefined"
        ? (Date.now() - TIME_ORIGIN)
        : performance.now();
}
function truncateStringAtBoundaryWithError(metric, value, length) {
    return __awaiter(this, void 0, void 0, function* () {
        const truncated = value.substr(0, length);
        if (truncated !== value) {
            yield Context.errorManager.record(metric, ErrorType.InvalidOverflow, `Value length ${value.length} exceeds maximum of ${length}.`);
        }
        return truncated;
    });
}

;// CONCATENATED MODULE: ./src/platform/qt/uploader.ts





const uploader_LOG_TAG = "platform.qt.Uploader";
class QtUploader extends uploader {
    post(url, body, headers = {}) {
        return __awaiter(this, void 0, void 0, function* () {
            return new Promise((resolve) => {
                const xhr = new XMLHttpRequest();
                xhr.timeout = DEFAULT_UPLOAD_TIMEOUT_MS;
                xhr.open("POST", url);
                for (const header in headers) {
                    xhr.setRequestHeader(header, headers[header]);
                }
                xhr.ontimeout = function (e) {
                    log(uploader_LOG_TAG, ["Timeout while attempting to upload ping.\n", e.type], LoggingLevel.Error);
                    resolve(new UploadResult(0));
                };
                xhr.onerror = function (e) {
                    log(uploader_LOG_TAG, ["Network error while attempting to upload ping.\n", e.type], LoggingLevel.Error);
                    resolve(new UploadResult(0));
                };
                xhr.onabort = function (e) {
                    log(uploader_LOG_TAG, ["The attempt to upload ping is aborted.\n", e.type], LoggingLevel.Error);
                    resolve(new UploadResult(0));
                };
                xhr.onload = () => {
                    resolve(new UploadResult(2, xhr.status));
                };
                if (!isString(body)) {
                    xhr.send(body.buffer);
                }
                else {
                    xhr.send(body);
                }
            });
        });
    }
}
/* harmony default export */ const qt_uploader = (new QtUploader());

;// CONCATENATED MODULE: ./src/platform/qt/platform_info.ts

const QtPlatformInfo = {
    os() {
        return __awaiter(this, void 0, void 0, function* () {
            const osName = Qt.platform.os;
            switch (osName) {
                case "android":
                    return "Android";
                case "ios":
                    return "iOS";
                case "tvos":
                    return "tvOS";
                case "linux":
                    return "Linux";
                case "osx":
                    return "Darwin";
                case "qnx":
                    return "QNX";
                case "windows":
                case "winrt":
                    return "Windows";
                case "wasm":
                    return "Wasm";
                default:
                    return "Unknown";
            }
        });
    },
    osVersion() {
        return __awaiter(this, void 0, void 0, function* () {
            return Promise.resolve("Unknown");
        });
    },
    arch() {
        return __awaiter(this, void 0, void 0, function* () {
            return Promise.resolve("Unknown");
        });
    },
    locale() {
        return __awaiter(this, void 0, void 0, function* () {
            const locale = Qt.locale();
            return Promise.resolve(locale ? locale.name.replace("_", "-") : "und");
        });
    }
};
/* harmony default export */ const platform_info = (QtPlatformInfo);

;// CONCATENATED MODULE: ./src/core/storage/utils.ts


const utils_LOG_TAG = "core.Storage.Utils";
function getValueFromNestedObject(obj, index) {
    if (index.length === 0) {
        throw Error("The index must contain at least one property to get.");
    }
    let target = obj;
    for (const key of index) {
        if (isObject(target) && key in target) {
            const temp = target[key];
            if (isJSONValue(temp)) {
                target = temp;
            }
        }
        else {
            return;
        }
    }
    return target;
}
function updateNestedObject(obj, index, transformFn) {
    if (index.length === 0) {
        throw Error("The index must contain at least one property to update.");
    }
    const returnObject = Object.assign({}, obj);
    let target = returnObject;
    for (const key of index.slice(0, index.length - 1)) {
        if (!isObject(target[key])) {
            target[key] = {};
        }
        target = target[key];
    }
    const finalKey = index[index.length - 1];
    const current = target[finalKey];
    try {
        const value = transformFn(current);
        target[finalKey] = value;
        return returnObject;
    }
    catch (e) {
        log(utils_LOG_TAG, ["Error while transforming stored value. Ignoring old value.", JSON.stringify(e)], LoggingLevel.Error);
        target[finalKey] = transformFn(undefined);
        return returnObject;
    }
}
function deleteKeyFromNestedObject(obj, index) {
    if (index.length === 0) {
        return {};
    }
    const returnObject = Object.assign({}, obj);
    let target = returnObject;
    for (const key of index.slice(0, index.length - 1)) {
        const value = target[key];
        if (!isObject(value)) {
            throw Error(`Attempted to delete an entry from an inexistent index: ${JSON.stringify(index)}.`);
        }
        else {
            target = value;
        }
    }
    const finalKey = index[index.length - 1];
    delete target[finalKey];
    return returnObject;
}

;// CONCATENATED MODULE: ./src/platform/qt/storage.ts




const storage_LOG_TAG = "platform.qt.Storage";
const DATABASE_NAME = "Glean";
const ESTIMATED_DATABASE_SIZE = 150 * 2 * 10 ** 3;
const SEPARATOR = "+";
function getKeyValueArrayFromNestedObject(value, index = "", result = []) {
    if (isObject(value)) {
        const keys = Object.keys(value);
        for (const key of keys) {
            const target = value[key];
            if (!isUndefined(target)) {
                getKeyValueArrayFromNestedObject(target, `${index}${key}${SEPARATOR}`, result);
            }
        }
    }
    else {
        result.push([index.slice(0, -1), JSON.stringify(value)]);
    }
    return result;
}
function queryResultToJSONObject(queryResult) {
    if (!queryResult || queryResult.rows.length === 0) {
        return;
    }
    const obj = {};
    for (let i = 0; i < queryResult.rows.length; i++) {
        const item = queryResult.rows.item(i);
        const index = item.key.split(SEPARATOR);
        let target = obj;
        for (const key of index.slice(0, -1)) {
            if (isUndefined(target[key])) {
                target[key] = {};
            }
            target = target[key];
        }
        try {
            target[index[index.length - 1]] = JSON.parse(item.value);
        }
        catch (e) {
            target[index[index.length - 1]] = item.value;
        }
    }
    return obj;
}
class QMLStore {
    constructor(tableName, name = DATABASE_NAME) {
        this.tableName = tableName;
        this.name = name;
        this.initialized = this._executeQuery(`CREATE TABLE IF NOT EXISTS ${tableName}(key VARCHAR(255), value VARCHAR(255));`);
        this.logTag = `${storage_LOG_TAG}.${tableName}`;
    }
    _createKeyFromIndex(index) {
        return index.join(SEPARATOR);
    }
    get _dbHandle() {
        try {
            const handle = LocalStorage.LocalStorage.openDatabaseSync(this.name, "1.0", `${this.name} Storage`, ESTIMATED_DATABASE_SIZE);
            this.dbHandle = handle;
        }
        catch (e) {
            log(this.logTag, ["Error while attempting to access LocalStorage.\n", JSON.stringify(e)], LoggingLevel.Debug);
        }
        finally {
            return this.dbHandle;
        }
    }
    _executeQuery(query) {
        const handle = this._dbHandle;
        return new Promise((resolve, reject) => {
            try {
                handle.transaction((tx) => {
                    const result = tx.executeSql(query);
                    resolve(result);
                });
            }
            catch (e) {
                log(this.logTag, [`Error executing LocalStorage query: ${query}.\n`, JSON.stringify(e)], LoggingLevel.Debug);
                reject();
            }
        });
    }
    _executeOnceInitialized(query) {
        return __awaiter(this, void 0, void 0, function* () {
            yield this.initialized;
            return this._executeQuery(query);
        });
    }
    _getFullResultObject(index) {
        return __awaiter(this, void 0, void 0, function* () {
            const key = this._createKeyFromIndex(index);
            const result = yield this._executeOnceInitialized(`SELECT * FROM ${this.tableName} WHERE key LIKE "${key}%"`);
            return queryResultToJSONObject(result);
        });
    }
    _getWholeStore() {
        return __awaiter(this, void 0, void 0, function* () {
            const result = yield this._executeOnceInitialized(`SELECT * FROM ${this.tableName}`);
            return queryResultToJSONObject(result);
        });
    }
    get(index = []) {
        return __awaiter(this, void 0, void 0, function* () {
            if (index.length === 0) {
                return this._getWholeStore();
            }
            const obj = (yield this._getFullResultObject(index)) || {};
            try {
                return getValueFromNestedObject(obj, index);
            }
            catch (e) {
                log(this.logTag, [
                    "Error getting value from database.",
                    JSON.stringify(e.message)
                ]);
            }
        });
    }
    update(index, transformFn) {
        return __awaiter(this, void 0, void 0, function* () {
            const result = (yield this._getFullResultObject(index)) || {};
            const transformedResult = updateNestedObject(result, index, transformFn);
            const updates = getKeyValueArrayFromNestedObject(transformedResult);
            for (const update of updates) {
                const [key, value] = update;
                const escapedValue = value.replace("'", "''");
                const updateResult = yield this._executeOnceInitialized(`UPDATE ${this.tableName} SET value='${escapedValue}' WHERE key='${key}'`);
                if (!(updateResult === null || updateResult === void 0 ? void 0 : updateResult.rows.length)) {
                    yield this._executeOnceInitialized(`INSERT INTO ${this.tableName}(key, value) VALUES('${key}', '${escapedValue}');`);
                }
            }
        });
    }
    delete(index) {
        return __awaiter(this, void 0, void 0, function* () {
            const key = this._createKeyFromIndex(index);
            yield this._executeOnceInitialized(`DELETE FROM ${this.tableName} WHERE key LIKE "${key}%"`);
        });
    }
}
/* harmony default export */ const storage = (QMLStore);

;// CONCATENATED MODULE: ./src/platform/qt/index.ts



const QtPlatform = {
    Storage: storage,
    uploader: qt_uploader,
    info: platform_info,
    name: "Qt"
};
/* harmony default export */ const qt = (QtPlatform);

;// CONCATENATED MODULE: ./src/core/constants.ts
const GLEAN_SCHEMA_VERSION = 1;
const GLEAN_VERSION = "0.20.0";
const PING_INFO_STORAGE = "glean_ping_info";
const CLIENT_INFO_STORAGE = "glean_client_info";
const KNOWN_CLIENT_ID = "c0ffeec0-ffee-c0ff-eec0-ffeec0ffeec0";
const DEFAULT_TELEMETRY_ENDPOINT = "https://incoming.telemetry.mozilla.org";
const DELETION_REQUEST_PING_NAME = "deletion-request";
const GLEAN_MAX_SOURCE_TAGS = 5;
const GLEAN_REFERENCE_TIME_EXTRA_KEY = "#glean_reference_time";
const GLEAN_EXECUTION_COUNTER_EXTRA_KEY = "#glean_execution_counter";
const GLEAN_RESERVED_EXTRA_KEYS = [
    GLEAN_EXECUTION_COUNTER_EXTRA_KEY,
    GLEAN_REFERENCE_TIME_EXTRA_KEY
];

;// CONCATENATED MODULE: ./src/core/config.ts



const config_LOG_TAG = "core.Config";
class Configuration {
    constructor(config) {
        this.appBuild = config === null || config === void 0 ? void 0 : config.appBuild;
        this.appDisplayVersion = config === null || config === void 0 ? void 0 : config.appDisplayVersion;
        this.debug = Configuration.sanitizeDebugOptions(config === null || config === void 0 ? void 0 : config.debug);
        if ((config === null || config === void 0 ? void 0 : config.serverEndpoint) && !validateURL(config.serverEndpoint)) {
            throw new Error(`Unable to initialize Glean, serverEndpoint ${config.serverEndpoint} is an invalid URL.`);
        }
        this.serverEndpoint = (config && config.serverEndpoint)
            ? config.serverEndpoint : DEFAULT_TELEMETRY_ENDPOINT;
        this.httpClient = config === null || config === void 0 ? void 0 : config.httpClient;
    }
    static sanitizeDebugOptions(debug) {
        const correctedDebugOptions = debug || {};
        if ((debug === null || debug === void 0 ? void 0 : debug.debugViewTag) !== undefined && !Configuration.validateDebugViewTag(debug === null || debug === void 0 ? void 0 : debug.debugViewTag)) {
            delete correctedDebugOptions["debugViewTag"];
        }
        if ((debug === null || debug === void 0 ? void 0 : debug.sourceTags) !== undefined && !Configuration.validateSourceTags(debug === null || debug === void 0 ? void 0 : debug.sourceTags)) {
            delete correctedDebugOptions["sourceTags"];
        }
        return correctedDebugOptions;
    }
    static validateDebugViewTag(tag) {
        const validation = validateHeader(tag);
        if (!validation) {
            log(config_LOG_TAG, [
                `"${tag}" is not a valid \`debugViewTag\` value.`,
                "Please make sure the value passed satisfies the regex `^[a-zA-Z0-9-]{1,20}$`."
            ], LoggingLevel.Error);
        }
        return validation;
    }
    static validateSourceTags(tags) {
        if (tags.length < 1 || tags.length > GLEAN_MAX_SOURCE_TAGS) {
            log(config_LOG_TAG, `A list of tags cannot contain more than ${GLEAN_MAX_SOURCE_TAGS} elements.`, LoggingLevel.Error);
            return false;
        }
        for (const tag of tags) {
            if (tag.startsWith("glean")) {
                log(config_LOG_TAG, "Tags starting with `glean` are reserved and must not be used.", LoggingLevel.Error);
                return false;
            }
            if (!validateHeader(tag)) {
                return false;
            }
        }
        return true;
    }
}

;// CONCATENATED MODULE: ./src/core/metrics/metric.ts

const metric_LOG_TAG = "core.Metrics.Metric";
class Metric {
    constructor(v) {
        if (!this.validate(v)) {
            throw new Error("Unable to create new Metric instance, value is in unexpected format.");
        }
        this._inner = v;
    }
    get() {
        return this._inner;
    }
    set(v) {
        if (!this.validate(v)) {
            log(metric_LOG_TAG, `Unable to set metric to ${JSON.stringify(v)}. Value is in unexpected format. Ignoring.`, LoggingLevel.Error);
            return;
        }
        this._inner = v;
    }
}

;// CONCATENATED MODULE: ./src/core/metrics/types/labeled.ts




class LabeledMetric extends Metric {
    constructor(v) {
        super(v);
    }
    validate(v) {
        return true;
    }
    payload() {
        return this._inner;
    }
}
const MAX_LABELS = 16;
const MAX_LABEL_LENGTH = 61;
const OTHER_LABEL = "__other__";
const LABEL_REGEX = /^[a-z_][a-z0-9_-]{0,29}(\.[a-z_][a-z0-9_-]{0,29})*$/;
function combineIdentifierAndLabel(metricName, label) {
    return `${metricName}/${label}`;
}
function stripLabel(identifier) {
    return identifier.split("/")[0];
}
function getValidDynamicLabel(metric) {
    return __awaiter(this, void 0, void 0, function* () {
        if (metric.dynamicLabel === undefined) {
            throw new Error("This point should never be reached.");
        }
        const key = combineIdentifierAndLabel(metric.baseIdentifier(), metric.dynamicLabel);
        for (const ping of metric.sendInPings) {
            if (yield Context.metricsDatabase.hasMetric(metric.lifetime, ping, metric.type, key)) {
                return key;
            }
        }
        let numUsedKeys = 0;
        for (const ping of metric.sendInPings) {
            numUsedKeys += yield Context.metricsDatabase.countByBaseIdentifier(metric.lifetime, ping, metric.type, metric.baseIdentifier());
        }
        let hitError = false;
        if (numUsedKeys >= MAX_LABELS) {
            hitError = true;
        }
        else if (metric.dynamicLabel.length > MAX_LABEL_LENGTH) {
            hitError = true;
            yield Context.errorManager.record(metric, ErrorType.InvalidLabel, `Label length ${metric.dynamicLabel.length} exceeds maximum of ${MAX_LABEL_LENGTH}.`);
        }
        else if (!LABEL_REGEX.test(metric.dynamicLabel)) {
            hitError = true;
            yield Context.errorManager.record(metric, ErrorType.InvalidLabel, `Label must be snake_case, got '${metric.dynamicLabel}'.`);
        }
        return (hitError)
            ? combineIdentifierAndLabel(metric.baseIdentifier(), OTHER_LABEL)
            : key;
    });
}
class LabeledMetricType {
    constructor(meta, submetric, labels) {
        return new Proxy(this, {
            get: (_target, label) => {
                if (labels) {
                    return LabeledMetricType.createFromStaticLabel(meta, submetric, labels, label);
                }
                return LabeledMetricType.createFromDynamicLabel(meta, submetric, label);
            }
        });
    }
    static createFromStaticLabel(meta, submetricClass, allowedLabels, label) {
        const adjustedLabel = allowedLabels.includes(label) ? label : OTHER_LABEL;
        const newMeta = Object.assign(Object.assign({}, meta), { name: combineIdentifierAndLabel(meta.name, adjustedLabel) });
        return new submetricClass(newMeta);
    }
    static createFromDynamicLabel(meta, submetricClass, label) {
        const newMeta = Object.assign(Object.assign({}, meta), { dynamicLabel: label });
        return new submetricClass(newMeta);
    }
}
/* harmony default export */ const labeled = (LabeledMetricType);

;// CONCATENATED MODULE: ./src/core/metrics/index.ts




class MetricType {
    constructor(type, meta) {
        this.type = type;
        this.name = meta.name;
        this.category = meta.category;
        this.sendInPings = meta.sendInPings;
        this.lifetime = meta.lifetime;
        this.disabled = meta.disabled;
        this.dynamicLabel = meta.dynamicLabel;
    }
    baseIdentifier() {
        if (this.category.length > 0) {
            return `${this.category}.${this.name}`;
        }
        else {
            return this.name;
        }
    }
    identifier() {
        return __awaiter(this, void 0, void 0, function* () {
            const baseIdentifier = this.baseIdentifier();
            if (!isUndefined(this.dynamicLabel)) {
                return yield getValidDynamicLabel(this);
            }
            else {
                return baseIdentifier;
            }
        });
    }
    shouldRecord(uploadEnabled) {
        return (uploadEnabled && !this.disabled);
    }
    testGetNumRecordedErrors(errorType, ping = this.sendInPings[0]) {
        return __awaiter(this, void 0, void 0, function* () {
            return Context.errorManager.testGetNumRecordedErrors(this, errorType, ping);
        });
    }
}

;// CONCATENATED MODULE: ./src/core/metrics/types/boolean.ts





const boolean_LOG_TAG = "core.metrics.BooleanMetricType";
class BooleanMetric extends Metric {
    constructor(v) {
        super(v);
    }
    validate(v) {
        return isBoolean(v);
    }
    payload() {
        return this._inner;
    }
}
class BooleanMetricType extends MetricType {
    constructor(meta) {
        super("boolean", meta);
    }
    set(value) {
        Context.dispatcher.launch(() => __awaiter(this, void 0, void 0, function* () {
            if (!this.shouldRecord(Context.uploadEnabled)) {
                return;
            }
            const metric = new BooleanMetric(value);
            yield Context.metricsDatabase.record(this, metric);
        }), `${boolean_LOG_TAG}.${this.baseIdentifier()}.set`);
    }
    testGetValue(ping = this.sendInPings[0]) {
        return __awaiter(this, void 0, void 0, function* () {
            let metric;
            yield Context.dispatcher.testLaunch(() => __awaiter(this, void 0, void 0, function* () {
                metric = yield Context.metricsDatabase.getMetric(ping, this);
            }));
            return metric;
        });
    }
}
/* harmony default export */ const types_boolean = (BooleanMetricType);

;// CONCATENATED MODULE: ./src/core/metrics/types/counter.ts






const counter_LOG_TAG = "core.metrics.CounterMetricType";
class CounterMetric extends Metric {
    constructor(v) {
        super(v);
    }
    validate(v) {
        if (!isInteger(v)) {
            return false;
        }
        if (v <= 0) {
            return false;
        }
        return true;
    }
    payload() {
        return this._inner;
    }
}
class CounterMetricType extends MetricType {
    constructor(meta) {
        super("counter", meta);
    }
    static _private_addUndispatched(instance, amount) {
        return __awaiter(this, void 0, void 0, function* () {
            if (!instance.shouldRecord(Context.uploadEnabled)) {
                return;
            }
            if (isUndefined(amount)) {
                amount = 1;
            }
            if (amount <= 0) {
                yield Context.errorManager.record(instance, ErrorType.InvalidValue, `Added negative and zero value ${amount}`);
                return;
            }
            const transformFn = ((amount) => {
                return (v) => {
                    let metric;
                    let result;
                    try {
                        metric = new CounterMetric(v);
                        result = metric.get() + amount;
                    }
                    catch (_a) {
                        metric = new CounterMetric(amount);
                        result = amount;
                    }
                    if (result > Number.MAX_SAFE_INTEGER) {
                        result = Number.MAX_SAFE_INTEGER;
                    }
                    metric.set(result);
                    return metric;
                };
            })(amount);
            yield Context.metricsDatabase.transform(instance, transformFn);
        });
    }
    add(amount) {
        Context.dispatcher.launch(() => __awaiter(this, void 0, void 0, function* () { return CounterMetricType._private_addUndispatched(this, amount); }), `${counter_LOG_TAG}.${this.baseIdentifier()}.add`);
    }
    testGetValue(ping = this.sendInPings[0]) {
        return __awaiter(this, void 0, void 0, function* () {
            let metric;
            yield Context.dispatcher.testLaunch(() => __awaiter(this, void 0, void 0, function* () {
                metric = yield Context.metricsDatabase.getMetric(ping, this);
            }));
            return metric;
        });
    }
}
/* harmony default export */ const counter = (CounterMetricType);

;// CONCATENATED MODULE: ./src/core/metrics/time_unit.ts
var TimeUnit;
(function (TimeUnit) {
    TimeUnit["Nanosecond"] = "nanosecond";
    TimeUnit["Microsecond"] = "microsecond";
    TimeUnit["Millisecond"] = "millisecond";
    TimeUnit["Second"] = "second";
    TimeUnit["Minute"] = "minute";
    TimeUnit["Hour"] = "hour";
    TimeUnit["Day"] = "day";
})(TimeUnit || (TimeUnit = {}));
/* harmony default export */ const time_unit = (TimeUnit);

;// CONCATENATED MODULE: ./src/core/metrics/types/datetime.ts






const datetime_LOG_TAG = "core.metrics.DatetimeMetricType";
function formatTimezoneOffset(timezone) {
    const offset = (timezone / 60) * -1;
    const sign = offset > 0 ? "+" : "-";
    const hours = Math.abs(offset).toString().padStart(2, "0");
    return `${sign}${hours}:00`;
}
class DatetimeMetric extends Metric {
    constructor(v) {
        super(v);
    }
    static fromDate(v, timeUnit) {
        return new DatetimeMetric({
            timeUnit,
            timezone: v.getTimezoneOffset(),
            date: v.toISOString()
        });
    }
    get date() {
        return new Date(this._inner.date);
    }
    get timezone() {
        return this._inner.timezone;
    }
    get timeUnit() {
        return this._inner.timeUnit;
    }
    get dateISOString() {
        return this._inner.date;
    }
    validate(v) {
        if (!isObject(v) || Object.keys(v).length !== 3) {
            return false;
        }
        const timeUnitVerification = "timeUnit" in v && isString(v.timeUnit) && Object.values(time_unit).includes(v.timeUnit);
        const timezoneVerification = "timezone" in v && isNumber(v.timezone);
        const dateVerification = "date" in v && isString(v.date) && v.date.length === 24 && !isNaN(Date.parse(v.date));
        if (!timeUnitVerification || !timezoneVerification || !dateVerification) {
            return false;
        }
        return true;
    }
    payload() {
        const extractedDateInfo = this.dateISOString.match(/\d+/g);
        if (!extractedDateInfo || extractedDateInfo.length < 0) {
            throw new Error("IMPOSSIBLE: Unable to extract date information from DatetimeMetric.");
        }
        const correctedDate = new Date(parseInt(extractedDateInfo[0]), parseInt(extractedDateInfo[1]) - 1, parseInt(extractedDateInfo[2]), parseInt(extractedDateInfo[3]) - (this.timezone / 60), parseInt(extractedDateInfo[4]), parseInt(extractedDateInfo[5]), parseInt(extractedDateInfo[6]));
        const timezone = formatTimezoneOffset(this.timezone);
        const year = correctedDate.getFullYear().toString().padStart(2, "0");
        const month = (correctedDate.getMonth() + 1).toString().padStart(2, "0");
        const day = correctedDate.getDate().toString().padStart(2, "0");
        if (this.timeUnit === time_unit.Day) {
            return `${year}-${month}-${day}${timezone}`;
        }
        const hours = correctedDate.getHours().toString().padStart(2, "0");
        if (this.timeUnit === time_unit.Hour) {
            return `${year}-${month}-${day}T${hours}${timezone}`;
        }
        const minutes = correctedDate.getMinutes().toString().padStart(2, "0");
        if (this.timeUnit === time_unit.Minute) {
            return `${year}-${month}-${day}T${hours}:${minutes}${timezone}`;
        }
        const seconds = correctedDate.getSeconds().toString().padStart(2, "0");
        if (this.timeUnit === time_unit.Second) {
            return `${year}-${month}-${day}T${hours}:${minutes}:${seconds}${timezone}`;
        }
        const milliseconds = correctedDate.getMilliseconds().toString().padStart(3, "0");
        if (this.timeUnit === time_unit.Millisecond) {
            return `${year}-${month}-${day}T${hours}:${minutes}:${seconds}.${milliseconds}${timezone}`;
        }
        if (this.timeUnit === time_unit.Microsecond) {
            return `${year}-${month}-${day}T${hours}:${minutes}:${seconds}.${milliseconds}000${timezone}`;
        }
        return `${year}-${month}-${day}T${hours}:${minutes}:${seconds}.${milliseconds}000000${timezone}`;
    }
}
class DatetimeMetricType extends MetricType {
    constructor(meta, timeUnit) {
        super("datetime", meta);
        this.timeUnit = timeUnit;
    }
    static _private_setUndispatched(instance, value) {
        return __awaiter(this, void 0, void 0, function* () {
            if (!instance.shouldRecord(Context.uploadEnabled)) {
                return;
            }
            if (!value) {
                value = new Date();
            }
            const truncatedDate = value;
            switch (instance.timeUnit) {
                case (time_unit.Day):
                    truncatedDate.setMilliseconds(0);
                    truncatedDate.setSeconds(0);
                    truncatedDate.setMinutes(0);
                    truncatedDate.setMilliseconds(0);
                case (time_unit.Hour):
                    truncatedDate.setMilliseconds(0);
                    truncatedDate.setSeconds(0);
                    truncatedDate.setMinutes(0);
                case (time_unit.Minute):
                    truncatedDate.setMilliseconds(0);
                    truncatedDate.setSeconds(0);
                case (time_unit.Second):
                    truncatedDate.setMilliseconds(0);
                default:
                    break;
            }
            const metric = DatetimeMetric.fromDate(value, instance.timeUnit);
            yield Context.metricsDatabase.record(instance, metric);
        });
    }
    set(value) {
        Context.dispatcher.launch(() => DatetimeMetricType._private_setUndispatched(this, value), `${datetime_LOG_TAG}.${this.baseIdentifier()}.set`);
    }
    testGetValueAsDatetimeMetric(ping) {
        return __awaiter(this, void 0, void 0, function* () {
            let value;
            yield Context.dispatcher.testLaunch(() => __awaiter(this, void 0, void 0, function* () {
                value = yield Context.metricsDatabase.getMetric(ping, this);
            }));
            if (value) {
                return new DatetimeMetric(value);
            }
        });
    }
    testGetValueAsString(ping = this.sendInPings[0]) {
        return __awaiter(this, void 0, void 0, function* () {
            const metric = yield this.testGetValueAsDatetimeMetric(ping);
            return metric ? metric.payload() : undefined;
        });
    }
    testGetValue(ping = this.sendInPings[0]) {
        return __awaiter(this, void 0, void 0, function* () {
            const metric = yield this.testGetValueAsDatetimeMetric(ping);
            return metric ? metric.date : undefined;
        });
    }
}
/* harmony default export */ const datetime = (DatetimeMetricType);

;// CONCATENATED MODULE: ./src/core/metrics/types/quantity.ts






const quantity_LOG_TAG = "core.metrics.QuantityMetricType";
class QuantityMetric extends Metric {
    constructor(v) {
        super(v);
    }
    validate(v) {
        if (!isInteger(v)) {
            return false;
        }
        if (v < 0) {
            return false;
        }
        return true;
    }
    payload() {
        return this._inner;
    }
}
class QuantityMetricType extends MetricType {
    constructor(meta) {
        super("quantity", meta);
    }
    static _private_setUndispatched(instance, value) {
        return __awaiter(this, void 0, void 0, function* () {
            if (!instance.shouldRecord(Context.uploadEnabled)) {
                return;
            }
            if (value < 0) {
                yield Context.errorManager.record(instance, ErrorType.InvalidValue, `Set negative value ${value}`);
                return;
            }
            if (value > Number.MAX_SAFE_INTEGER) {
                value = Number.MAX_SAFE_INTEGER;
            }
            const metric = new QuantityMetric(value);
            yield Context.metricsDatabase.record(instance, metric);
        });
    }
    set(value) {
        Context.dispatcher.launch(() => QuantityMetricType._private_setUndispatched(this, value), `${quantity_LOG_TAG}.${this.baseIdentifier()}.set`);
    }
    testGetValue(ping = this.sendInPings[0]) {
        return __awaiter(this, void 0, void 0, function* () {
            let metric;
            yield Context.dispatcher.testLaunch(() => __awaiter(this, void 0, void 0, function* () {
                metric = yield Context.metricsDatabase.getMetric(ping, this);
            }));
            return metric;
        });
    }
}
/* harmony default export */ const quantity = (QuantityMetricType);

;// CONCATENATED MODULE: ./src/core/metrics/types/string.ts





const string_LOG_TAG = "core.metrics.StringMetricType";
const MAX_LENGTH_VALUE = 100;
class StringMetric extends Metric {
    constructor(v) {
        super(v);
    }
    validate(v) {
        if (!isString(v)) {
            return false;
        }
        if (v.length > MAX_LENGTH_VALUE) {
            return false;
        }
        return true;
    }
    payload() {
        return this._inner;
    }
}
class StringMetricType extends MetricType {
    constructor(meta) {
        super("string", meta);
    }
    static _private_setUndispatched(instance, value) {
        return __awaiter(this, void 0, void 0, function* () {
            if (!instance.shouldRecord(Context.uploadEnabled)) {
                return;
            }
            const truncatedValue = yield truncateStringAtBoundaryWithError(instance, value, MAX_LENGTH_VALUE);
            const metric = new StringMetric(truncatedValue);
            yield Context.metricsDatabase.record(instance, metric);
        });
    }
    set(value) {
        Context.dispatcher.launch(() => StringMetricType._private_setUndispatched(this, value), `${string_LOG_TAG}.${this.baseIdentifier()}.set`);
    }
    testGetValue(ping = this.sendInPings[0]) {
        return __awaiter(this, void 0, void 0, function* () {
            let metric;
            yield Context.dispatcher.testLaunch(() => __awaiter(this, void 0, void 0, function* () {
                metric = yield Context.metricsDatabase.getMetric(ping, this);
            }));
            return metric;
        });
    }
}
/* harmony default export */ const string = (StringMetricType);

;// CONCATENATED MODULE: ./src/core/metrics/types/string_list.ts






const string_list_LOG_TAG = "core.metrics.StringListMetricType";
const MAX_LIST_LENGTH = 20;
const MAX_STRING_LENGTH = 50;
class StringListMetric extends Metric {
    constructor(v) {
        super(v);
    }
    validate(v) {
        if (!Array.isArray(v)) {
            return false;
        }
        if (v.length > MAX_LIST_LENGTH) {
            return false;
        }
        for (const s of v) {
            if (!isString(s) || s.length > MAX_STRING_LENGTH) {
                return false;
            }
        }
        return true;
    }
    payload() {
        return this._inner;
    }
}
class StringListMetricType extends MetricType {
    constructor(meta) {
        super("string_list", meta);
    }
    set(value) {
        Context.dispatcher.launch(() => __awaiter(this, void 0, void 0, function* () {
            if (!this.shouldRecord(Context.uploadEnabled)) {
                return;
            }
            const truncatedList = [];
            if (value.length > MAX_LIST_LENGTH) {
                yield Context.errorManager.record(this, ErrorType.InvalidValue, `String list length of ${value.length} exceeds maximum of ${MAX_LIST_LENGTH}.`);
            }
            for (let i = 0; i < Math.min(value.length, MAX_LIST_LENGTH); ++i) {
                const truncatedString = yield truncateStringAtBoundaryWithError(this, value[i], MAX_STRING_LENGTH);
                truncatedList.push(truncatedString);
            }
            const metric = new StringListMetric(truncatedList);
            yield Context.metricsDatabase.record(this, metric);
        }), `${string_list_LOG_TAG}.${this.baseIdentifier()}.set`);
    }
    add(value) {
        Context.dispatcher.launch(() => __awaiter(this, void 0, void 0, function* () {
            if (!this.shouldRecord(Context.uploadEnabled)) {
                return;
            }
            const truncatedValue = yield truncateStringAtBoundaryWithError(this, value, MAX_STRING_LENGTH);
            let currentLen = 0;
            const transformFn = ((value) => {
                return (v) => {
                    let metric;
                    let result;
                    try {
                        metric = new StringListMetric(v);
                        result = metric.get();
                        currentLen = result.length;
                        if (result.length < MAX_LIST_LENGTH) {
                            result.push(value);
                        }
                    }
                    catch (_a) {
                        metric = new StringListMetric([value]);
                        result = [value];
                    }
                    metric.set(result);
                    return metric;
                };
            })(truncatedValue);
            yield Context.metricsDatabase.transform(this, transformFn);
            if (currentLen >= MAX_LIST_LENGTH) {
                yield Context.errorManager.record(this, ErrorType.InvalidValue, `String list length of ${currentLen + 1} exceeds maximum of ${MAX_LIST_LENGTH}.`);
            }
        }), `${string_list_LOG_TAG}.${this.baseIdentifier()}.add`);
    }
    testGetValue(ping = this.sendInPings[0]) {
        return __awaiter(this, void 0, void 0, function* () {
            let metric;
            yield Context.dispatcher.testLaunch(() => __awaiter(this, void 0, void 0, function* () {
                metric = yield Context.metricsDatabase.getMetric(ping, this);
            }));
            return metric;
        });
    }
}
/* harmony default export */ const string_list = (StringListMetricType);

;// CONCATENATED MODULE: ./src/core/metrics/types/text.ts





const text_LOG_TAG = "core.metrics.TextMetricType";
const TEXT_MAX_LENGTH = 200 * 1024;
class TextMetric extends Metric {
    constructor(v) {
        super(v);
    }
    validate(v) {
        if (!isString(v)) {
            return false;
        }
        if (v.length > TEXT_MAX_LENGTH) {
            return false;
        }
        return true;
    }
    payload() {
        return this._inner;
    }
}
class TextMetricType extends MetricType {
    constructor(meta) {
        super("text", meta);
    }
    set(text) {
        Context.dispatcher.launch(() => __awaiter(this, void 0, void 0, function* () {
            if (!this.shouldRecord(Context.uploadEnabled)) {
                return;
            }
            const truncatedValue = yield truncateStringAtBoundaryWithError(this, text, TEXT_MAX_LENGTH);
            const metric = new TextMetric(truncatedValue);
            yield Context.metricsDatabase.record(this, metric);
        }), `${text_LOG_TAG}.${this.baseIdentifier()}.set`);
    }
    testGetValue(ping = this.sendInPings[0]) {
        return __awaiter(this, void 0, void 0, function* () {
            let metric;
            yield Context.dispatcher.testLaunch(() => __awaiter(this, void 0, void 0, function* () {
                metric = yield Context.metricsDatabase.getMetric(ping, this);
            }));
            return metric;
        });
    }
}
/* harmony default export */ const types_text = (TextMetricType);

;// CONCATENATED MODULE: ./src/core/metrics/types/timespan.ts







const timespan_LOG_TAG = "core.metrics.TimespanMetricType";
class TimespanMetric extends Metric {
    constructor(v) {
        super(v);
    }
    get timespan() {
        switch (this._inner.timeUnit) {
            case time_unit.Nanosecond:
                return this._inner.timespan * 10 ** 6;
            case time_unit.Microsecond:
                return this._inner.timespan * 10 ** 3;
            case time_unit.Millisecond:
                return this._inner.timespan;
            case time_unit.Second:
                return Math.round(this._inner.timespan / 1000);
            case time_unit.Minute:
                return Math.round(this._inner.timespan / 1000 / 60);
            case time_unit.Hour:
                return Math.round(this._inner.timespan / 1000 / 60 / 60);
            case time_unit.Day:
                return Math.round(this._inner.timespan / 1000 / 60 / 60 / 24);
        }
    }
    validate(v) {
        if (!isObject(v) || Object.keys(v).length !== 2) {
            return false;
        }
        const timeUnitVerification = "timeUnit" in v && isString(v.timeUnit) && Object.values(time_unit).includes(v.timeUnit);
        const timespanVerification = "timespan" in v && isNumber(v.timespan) && v.timespan >= 0;
        if (!timeUnitVerification || !timespanVerification) {
            return false;
        }
        return true;
    }
    payload() {
        return {
            time_unit: this._inner.timeUnit,
            value: this.timespan
        };
    }
}
class TimespanMetricType extends MetricType {
    constructor(meta, timeUnit) {
        super("timespan", meta);
        this.timeUnit = timeUnit;
    }
    static _private_setRawUndispatched(instance, elapsed) {
        return __awaiter(this, void 0, void 0, function* () {
            if (!instance.shouldRecord(Context.uploadEnabled)) {
                return;
            }
            if (!isUndefined(instance.startTime)) {
                yield Context.errorManager.record(instance, ErrorType.InvalidState, "Timespan already running. Raw value not recorded.");
                return;
            }
            let reportValueExists = false;
            const transformFn = ((elapsed) => {
                return (old) => {
                    let metric;
                    try {
                        metric = new TimespanMetric(old);
                        reportValueExists = true;
                    }
                    catch (_a) {
                        metric = new TimespanMetric({
                            timespan: elapsed,
                            timeUnit: instance.timeUnit,
                        });
                    }
                    return metric;
                };
            })(elapsed);
            yield Context.metricsDatabase.transform(instance, transformFn);
            if (reportValueExists) {
                yield Context.errorManager.record(instance, ErrorType.InvalidState, "Timespan value already recorded. New value discarded.");
            }
        });
    }
    start() {
        const startTime = getMonotonicNow();
        Context.dispatcher.launch(() => __awaiter(this, void 0, void 0, function* () {
            if (!this.shouldRecord(Context.uploadEnabled)) {
                return;
            }
            if (!isUndefined(this.startTime)) {
                yield Context.errorManager.record(this, ErrorType.InvalidState, "Timespan already started");
                return;
            }
            this.startTime = startTime;
            return Promise.resolve();
        }), `${timespan_LOG_TAG}.${this.baseIdentifier()}.start`);
    }
    stop() {
        const stopTime = getMonotonicNow();
        Context.dispatcher.launch(() => __awaiter(this, void 0, void 0, function* () {
            if (!this.shouldRecord(Context.uploadEnabled)) {
                this.startTime = undefined;
                return;
            }
            if (isUndefined(this.startTime)) {
                yield Context.errorManager.record(this, ErrorType.InvalidState, "Timespan not running.");
                return;
            }
            const elapsed = stopTime - this.startTime;
            this.startTime = undefined;
            if (elapsed < 0) {
                yield Context.errorManager.record(this, ErrorType.InvalidState, "Timespan was negative.");
                return;
            }
            yield TimespanMetricType._private_setRawUndispatched(this, elapsed);
        }), `${timespan_LOG_TAG}.${this.baseIdentifier()}.stop`);
    }
    cancel() {
        Context.dispatcher.launch(() => {
            this.startTime = undefined;
            return Promise.resolve();
        }, `${timespan_LOG_TAG}.${this.baseIdentifier()}.cancel`);
    }
    setRawNanos(elapsed) {
        Context.dispatcher.launch(() => __awaiter(this, void 0, void 0, function* () {
            const elapsedMillis = elapsed * 10 ** (-6);
            yield TimespanMetricType._private_setRawUndispatched(this, elapsedMillis);
        }), `${timespan_LOG_TAG}.${this.baseIdentifier()}.setRawNanos`);
    }
    testGetValue(ping = this.sendInPings[0]) {
        return __awaiter(this, void 0, void 0, function* () {
            let value;
            yield Context.dispatcher.testLaunch(() => __awaiter(this, void 0, void 0, function* () {
                value = yield Context.metricsDatabase.getMetric(ping, this);
            }));
            if (value) {
                return (new TimespanMetric(value)).timespan;
            }
        });
    }
}
/* harmony default export */ const timespan = (TimespanMetricType);

;// CONCATENATED MODULE: ./src/core/metrics/types/url.ts






const url_LOG_TAG = "core.metrics.URLMetricType";
const URL_MAX_LENGTH = 2048;
const URL_VALIDATION_REGEX = /^[a-zA-Z][a-zA-Z0-9-\+\.]*:(.*)$/;
class UrlMetricError extends Error {
    constructor(type, message) {
        super(message);
        this.type = type;
        this.name = "UrlMetricError";
    }
}
class UrlMetric extends Metric {
    constructor(v) {
        super(v);
    }
    validate(v) {
        if (!isString(v)) {
            return false;
        }
        if (v.length > URL_MAX_LENGTH) {
            throw new UrlMetricError(ErrorType.InvalidOverflow, `URL length ${v.length} exceeds maximum of ${URL_MAX_LENGTH}.`);
        }
        if (v.startsWith("data:")) {
            throw new UrlMetricError(ErrorType.InvalidValue, "URL metric does not support data URLs.");
        }
        if (!URL_VALIDATION_REGEX.test(v)) {
            throw new UrlMetricError(ErrorType.InvalidValue, `"${v}" does not start with a valid URL scheme.`);
        }
        return true;
    }
    payload() {
        return encodeURI(this._inner);
    }
}
class UrlMetricType extends MetricType {
    constructor(meta) {
        super("url", meta);
    }
    set(url) {
        Context.dispatcher.launch(() => __awaiter(this, void 0, void 0, function* () {
            if (!this.shouldRecord(Context.uploadEnabled)) {
                return;
            }
            try {
                const metric = new UrlMetric(url);
                yield Context.metricsDatabase.record(this, metric);
            }
            catch (e) {
                if (e instanceof UrlMetricError) {
                    yield Context.errorManager.record(this, e.type, e.message);
                }
            }
        }), `${url_LOG_TAG}.${this.baseIdentifier()}.set`);
    }
    setUrl(url) {
        this.set(url.toString());
    }
    testGetValue(ping = this.sendInPings[0]) {
        return __awaiter(this, void 0, void 0, function* () {
            let metric;
            yield Context.dispatcher.testLaunch(() => __awaiter(this, void 0, void 0, function* () {
                metric = yield Context.metricsDatabase.getMetric(ping, this);
            }));
            return metric;
        });
    }
}
/* harmony default export */ const url = (UrlMetricType);

;// CONCATENATED MODULE: ./src/core/metrics/types/uuid.ts






const uuid_LOG_TAG = "core.metrics.UUIDMetricType";
const UUID_REGEX = /^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/i;
class UUIDMetric extends Metric {
    constructor(v) {
        super(v);
    }
    validate(v) {
        if (!isString(v)) {
            return false;
        }
        return UUID_REGEX.test(v);
    }
    payload() {
        return this._inner;
    }
}
class UUIDMetricType extends MetricType {
    constructor(meta) {
        super("uuid", meta);
    }
    static _private_setUndispatched(instance, value) {
        return __awaiter(this, void 0, void 0, function* () {
            if (!instance.shouldRecord(Context.uploadEnabled)) {
                return;
            }
            if (!value) {
                value = generateUUIDv4();
            }
            let metric;
            try {
                metric = new UUIDMetric(value);
            }
            catch (_a) {
                yield Context.errorManager.record(instance, ErrorType.InvalidValue, `"${value}" is not a valid UUID.`);
                return;
            }
            yield Context.metricsDatabase.record(instance, metric);
        });
    }
    set(value) {
        Context.dispatcher.launch(() => UUIDMetricType._private_setUndispatched(this, value), `${uuid_LOG_TAG}.${this.baseIdentifier()}.set`);
    }
    generateAndSet() {
        if (!this.shouldRecord(Context.uploadEnabled)) {
            return;
        }
        const value = generateUUIDv4();
        this.set(value);
        return value;
    }
    testGetValue(ping = this.sendInPings[0]) {
        return __awaiter(this, void 0, void 0, function* () {
            let metric;
            yield Context.dispatcher.testLaunch(() => __awaiter(this, void 0, void 0, function* () {
                metric = yield Context.metricsDatabase.getMetric(ping, this);
            }));
            return metric;
        });
    }
}
/* harmony default export */ const uuid = (UUIDMetricType);

;// CONCATENATED MODULE: ./src/core/metrics/utils.ts











const METRIC_MAP = Object.freeze({
    "boolean": BooleanMetric,
    "counter": CounterMetric,
    "datetime": DatetimeMetric,
    "labeled_boolean": LabeledMetric,
    "labeled_counter": LabeledMetric,
    "labeled_string": LabeledMetric,
    "quantity": QuantityMetric,
    "string": StringMetric,
    "string_list": StringListMetric,
    "text": TextMetric,
    "timespan": TimespanMetric,
    "url": UrlMetric,
    "uuid": UUIDMetric,
});
function createMetric(type, v) {
    if (!(type in METRIC_MAP)) {
        throw new Error(`Unable to create metric of unknown type ${type}`);
    }
    return new METRIC_MAP[type](v);
}
function validateMetricInternalRepresentation(type, v) {
    try {
        createMetric(type, v);
        return true;
    }
    catch (_a) {
        return false;
    }
}

;// CONCATENATED MODULE: ./src/core/metrics/database.ts




const database_LOG_TAG = "core.Metrics.Database";
const RESERVED_METRIC_NAME_PREFIX = "reserved#";
const RESERVED_METRIC_IDENTIFIER_PREFIX = `glean.${RESERVED_METRIC_NAME_PREFIX}`;
function generateReservedMetricIdentifiers(name) {
    return {
        category: "glean",
        name: `${RESERVED_METRIC_NAME_PREFIX}${name}`
    };
}
function isValidInternalMetricsRepresentation(v) {
    if (isObject(v)) {
        for (const metricType in v) {
            const metrics = v[metricType];
            if (isObject(metrics)) {
                for (const metricIdentifier in metrics) {
                    if (!validateMetricInternalRepresentation(metricType, metrics[metricIdentifier])) {
                        log(database_LOG_TAG, `Invalid metric representation found for metric "${metricIdentifier}"`, LoggingLevel.Debug);
                        return false;
                    }
                }
            }
            else {
                return false;
            }
        }
        return true;
    }
    else {
        return false;
    }
}
function createMetricsPayload(v) {
    const result = {};
    for (const metricType in v) {
        const metrics = v[metricType];
        result[metricType] = {};
        for (const metricIdentifier in metrics) {
            const metric = createMetric(metricType, metrics[metricIdentifier]);
            result[metricType][metricIdentifier] = metric.payload();
        }
    }
    return result;
}
class MetricsDatabase {
    constructor(storage) {
        this.userStore = new storage("userLifetimeMetrics");
        this.pingStore = new storage("pingLifetimeMetrics");
        this.appStore = new storage("appLifetimeMetrics");
    }
    _chooseStore(lifetime) {
        switch (lifetime) {
            case "user":
                return this.userStore;
            case "ping":
                return this.pingStore;
            case "application":
                return this.appStore;
        }
    }
    record(metric, value) {
        return __awaiter(this, void 0, void 0, function* () {
            yield this.transform(metric, () => value);
        });
    }
    transform(metric, transformFn) {
        return __awaiter(this, void 0, void 0, function* () {
            if (metric.disabled) {
                return;
            }
            const store = this._chooseStore(metric.lifetime);
            const storageKey = yield metric.identifier();
            for (const ping of metric.sendInPings) {
                const finalTransformFn = (v) => transformFn(v).get();
                yield store.update([ping, metric.type, storageKey], finalTransformFn);
            }
        });
    }
    hasMetric(lifetime, ping, metricType, metricIdentifier) {
        return __awaiter(this, void 0, void 0, function* () {
            const store = this._chooseStore(lifetime);
            const value = yield store.get([ping, metricType, metricIdentifier]);
            return !isUndefined(value);
        });
    }
    countByBaseIdentifier(lifetime, ping, metricType, metricIdentifier) {
        return __awaiter(this, void 0, void 0, function* () {
            const store = this._chooseStore(lifetime);
            const pingStorage = yield store.get([ping, metricType]);
            if (isUndefined(pingStorage)) {
                return 0;
            }
            return Object.keys(pingStorage).filter(n => n.startsWith(metricIdentifier)).length;
        });
    }
    getMetric(ping, metric) {
        return __awaiter(this, void 0, void 0, function* () {
            const store = this._chooseStore(metric.lifetime);
            const storageKey = yield metric.identifier();
            const value = yield store.get([ping, metric.type, storageKey]);
            if (!isUndefined(value) && !validateMetricInternalRepresentation(metric.type, value)) {
                log(database_LOG_TAG, `Unexpected value found for metric ${storageKey}: ${JSON.stringify(value)}. Clearing.`, LoggingLevel.Error);
                yield store.delete([ping, metric.type, storageKey]);
                return;
            }
            else {
                return value;
            }
        });
    }
    getAndValidatePingData(ping, lifetime) {
        return __awaiter(this, void 0, void 0, function* () {
            const store = this._chooseStore(lifetime);
            const data = yield store.get([ping]);
            if (isUndefined(data)) {
                return {};
            }
            if (!isValidInternalMetricsRepresentation(data)) {
                log(database_LOG_TAG, `Unexpected value found for ping "${ping}" in "${lifetime}" store: ${JSON.stringify(data)}. Clearing.`, LoggingLevel.Debug);
                yield store.delete([ping]);
                return {};
            }
            return data;
        });
    }
    processLabeledMetric(snapshot, metricType, metricId, metricData) {
        const newType = `labeled_${metricType}`;
        const idLabelSplit = metricId.split("/", 2);
        const newId = idLabelSplit[0];
        const label = idLabelSplit[1];
        if (newType in snapshot && newId in snapshot[newType]) {
            const existingData = snapshot[newType][newId];
            snapshot[newType][newId] = Object.assign(Object.assign({}, existingData), { [label]: metricData });
        }
        else {
            snapshot[newType] = Object.assign(Object.assign({}, snapshot[newType]), { [newId]: {
                    [label]: metricData
                } });
        }
    }
    getPingMetrics(ping, clearPingLifetimeData) {
        return __awaiter(this, void 0, void 0, function* () {
            const userData = yield this.getAndValidatePingData(ping, "user");
            const pingData = yield this.getAndValidatePingData(ping, "ping");
            const appData = yield this.getAndValidatePingData(ping, "application");
            if (clearPingLifetimeData && Object.keys(pingData).length > 0) {
                yield this.clear("ping", ping);
            }
            const response = {};
            for (const data of [userData, pingData, appData]) {
                for (const metricType in data) {
                    for (const metricId in data[metricType]) {
                        if (!metricId.startsWith(RESERVED_METRIC_IDENTIFIER_PREFIX)) {
                            if (metricId.includes("/")) {
                                this.processLabeledMetric(response, metricType, metricId, data[metricType][metricId]);
                            }
                            else {
                                response[metricType] = Object.assign(Object.assign({}, response[metricType]), { [metricId]: data[metricType][metricId] });
                            }
                        }
                    }
                }
            }
            if (Object.keys(response).length === 0) {
                return;
            }
            else {
                return createMetricsPayload(response);
            }
        });
    }
    clear(lifetime, ping) {
        return __awaiter(this, void 0, void 0, function* () {
            const store = this._chooseStore(lifetime);
            const storageIndex = ping ? [ping] : [];
            yield store.delete(storageIndex);
        });
    }
    clearAll() {
        return __awaiter(this, void 0, void 0, function* () {
            yield this.userStore.delete([]);
            yield this.pingStore.delete([]);
            yield this.appStore.delete([]);
        });
    }
}
/* harmony default export */ const database = (MetricsDatabase);

;// CONCATENATED MODULE: ./node_modules/fflate/esm/browser.js
// DEFLATE is a complex format; to read this code, you should probably check the RFC first:
// https://tools.ietf.org/html/rfc1951
// You may also wish to take a look at the guide I made about this program:
// https://gist.github.com/101arrowz/253f31eb5abc3d9275ab943003ffecad
// Some of the following code is similar to that of UZIP.js:
// https://github.com/photopea/UZIP.js
// However, the vast majority of the codebase has diverged from UZIP.js to increase performance and reduce bundle size.
// Sometimes 0 will appear where -1 would be more appropriate. This is because using a uint
// is better for memory in most engines (I *think*).
var ch2 = {};
var wk = (function (c, id, msg, transfer, cb) {
    var w = new Worker(ch2[id] || (ch2[id] = URL.createObjectURL(new Blob([
        c + ';addEventListener("error",function(e){e=e.error;postMessage({$e$:[e.message,e.code,e.stack]})})'
    ], { type: 'text/javascript' }))));
    w.onmessage = function (e) {
        var d = e.data, ed = d.$e$;
        if (ed) {
            var err = new Error(ed[0]);
            err['code'] = ed[1];
            err.stack = ed[2];
            cb(err, null);
        }
        else
            cb(null, d);
    };
    w.postMessage(msg, transfer);
    return w;
});

// aliases for shorter compressed code (most minifers don't do this)
var u8 = Uint8Array, u16 = Uint16Array, u32 = Uint32Array;
// fixed length extra bits
var fleb = new u8([0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, /* unused */ 0, 0, /* impossible */ 0]);
// fixed distance extra bits
// see fleb note
var fdeb = new u8([0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, /* unused */ 0, 0]);
// code length index map
var clim = new u8([16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15]);
// get base, reverse index map from extra bits
var freb = function (eb, start) {
    var b = new u16(31);
    for (var i = 0; i < 31; ++i) {
        b[i] = start += 1 << eb[i - 1];
    }
    // numbers here are at max 18 bits
    var r = new u32(b[30]);
    for (var i = 1; i < 30; ++i) {
        for (var j = b[i]; j < b[i + 1]; ++j) {
            r[j] = ((j - b[i]) << 5) | i;
        }
    }
    return [b, r];
};
var _a = freb(fleb, 2), fl = _a[0], revfl = _a[1];
// we can ignore the fact that the other numbers are wrong; they never happen anyway
fl[28] = 258, revfl[258] = 28;
var _b = freb(fdeb, 0), fd = _b[0], revfd = _b[1];
// map of value to reverse (assuming 16 bits)
var rev = new u16(32768);
for (var browser_i = 0; browser_i < 32768; ++browser_i) {
    // reverse table algorithm from SO
    var x = ((browser_i & 0xAAAA) >>> 1) | ((browser_i & 0x5555) << 1);
    x = ((x & 0xCCCC) >>> 2) | ((x & 0x3333) << 2);
    x = ((x & 0xF0F0) >>> 4) | ((x & 0x0F0F) << 4);
    rev[browser_i] = (((x & 0xFF00) >>> 8) | ((x & 0x00FF) << 8)) >>> 1;
}
// create huffman tree from u8 "map": index -> code length for code index
// mb (max bits) must be at most 15
// TODO: optimize/split up?
var hMap = (function (cd, mb, r) {
    var s = cd.length;
    // index
    var i = 0;
    // u16 "map": index -> # of codes with bit length = index
    var l = new u16(mb);
    // length of cd must be 288 (total # of codes)
    for (; i < s; ++i)
        ++l[cd[i] - 1];
    // u16 "map": index -> minimum code for bit length = index
    var le = new u16(mb);
    for (i = 0; i < mb; ++i) {
        le[i] = (le[i - 1] + l[i - 1]) << 1;
    }
    var co;
    if (r) {
        // u16 "map": index -> number of actual bits, symbol for code
        co = new u16(1 << mb);
        // bits to remove for reverser
        var rvb = 15 - mb;
        for (i = 0; i < s; ++i) {
            // ignore 0 lengths
            if (cd[i]) {
                // num encoding both symbol and bits read
                var sv = (i << 4) | cd[i];
                // free bits
                var r_1 = mb - cd[i];
                // start value
                var v = le[cd[i] - 1]++ << r_1;
                // m is end value
                for (var m = v | ((1 << r_1) - 1); v <= m; ++v) {
                    // every 16 bit value starting with the code yields the same result
                    co[rev[v] >>> rvb] = sv;
                }
            }
        }
    }
    else {
        co = new u16(s);
        for (i = 0; i < s; ++i) {
            if (cd[i]) {
                co[i] = rev[le[cd[i] - 1]++] >>> (15 - cd[i]);
            }
        }
    }
    return co;
});
// fixed length tree
var flt = new u8(288);
for (var browser_i = 0; browser_i < 144; ++browser_i)
    flt[browser_i] = 8;
for (var browser_i = 144; browser_i < 256; ++browser_i)
    flt[browser_i] = 9;
for (var browser_i = 256; browser_i < 280; ++browser_i)
    flt[browser_i] = 7;
for (var browser_i = 280; browser_i < 288; ++browser_i)
    flt[browser_i] = 8;
// fixed distance tree
var fdt = new u8(32);
for (var browser_i = 0; browser_i < 32; ++browser_i)
    fdt[browser_i] = 5;
// fixed length map
var flm = /*#__PURE__*/ hMap(flt, 9, 0), flrm = /*#__PURE__*/ (/* unused pure expression or super */ null && (hMap(flt, 9, 1)));
// fixed distance map
var fdm = /*#__PURE__*/ hMap(fdt, 5, 0), fdrm = /*#__PURE__*/ (/* unused pure expression or super */ null && (hMap(fdt, 5, 1)));
// find max of array
var max = function (a) {
    var m = a[0];
    for (var i = 1; i < a.length; ++i) {
        if (a[i] > m)
            m = a[i];
    }
    return m;
};
// read d, starting at bit p and mask with m
var bits = function (d, p, m) {
    var o = (p / 8) | 0;
    return ((d[o] | (d[o + 1] << 8)) >> (p & 7)) & m;
};
// read d, starting at bit p continuing for at least 16 bits
var bits16 = function (d, p) {
    var o = (p / 8) | 0;
    return ((d[o] | (d[o + 1] << 8) | (d[o + 2] << 16)) >> (p & 7));
};
// get end of byte
var shft = function (p) { return ((p + 7) / 8) | 0; };
// typed array slice - allows garbage collector to free original reference,
// while being more compatible than .slice
var slc = function (v, s, e) {
    if (s == null || s < 0)
        s = 0;
    if (e == null || e > v.length)
        e = v.length;
    // can't use .constructor in case user-supplied
    var n = new (v instanceof u16 ? u16 : v instanceof u32 ? u32 : u8)(e - s);
    n.set(v.subarray(s, e));
    return n;
};
/**
 * Codes for errors generated within this library
 */
var FlateErrorCode = {
    UnexpectedEOF: 0,
    InvalidBlockType: 1,
    InvalidLengthLiteral: 2,
    InvalidDistance: 3,
    StreamFinished: 4,
    NoStreamHandler: 5,
    InvalidHeader: 6,
    NoCallback: 7,
    InvalidUTF8: 8,
    ExtraFieldTooLong: 9,
    InvalidDate: 10,
    FilenameTooLong: 11,
    StreamFinishing: 12,
    InvalidZipData: 13,
    UnknownCompressionMethod: 14
};
// error codes
var ec = [
    'unexpected EOF',
    'invalid block type',
    'invalid length/literal',
    'invalid distance',
    'stream finished',
    'no stream handler',
    ,
    'no callback',
    'invalid UTF-8 data',
    'extra field too long',
    'date not in range 1980-2099',
    'filename too long',
    'stream finishing',
    'invalid zip data'
    // determined by unknown compression method
];
;
var err = function (ind, msg, nt) {
    var e = new Error(msg || ec[ind]);
    e.code = ind;
    if (Error.captureStackTrace)
        Error.captureStackTrace(e, err);
    if (!nt)
        throw e;
    return e;
};
// expands raw DEFLATE data
var inflt = function (dat, buf, st) {
    // source length
    var sl = dat.length;
    if (!sl || (st && st.f && !st.l))
        return buf || new u8(0);
    // have to estimate size
    var noBuf = !buf || st;
    // no state
    var noSt = !st || st.i;
    if (!st)
        st = {};
    // Assumes roughly 33% compression ratio average
    if (!buf)
        buf = new u8(sl * 3);
    // ensure buffer can fit at least l elements
    var cbuf = function (l) {
        var bl = buf.length;
        // need to increase size to fit
        if (l > bl) {
            // Double or set to necessary, whichever is greater
            var nbuf = new u8(Math.max(bl * 2, l));
            nbuf.set(buf);
            buf = nbuf;
        }
    };
    //  last chunk         bitpos           bytes
    var final = st.f || 0, pos = st.p || 0, bt = st.b || 0, lm = st.l, dm = st.d, lbt = st.m, dbt = st.n;
    // total bits
    var tbts = sl * 8;
    do {
        if (!lm) {
            // BFINAL - this is only 1 when last chunk is next
            final = bits(dat, pos, 1);
            // type: 0 = no compression, 1 = fixed huffman, 2 = dynamic huffman
            var type = bits(dat, pos + 1, 3);
            pos += 3;
            if (!type) {
                // go to end of byte boundary
                var s = shft(pos) + 4, l = dat[s - 4] | (dat[s - 3] << 8), t = s + l;
                if (t > sl) {
                    if (noSt)
                        err(0);
                    break;
                }
                // ensure size
                if (noBuf)
                    cbuf(bt + l);
                // Copy over uncompressed data
                buf.set(dat.subarray(s, t), bt);
                // Get new bitpos, update byte count
                st.b = bt += l, st.p = pos = t * 8, st.f = final;
                continue;
            }
            else if (type == 1)
                lm = flrm, dm = fdrm, lbt = 9, dbt = 5;
            else if (type == 2) {
                //  literal                            lengths
                var hLit = bits(dat, pos, 31) + 257, hcLen = bits(dat, pos + 10, 15) + 4;
                var tl = hLit + bits(dat, pos + 5, 31) + 1;
                pos += 14;
                // length+distance tree
                var ldt = new u8(tl);
                // code length tree
                var clt = new u8(19);
                for (var i = 0; i < hcLen; ++i) {
                    // use index map to get real code
                    clt[clim[i]] = bits(dat, pos + i * 3, 7);
                }
                pos += hcLen * 3;
                // code lengths bits
                var clb = max(clt), clbmsk = (1 << clb) - 1;
                // code lengths map
                var clm = hMap(clt, clb, 1);
                for (var i = 0; i < tl;) {
                    var r = clm[bits(dat, pos, clbmsk)];
                    // bits read
                    pos += r & 15;
                    // symbol
                    var s = r >>> 4;
                    // code length to copy
                    if (s < 16) {
                        ldt[i++] = s;
                    }
                    else {
                        //  copy   count
                        var c = 0, n = 0;
                        if (s == 16)
                            n = 3 + bits(dat, pos, 3), pos += 2, c = ldt[i - 1];
                        else if (s == 17)
                            n = 3 + bits(dat, pos, 7), pos += 3;
                        else if (s == 18)
                            n = 11 + bits(dat, pos, 127), pos += 7;
                        while (n--)
                            ldt[i++] = c;
                    }
                }
                //    length tree                 distance tree
                var lt = ldt.subarray(0, hLit), dt = ldt.subarray(hLit);
                // max length bits
                lbt = max(lt);
                // max dist bits
                dbt = max(dt);
                lm = hMap(lt, lbt, 1);
                dm = hMap(dt, dbt, 1);
            }
            else
                err(1);
            if (pos > tbts) {
                if (noSt)
                    err(0);
                break;
            }
        }
        // Make sure the buffer can hold this + the largest possible addition
        // Maximum chunk size (practically, theoretically infinite) is 2^17;
        if (noBuf)
            cbuf(bt + 131072);
        var lms = (1 << lbt) - 1, dms = (1 << dbt) - 1;
        var lpos = pos;
        for (;; lpos = pos) {
            // bits read, code
            var c = lm[bits16(dat, pos) & lms], sym = c >>> 4;
            pos += c & 15;
            if (pos > tbts) {
                if (noSt)
                    err(0);
                break;
            }
            if (!c)
                err(2);
            if (sym < 256)
                buf[bt++] = sym;
            else if (sym == 256) {
                lpos = pos, lm = null;
                break;
            }
            else {
                var add = sym - 254;
                // no extra bits needed if less
                if (sym > 264) {
                    // index
                    var i = sym - 257, b = fleb[i];
                    add = bits(dat, pos, (1 << b) - 1) + fl[i];
                    pos += b;
                }
                // dist
                var d = dm[bits16(dat, pos) & dms], dsym = d >>> 4;
                if (!d)
                    err(3);
                pos += d & 15;
                var dt = fd[dsym];
                if (dsym > 3) {
                    var b = fdeb[dsym];
                    dt += bits16(dat, pos) & ((1 << b) - 1), pos += b;
                }
                if (pos > tbts) {
                    if (noSt)
                        err(0);
                    break;
                }
                if (noBuf)
                    cbuf(bt + 131072);
                var end = bt + add;
                for (; bt < end; bt += 4) {
                    buf[bt] = buf[bt - dt];
                    buf[bt + 1] = buf[bt + 1 - dt];
                    buf[bt + 2] = buf[bt + 2 - dt];
                    buf[bt + 3] = buf[bt + 3 - dt];
                }
                bt = end;
            }
        }
        st.l = lm, st.p = lpos, st.b = bt, st.f = final;
        if (lm)
            final = 1, st.m = lbt, st.d = dm, st.n = dbt;
    } while (!final);
    return bt == buf.length ? buf : slc(buf, 0, bt);
};
// starting at p, write the minimum number of bits that can hold v to d
var wbits = function (d, p, v) {
    v <<= p & 7;
    var o = (p / 8) | 0;
    d[o] |= v;
    d[o + 1] |= v >>> 8;
};
// starting at p, write the minimum number of bits (>8) that can hold v to d
var wbits16 = function (d, p, v) {
    v <<= p & 7;
    var o = (p / 8) | 0;
    d[o] |= v;
    d[o + 1] |= v >>> 8;
    d[o + 2] |= v >>> 16;
};
// creates code lengths from a frequency table
var hTree = function (d, mb) {
    // Need extra info to make a tree
    var t = [];
    for (var i = 0; i < d.length; ++i) {
        if (d[i])
            t.push({ s: i, f: d[i] });
    }
    var s = t.length;
    var t2 = t.slice();
    if (!s)
        return [et, 0];
    if (s == 1) {
        var v = new u8(t[0].s + 1);
        v[t[0].s] = 1;
        return [v, 1];
    }
    t.sort(function (a, b) { return a.f - b.f; });
    // after i2 reaches last ind, will be stopped
    // freq must be greater than largest possible number of symbols
    t.push({ s: -1, f: 25001 });
    var l = t[0], r = t[1], i0 = 0, i1 = 1, i2 = 2;
    t[0] = { s: -1, f: l.f + r.f, l: l, r: r };
    // efficient algorithm from UZIP.js
    // i0 is lookbehind, i2 is lookahead - after processing two low-freq
    // symbols that combined have high freq, will start processing i2 (high-freq,
    // non-composite) symbols instead
    // see https://reddit.com/r/photopea/comments/ikekht/uzipjs_questions/
    while (i1 != s - 1) {
        l = t[t[i0].f < t[i2].f ? i0++ : i2++];
        r = t[i0 != i1 && t[i0].f < t[i2].f ? i0++ : i2++];
        t[i1++] = { s: -1, f: l.f + r.f, l: l, r: r };
    }
    var maxSym = t2[0].s;
    for (var i = 1; i < s; ++i) {
        if (t2[i].s > maxSym)
            maxSym = t2[i].s;
    }
    // code lengths
    var tr = new u16(maxSym + 1);
    // max bits in tree
    var mbt = ln(t[i1 - 1], tr, 0);
    if (mbt > mb) {
        // more algorithms from UZIP.js
        // TODO: find out how this code works (debt)
        //  ind    debt
        var i = 0, dt = 0;
        //    left            cost
        var lft = mbt - mb, cst = 1 << lft;
        t2.sort(function (a, b) { return tr[b.s] - tr[a.s] || a.f - b.f; });
        for (; i < s; ++i) {
            var i2_1 = t2[i].s;
            if (tr[i2_1] > mb) {
                dt += cst - (1 << (mbt - tr[i2_1]));
                tr[i2_1] = mb;
            }
            else
                break;
        }
        dt >>>= lft;
        while (dt > 0) {
            var i2_2 = t2[i].s;
            if (tr[i2_2] < mb)
                dt -= 1 << (mb - tr[i2_2]++ - 1);
            else
                ++i;
        }
        for (; i >= 0 && dt; --i) {
            var i2_3 = t2[i].s;
            if (tr[i2_3] == mb) {
                --tr[i2_3];
                ++dt;
            }
        }
        mbt = mb;
    }
    return [new u8(tr), mbt];
};
// get the max length and assign length codes
var ln = function (n, l, d) {
    return n.s == -1
        ? Math.max(ln(n.l, l, d + 1), ln(n.r, l, d + 1))
        : (l[n.s] = d);
};
// length codes generation
var lc = function (c) {
    var s = c.length;
    // Note that the semicolon was intentional
    while (s && !c[--s])
        ;
    var cl = new u16(++s);
    //  ind      num         streak
    var cli = 0, cln = c[0], cls = 1;
    var w = function (v) { cl[cli++] = v; };
    for (var i = 1; i <= s; ++i) {
        if (c[i] == cln && i != s)
            ++cls;
        else {
            if (!cln && cls > 2) {
                for (; cls > 138; cls -= 138)
                    w(32754);
                if (cls > 2) {
                    w(cls > 10 ? ((cls - 11) << 5) | 28690 : ((cls - 3) << 5) | 12305);
                    cls = 0;
                }
            }
            else if (cls > 3) {
                w(cln), --cls;
                for (; cls > 6; cls -= 6)
                    w(8304);
                if (cls > 2)
                    w(((cls - 3) << 5) | 8208), cls = 0;
            }
            while (cls--)
                w(cln);
            cls = 1;
            cln = c[i];
        }
    }
    return [cl.subarray(0, cli), s];
};
// calculate the length of output from tree, code lengths
var clen = function (cf, cl) {
    var l = 0;
    for (var i = 0; i < cl.length; ++i)
        l += cf[i] * cl[i];
    return l;
};
// writes a fixed block
// returns the new bit pos
var wfblk = function (out, pos, dat) {
    // no need to write 00 as type: TypedArray defaults to 0
    var s = dat.length;
    var o = shft(pos + 2);
    out[o] = s & 255;
    out[o + 1] = s >>> 8;
    out[o + 2] = out[o] ^ 255;
    out[o + 3] = out[o + 1] ^ 255;
    for (var i = 0; i < s; ++i)
        out[o + i + 4] = dat[i];
    return (o + 4 + s) * 8;
};
// writes a block
var wblk = function (dat, out, final, syms, lf, df, eb, li, bs, bl, p) {
    wbits(out, p++, final);
    ++lf[256];
    var _a = hTree(lf, 15), dlt = _a[0], mlb = _a[1];
    var _b = hTree(df, 15), ddt = _b[0], mdb = _b[1];
    var _c = lc(dlt), lclt = _c[0], nlc = _c[1];
    var _d = lc(ddt), lcdt = _d[0], ndc = _d[1];
    var lcfreq = new u16(19);
    for (var i = 0; i < lclt.length; ++i)
        lcfreq[lclt[i] & 31]++;
    for (var i = 0; i < lcdt.length; ++i)
        lcfreq[lcdt[i] & 31]++;
    var _e = hTree(lcfreq, 7), lct = _e[0], mlcb = _e[1];
    var nlcc = 19;
    for (; nlcc > 4 && !lct[clim[nlcc - 1]]; --nlcc)
        ;
    var flen = (bl + 5) << 3;
    var ftlen = clen(lf, flt) + clen(df, fdt) + eb;
    var dtlen = clen(lf, dlt) + clen(df, ddt) + eb + 14 + 3 * nlcc + clen(lcfreq, lct) + (2 * lcfreq[16] + 3 * lcfreq[17] + 7 * lcfreq[18]);
    if (flen <= ftlen && flen <= dtlen)
        return wfblk(out, p, dat.subarray(bs, bs + bl));
    var lm, ll, dm, dl;
    wbits(out, p, 1 + (dtlen < ftlen)), p += 2;
    if (dtlen < ftlen) {
        lm = hMap(dlt, mlb, 0), ll = dlt, dm = hMap(ddt, mdb, 0), dl = ddt;
        var llm = hMap(lct, mlcb, 0);
        wbits(out, p, nlc - 257);
        wbits(out, p + 5, ndc - 1);
        wbits(out, p + 10, nlcc - 4);
        p += 14;
        for (var i = 0; i < nlcc; ++i)
            wbits(out, p + 3 * i, lct[clim[i]]);
        p += 3 * nlcc;
        var lcts = [lclt, lcdt];
        for (var it = 0; it < 2; ++it) {
            var clct = lcts[it];
            for (var i = 0; i < clct.length; ++i) {
                var len = clct[i] & 31;
                wbits(out, p, llm[len]), p += lct[len];
                if (len > 15)
                    wbits(out, p, (clct[i] >>> 5) & 127), p += clct[i] >>> 12;
            }
        }
    }
    else {
        lm = flm, ll = flt, dm = fdm, dl = fdt;
    }
    for (var i = 0; i < li; ++i) {
        if (syms[i] > 255) {
            var len = (syms[i] >>> 18) & 31;
            wbits16(out, p, lm[len + 257]), p += ll[len + 257];
            if (len > 7)
                wbits(out, p, (syms[i] >>> 23) & 31), p += fleb[len];
            var dst = syms[i] & 31;
            wbits16(out, p, dm[dst]), p += dl[dst];
            if (dst > 3)
                wbits16(out, p, (syms[i] >>> 5) & 8191), p += fdeb[dst];
        }
        else {
            wbits16(out, p, lm[syms[i]]), p += ll[syms[i]];
        }
    }
    wbits16(out, p, lm[256]);
    return p + ll[256];
};
// deflate options (nice << 13) | chain
var deo = /*#__PURE__*/ new u32([65540, 131080, 131088, 131104, 262176, 1048704, 1048832, 2114560, 2117632]);
// empty
var et = /*#__PURE__*/ new u8(0);
// compresses data into a raw DEFLATE buffer
var dflt = function (dat, lvl, plvl, pre, post, lst) {
    var s = dat.length;
    var o = new u8(pre + s + 5 * (1 + Math.ceil(s / 7000)) + post);
    // writing to this writes to the output buffer
    var w = o.subarray(pre, o.length - post);
    var pos = 0;
    if (!lvl || s < 8) {
        for (var i = 0; i <= s; i += 65535) {
            // end
            var e = i + 65535;
            if (e < s) {
                // write full block
                pos = wfblk(w, pos, dat.subarray(i, e));
            }
            else {
                // write final block
                w[i] = lst;
                pos = wfblk(w, pos, dat.subarray(i, s));
            }
        }
    }
    else {
        var opt = deo[lvl - 1];
        var n = opt >>> 13, c = opt & 8191;
        var msk_1 = (1 << plvl) - 1;
        //    prev 2-byte val map    curr 2-byte val map
        var prev = new u16(32768), head = new u16(msk_1 + 1);
        var bs1_1 = Math.ceil(plvl / 3), bs2_1 = 2 * bs1_1;
        var hsh = function (i) { return (dat[i] ^ (dat[i + 1] << bs1_1) ^ (dat[i + 2] << bs2_1)) & msk_1; };
        // 24576 is an arbitrary number of maximum symbols per block
        // 424 buffer for last block
        var syms = new u32(25000);
        // length/literal freq   distance freq
        var lf = new u16(288), df = new u16(32);
        //  l/lcnt  exbits  index  l/lind  waitdx  bitpos
        var lc_1 = 0, eb = 0, i = 0, li = 0, wi = 0, bs = 0;
        for (; i < s; ++i) {
            // hash value
            // deopt when i > s - 3 - at end, deopt acceptable
            var hv = hsh(i);
            // index mod 32768    previous index mod
            var imod = i & 32767, pimod = head[hv];
            prev[imod] = pimod;
            head[hv] = imod;
            // We always should modify head and prev, but only add symbols if
            // this data is not yet processed ("wait" for wait index)
            if (wi <= i) {
                // bytes remaining
                var rem = s - i;
                if ((lc_1 > 7000 || li > 24576) && rem > 423) {
                    pos = wblk(dat, w, 0, syms, lf, df, eb, li, bs, i - bs, pos);
                    li = lc_1 = eb = 0, bs = i;
                    for (var j = 0; j < 286; ++j)
                        lf[j] = 0;
                    for (var j = 0; j < 30; ++j)
                        df[j] = 0;
                }
                //  len    dist   chain
                var l = 2, d = 0, ch_1 = c, dif = (imod - pimod) & 32767;
                if (rem > 2 && hv == hsh(i - dif)) {
                    var maxn = Math.min(n, rem) - 1;
                    var maxd = Math.min(32767, i);
                    // max possible length
                    // not capped at dif because decompressors implement "rolling" index population
                    var ml = Math.min(258, rem);
                    while (dif <= maxd && --ch_1 && imod != pimod) {
                        if (dat[i + l] == dat[i + l - dif]) {
                            var nl = 0;
                            for (; nl < ml && dat[i + nl] == dat[i + nl - dif]; ++nl)
                                ;
                            if (nl > l) {
                                l = nl, d = dif;
                                // break out early when we reach "nice" (we are satisfied enough)
                                if (nl > maxn)
                                    break;
                                // now, find the rarest 2-byte sequence within this
                                // length of literals and search for that instead.
                                // Much faster than just using the start
                                var mmd = Math.min(dif, nl - 2);
                                var md = 0;
                                for (var j = 0; j < mmd; ++j) {
                                    var ti = (i - dif + j + 32768) & 32767;
                                    var pti = prev[ti];
                                    var cd = (ti - pti + 32768) & 32767;
                                    if (cd > md)
                                        md = cd, pimod = ti;
                                }
                            }
                        }
                        // check the previous match
                        imod = pimod, pimod = prev[imod];
                        dif += (imod - pimod + 32768) & 32767;
                    }
                }
                // d will be nonzero only when a match was found
                if (d) {
                    // store both dist and len data in one Uint32
                    // Make sure this is recognized as a len/dist with 28th bit (2^28)
                    syms[li++] = 268435456 | (revfl[l] << 18) | revfd[d];
                    var lin = revfl[l] & 31, din = revfd[d] & 31;
                    eb += fleb[lin] + fdeb[din];
                    ++lf[257 + lin];
                    ++df[din];
                    wi = i + l;
                    ++lc_1;
                }
                else {
                    syms[li++] = dat[i];
                    ++lf[dat[i]];
                }
            }
        }
        pos = wblk(dat, w, lst, syms, lf, df, eb, li, bs, i - bs, pos);
        // this is the easiest way to avoid needing to maintain state
        if (!lst && pos & 7)
            pos = wfblk(w, pos + 1, et);
    }
    return slc(o, 0, pre + shft(pos) + post);
};
// CRC32 table
var crct = /*#__PURE__*/ (function () {
    var t = new Int32Array(256);
    for (var i = 0; i < 256; ++i) {
        var c = i, k = 9;
        while (--k)
            c = ((c & 1) && -306674912) ^ (c >>> 1);
        t[i] = c;
    }
    return t;
})();
// CRC32
var crc = function () {
    var c = -1;
    return {
        p: function (d) {
            // closures have awful performance
            var cr = c;
            for (var i = 0; i < d.length; ++i)
                cr = crct[(cr & 255) ^ d[i]] ^ (cr >>> 8);
            c = cr;
        },
        d: function () { return ~c; }
    };
};
// Alder32
var adler = function () {
    var a = 1, b = 0;
    return {
        p: function (d) {
            // closures have awful performance
            var n = a, m = b;
            var l = d.length | 0;
            for (var i = 0; i != l;) {
                var e = Math.min(i + 2655, l);
                for (; i < e; ++i)
                    m += n += d[i];
                n = (n & 65535) + 15 * (n >> 16), m = (m & 65535) + 15 * (m >> 16);
            }
            a = n, b = m;
        },
        d: function () {
            a %= 65521, b %= 65521;
            return (a & 255) << 24 | (a >>> 8) << 16 | (b & 255) << 8 | (b >>> 8);
        }
    };
};
;
// deflate with opts
var dopt = function (dat, opt, pre, post, st) {
    return dflt(dat, opt.level == null ? 6 : opt.level, opt.mem == null ? Math.ceil(Math.max(8, Math.min(13, Math.log(dat.length))) * 1.5) : (12 + opt.mem), pre, post, !st);
};
// Walmart object spread
var mrg = function (a, b) {
    var o = {};
    for (var k in a)
        o[k] = a[k];
    for (var k in b)
        o[k] = b[k];
    return o;
};
// worker clone
// This is possibly the craziest part of the entire codebase, despite how simple it may seem.
// The only parameter to this function is a closure that returns an array of variables outside of the function scope.
// We're going to try to figure out the variable names used in the closure as strings because that is crucial for workerization.
// We will return an object mapping of true variable name to value (basically, the current scope as a JS object).
// The reason we can't just use the original variable names is minifiers mangling the toplevel scope.
// This took me three weeks to figure out how to do.
var wcln = function (fn, fnStr, td) {
    var dt = fn();
    var st = fn.toString();
    var ks = st.slice(st.indexOf('[') + 1, st.lastIndexOf(']')).replace(/ /g, '').split(',');
    for (var i = 0; i < dt.length; ++i) {
        var v = dt[i], k = ks[i];
        if (typeof v == 'function') {
            fnStr += ';' + k + '=';
            var st_1 = v.toString();
            if (v.prototype) {
                // for global objects
                if (st_1.indexOf('[native code]') != -1) {
                    var spInd = st_1.indexOf(' ', 8) + 1;
                    fnStr += st_1.slice(spInd, st_1.indexOf('(', spInd));
                }
                else {
                    fnStr += st_1;
                    for (var t in v.prototype)
                        fnStr += ';' + k + '.prototype.' + t + '=' + v.prototype[t].toString();
                }
            }
            else
                fnStr += st_1;
        }
        else
            td[k] = v;
    }
    return [fnStr, td];
};
var ch = (/* unused pure expression or super */ null && ([]));
// clone bufs
var cbfs = function (v) {
    var tl = [];
    for (var k in v) {
        if (v[k] instanceof u8 || v[k] instanceof u16 || v[k] instanceof u32)
            tl.push((v[k] = new v[k].constructor(v[k])).buffer);
    }
    return tl;
};
// use a worker to execute code
var wrkr = function (fns, init, id, cb) {
    var _a;
    if (!ch[id]) {
        var fnStr = '', td_1 = {}, m = fns.length - 1;
        for (var i = 0; i < m; ++i)
            _a = wcln(fns[i], fnStr, td_1), fnStr = _a[0], td_1 = _a[1];
        ch[id] = wcln(fns[m], fnStr, td_1);
    }
    var td = mrg({}, ch[id][1]);
    return wk(ch[id][0] + ';onmessage=function(e){for(var k in e.data)self[k]=e.data[k];onmessage=' + init.toString() + '}', id, td, cbfs(td), cb);
};
// base async inflate fn
var bInflt = function () { return [u8, u16, u32, fleb, fdeb, clim, fl, fd, flrm, fdrm, rev, ec, hMap, max, bits, bits16, shft, slc, err, inflt, inflateSync, pbf, gu8]; };
var bDflt = function () { return [u8, u16, u32, fleb, fdeb, clim, revfl, revfd, flm, flt, fdm, fdt, rev, deo, et, hMap, wbits, wbits16, hTree, ln, lc, clen, wfblk, wblk, shft, slc, dflt, dopt, deflateSync, pbf]; };
// gzip extra
var gze = function () { return [gzh, gzhl, wbytes, crc, crct]; };
// gunzip extra
var guze = function () { return [gzs, gzl]; };
// zlib extra
var zle = function () { return [zlh, wbytes, adler]; };
// unzlib extra
var zule = function () { return [zlv]; };
// post buf
var pbf = function (msg) { return postMessage(msg, [msg.buffer]); };
// get u8
var gu8 = function (o) { return o && o.size && new u8(o.size); };
// async helper
var cbify = function (dat, opts, fns, init, id, cb) {
    var w = wrkr(fns, init, id, function (err, dat) {
        w.terminate();
        cb(err, dat);
    });
    w.postMessage([dat, opts], opts.consume ? [dat.buffer] : []);
    return function () { w.terminate(); };
};
// auto stream
var astrm = function (strm) {
    strm.ondata = function (dat, final) { return postMessage([dat, final], [dat.buffer]); };
    return function (ev) { return strm.push(ev.data[0], ev.data[1]); };
};
// async stream attach
var astrmify = function (fns, strm, opts, init, id) {
    var t;
    var w = wrkr(fns, init, id, function (err, dat) {
        if (err)
            w.terminate(), strm.ondata.call(strm, err);
        else {
            if (dat[1])
                w.terminate();
            strm.ondata.call(strm, err, dat[0], dat[1]);
        }
    });
    w.postMessage(opts);
    strm.push = function (d, f) {
        if (!strm.ondata)
            err(5);
        if (t)
            strm.ondata(err(4, 0, 1), null, !!f);
        w.postMessage([d, t = f], [d.buffer]);
    };
    strm.terminate = function () { w.terminate(); };
};
// read 2 bytes
var b2 = function (d, b) { return d[b] | (d[b + 1] << 8); };
// read 4 bytes
var b4 = function (d, b) { return (d[b] | (d[b + 1] << 8) | (d[b + 2] << 16) | (d[b + 3] << 24)) >>> 0; };
var b8 = function (d, b) { return b4(d, b) + (b4(d, b + 4) * 4294967296); };
// write bytes
var wbytes = function (d, b, v) {
    for (; v; ++b)
        d[b] = v, v >>>= 8;
};
// gzip header
var gzh = function (c, o) {
    var fn = o.filename;
    c[0] = 31, c[1] = 139, c[2] = 8, c[8] = o.level < 2 ? 4 : o.level == 9 ? 2 : 0, c[9] = 3; // assume Unix
    if (o.mtime != 0)
        wbytes(c, 4, Math.floor(new Date(o.mtime || Date.now()) / 1000));
    if (fn) {
        c[3] = 8;
        for (var i = 0; i <= fn.length; ++i)
            c[i + 10] = fn.charCodeAt(i);
    }
};
// gzip footer: -8 to -4 = CRC, -4 to -0 is length
// gzip start
var gzs = function (d) {
    if (d[0] != 31 || d[1] != 139 || d[2] != 8)
        err(6, 'invalid gzip data');
    var flg = d[3];
    var st = 10;
    if (flg & 4)
        st += d[10] | (d[11] << 8) + 2;
    for (var zs = (flg >> 3 & 1) + (flg >> 4 & 1); zs > 0; zs -= !d[st++])
        ;
    return st + (flg & 2);
};
// gzip length
var gzl = function (d) {
    var l = d.length;
    return ((d[l - 4] | d[l - 3] << 8 | d[l - 2] << 16) | (d[l - 1] << 24)) >>> 0;
};
// gzip header length
var gzhl = function (o) { return 10 + ((o.filename && (o.filename.length + 1)) || 0); };
// zlib header
var zlh = function (c, o) {
    var lv = o.level, fl = lv == 0 ? 0 : lv < 6 ? 1 : lv == 9 ? 3 : 2;
    c[0] = 120, c[1] = (fl << 6) | (fl ? (32 - 2 * fl) : 1);
};
// zlib valid
var zlv = function (d) {
    if ((d[0] & 15) != 8 || (d[0] >>> 4) > 7 || ((d[0] << 8 | d[1]) % 31))
        err(6, 'invalid zlib data');
    if (d[1] & 32)
        err(6, 'invalid zlib data: preset dictionaries not supported');
};
function AsyncCmpStrm(opts, cb) {
    if (!cb && typeof opts == 'function')
        cb = opts, opts = {};
    this.ondata = cb;
    return opts;
}
// zlib footer: -4 to -0 is Adler32
/**
 * Streaming DEFLATE compression
 */
var Deflate = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    function Deflate(opts, cb) {
        if (!cb && typeof opts == 'function')
            cb = opts, opts = {};
        this.ondata = cb;
        this.o = opts || {};
    }
    Deflate.prototype.p = function (c, f) {
        this.ondata(dopt(c, this.o, 0, 0, !f), f);
    };
    /**
     * Pushes a chunk to be deflated
     * @param chunk The chunk to push
     * @param final Whether this is the last chunk
     */
    Deflate.prototype.push = function (chunk, final) {
        if (!this.ondata)
            err(5);
        if (this.d)
            err(4);
        this.d = final;
        this.p(chunk, final || false);
    };
    return Deflate;
}())));

/**
 * Asynchronous streaming DEFLATE compression
 */
var AsyncDeflate = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    function AsyncDeflate(opts, cb) {
        astrmify([
            bDflt,
            function () { return [astrm, Deflate]; }
        ], this, AsyncCmpStrm.call(this, opts, cb), function (ev) {
            var strm = new Deflate(ev.data);
            onmessage = astrm(strm);
        }, 6);
    }
    return AsyncDeflate;
}())));

function deflate(data, opts, cb) {
    if (!cb)
        cb = opts, opts = {};
    if (typeof cb != 'function')
        err(7);
    return cbify(data, opts, [
        bDflt,
    ], function (ev) { return pbf(deflateSync(ev.data[0], ev.data[1])); }, 0, cb);
}
/**
 * Compresses data with DEFLATE without any wrapper
 * @param data The data to compress
 * @param opts The compression options
 * @returns The deflated version of the data
 */
function deflateSync(data, opts) {
    return dopt(data, opts || {}, 0, 0);
}
/**
 * Streaming DEFLATE decompression
 */
var Inflate = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    /**
     * Creates an inflation stream
     * @param cb The callback to call whenever data is inflated
     */
    function Inflate(cb) {
        this.s = {};
        this.p = new u8(0);
        this.ondata = cb;
    }
    Inflate.prototype.e = function (c) {
        if (!this.ondata)
            err(5);
        if (this.d)
            err(4);
        var l = this.p.length;
        var n = new u8(l + c.length);
        n.set(this.p), n.set(c, l), this.p = n;
    };
    Inflate.prototype.c = function (final) {
        this.d = this.s.i = final || false;
        var bts = this.s.b;
        var dt = inflt(this.p, this.o, this.s);
        this.ondata(slc(dt, bts, this.s.b), this.d);
        this.o = slc(dt, this.s.b - 32768), this.s.b = this.o.length;
        this.p = slc(this.p, (this.s.p / 8) | 0), this.s.p &= 7;
    };
    /**
     * Pushes a chunk to be inflated
     * @param chunk The chunk to push
     * @param final Whether this is the final chunk
     */
    Inflate.prototype.push = function (chunk, final) {
        this.e(chunk), this.c(final);
    };
    return Inflate;
}())));

/**
 * Asynchronous streaming DEFLATE decompression
 */
var AsyncInflate = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    /**
     * Creates an asynchronous inflation stream
     * @param cb The callback to call whenever data is deflated
     */
    function AsyncInflate(cb) {
        this.ondata = cb;
        astrmify([
            bInflt,
            function () { return [astrm, Inflate]; }
        ], this, 0, function () {
            var strm = new Inflate();
            onmessage = astrm(strm);
        }, 7);
    }
    return AsyncInflate;
}())));

function inflate(data, opts, cb) {
    if (!cb)
        cb = opts, opts = {};
    if (typeof cb != 'function')
        err(7);
    return cbify(data, opts, [
        bInflt
    ], function (ev) { return pbf(inflateSync(ev.data[0], gu8(ev.data[1]))); }, 1, cb);
}
/**
 * Expands DEFLATE data with no wrapper
 * @param data The data to decompress
 * @param out Where to write the data. Saves memory if you know the decompressed size and provide an output buffer of that length.
 * @returns The decompressed version of the data
 */
function inflateSync(data, out) {
    return inflt(data, out);
}
// before you yell at me for not just using extends, my reason is that TS inheritance is hard to workerize.
/**
 * Streaming GZIP compression
 */
var Gzip = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    function Gzip(opts, cb) {
        this.c = crc();
        this.l = 0;
        this.v = 1;
        Deflate.call(this, opts, cb);
    }
    /**
     * Pushes a chunk to be GZIPped
     * @param chunk The chunk to push
     * @param final Whether this is the last chunk
     */
    Gzip.prototype.push = function (chunk, final) {
        Deflate.prototype.push.call(this, chunk, final);
    };
    Gzip.prototype.p = function (c, f) {
        this.c.p(c);
        this.l += c.length;
        var raw = dopt(c, this.o, this.v && gzhl(this.o), f && 8, !f);
        if (this.v)
            gzh(raw, this.o), this.v = 0;
        if (f)
            wbytes(raw, raw.length - 8, this.c.d()), wbytes(raw, raw.length - 4, this.l);
        this.ondata(raw, f);
    };
    return Gzip;
}())));

/**
 * Asynchronous streaming GZIP compression
 */
var AsyncGzip = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    function AsyncGzip(opts, cb) {
        astrmify([
            bDflt,
            gze,
            function () { return [astrm, Deflate, Gzip]; }
        ], this, AsyncCmpStrm.call(this, opts, cb), function (ev) {
            var strm = new Gzip(ev.data);
            onmessage = astrm(strm);
        }, 8);
    }
    return AsyncGzip;
}())));

function gzip(data, opts, cb) {
    if (!cb)
        cb = opts, opts = {};
    if (typeof cb != 'function')
        err(7);
    return cbify(data, opts, [
        bDflt,
        gze,
        function () { return [gzipSync]; }
    ], function (ev) { return pbf(gzipSync(ev.data[0], ev.data[1])); }, 2, cb);
}
/**
 * Compresses data with GZIP
 * @param data The data to compress
 * @param opts The compression options
 * @returns The gzipped version of the data
 */
function gzipSync(data, opts) {
    if (!opts)
        opts = {};
    var c = crc(), l = data.length;
    c.p(data);
    var d = dopt(data, opts, gzhl(opts), 8), s = d.length;
    return gzh(d, opts), wbytes(d, s - 8, c.d()), wbytes(d, s - 4, l), d;
}
/**
 * Streaming GZIP decompression
 */
var Gunzip = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    /**
     * Creates a GUNZIP stream
     * @param cb The callback to call whenever data is inflated
     */
    function Gunzip(cb) {
        this.v = 1;
        Inflate.call(this, cb);
    }
    /**
     * Pushes a chunk to be GUNZIPped
     * @param chunk The chunk to push
     * @param final Whether this is the last chunk
     */
    Gunzip.prototype.push = function (chunk, final) {
        Inflate.prototype.e.call(this, chunk);
        if (this.v) {
            var s = this.p.length > 3 ? gzs(this.p) : 4;
            if (s >= this.p.length && !final)
                return;
            this.p = this.p.subarray(s), this.v = 0;
        }
        if (final) {
            if (this.p.length < 8)
                err(6, 'invalid gzip data');
            this.p = this.p.subarray(0, -8);
        }
        // necessary to prevent TS from using the closure value
        // This allows for workerization to function correctly
        Inflate.prototype.c.call(this, final);
    };
    return Gunzip;
}())));

/**
 * Asynchronous streaming GZIP decompression
 */
var AsyncGunzip = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    /**
     * Creates an asynchronous GUNZIP stream
     * @param cb The callback to call whenever data is deflated
     */
    function AsyncGunzip(cb) {
        this.ondata = cb;
        astrmify([
            bInflt,
            guze,
            function () { return [astrm, Inflate, Gunzip]; }
        ], this, 0, function () {
            var strm = new Gunzip();
            onmessage = astrm(strm);
        }, 9);
    }
    return AsyncGunzip;
}())));

function gunzip(data, opts, cb) {
    if (!cb)
        cb = opts, opts = {};
    if (typeof cb != 'function')
        err(7);
    return cbify(data, opts, [
        bInflt,
        guze,
        function () { return [gunzipSync]; }
    ], function (ev) { return pbf(gunzipSync(ev.data[0])); }, 3, cb);
}
/**
 * Expands GZIP data
 * @param data The data to decompress
 * @param out Where to write the data. GZIP already encodes the output size, so providing this doesn't save memory.
 * @returns The decompressed version of the data
 */
function gunzipSync(data, out) {
    return inflt(data.subarray(gzs(data), -8), out || new u8(gzl(data)));
}
/**
 * Streaming Zlib compression
 */
var Zlib = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    function Zlib(opts, cb) {
        this.c = adler();
        this.v = 1;
        Deflate.call(this, opts, cb);
    }
    /**
     * Pushes a chunk to be zlibbed
     * @param chunk The chunk to push
     * @param final Whether this is the last chunk
     */
    Zlib.prototype.push = function (chunk, final) {
        Deflate.prototype.push.call(this, chunk, final);
    };
    Zlib.prototype.p = function (c, f) {
        this.c.p(c);
        var raw = dopt(c, this.o, this.v && 2, f && 4, !f);
        if (this.v)
            zlh(raw, this.o), this.v = 0;
        if (f)
            wbytes(raw, raw.length - 4, this.c.d());
        this.ondata(raw, f);
    };
    return Zlib;
}())));

/**
 * Asynchronous streaming Zlib compression
 */
var AsyncZlib = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    function AsyncZlib(opts, cb) {
        astrmify([
            bDflt,
            zle,
            function () { return [astrm, Deflate, Zlib]; }
        ], this, AsyncCmpStrm.call(this, opts, cb), function (ev) {
            var strm = new Zlib(ev.data);
            onmessage = astrm(strm);
        }, 10);
    }
    return AsyncZlib;
}())));

function zlib(data, opts, cb) {
    if (!cb)
        cb = opts, opts = {};
    if (typeof cb != 'function')
        err(7);
    return cbify(data, opts, [
        bDflt,
        zle,
        function () { return [zlibSync]; }
    ], function (ev) { return pbf(zlibSync(ev.data[0], ev.data[1])); }, 4, cb);
}
/**
 * Compress data with Zlib
 * @param data The data to compress
 * @param opts The compression options
 * @returns The zlib-compressed version of the data
 */
function zlibSync(data, opts) {
    if (!opts)
        opts = {};
    var a = adler();
    a.p(data);
    var d = dopt(data, opts, 2, 4);
    return zlh(d, opts), wbytes(d, d.length - 4, a.d()), d;
}
/**
 * Streaming Zlib decompression
 */
var Unzlib = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    /**
     * Creates a Zlib decompression stream
     * @param cb The callback to call whenever data is inflated
     */
    function Unzlib(cb) {
        this.v = 1;
        Inflate.call(this, cb);
    }
    /**
     * Pushes a chunk to be unzlibbed
     * @param chunk The chunk to push
     * @param final Whether this is the last chunk
     */
    Unzlib.prototype.push = function (chunk, final) {
        Inflate.prototype.e.call(this, chunk);
        if (this.v) {
            if (this.p.length < 2 && !final)
                return;
            this.p = this.p.subarray(2), this.v = 0;
        }
        if (final) {
            if (this.p.length < 4)
                err(6, 'invalid zlib data');
            this.p = this.p.subarray(0, -4);
        }
        // necessary to prevent TS from using the closure value
        // This allows for workerization to function correctly
        Inflate.prototype.c.call(this, final);
    };
    return Unzlib;
}())));

/**
 * Asynchronous streaming Zlib decompression
 */
var AsyncUnzlib = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    /**
     * Creates an asynchronous Zlib decompression stream
     * @param cb The callback to call whenever data is deflated
     */
    function AsyncUnzlib(cb) {
        this.ondata = cb;
        astrmify([
            bInflt,
            zule,
            function () { return [astrm, Inflate, Unzlib]; }
        ], this, 0, function () {
            var strm = new Unzlib();
            onmessage = astrm(strm);
        }, 11);
    }
    return AsyncUnzlib;
}())));

function unzlib(data, opts, cb) {
    if (!cb)
        cb = opts, opts = {};
    if (typeof cb != 'function')
        err(7);
    return cbify(data, opts, [
        bInflt,
        zule,
        function () { return [unzlibSync]; }
    ], function (ev) { return pbf(unzlibSync(ev.data[0], gu8(ev.data[1]))); }, 5, cb);
}
/**
 * Expands Zlib data
 * @param data The data to decompress
 * @param out Where to write the data. Saves memory if you know the decompressed size and provide an output buffer of that length.
 * @returns The decompressed version of the data
 */
function unzlibSync(data, out) {
    return inflt((zlv(data), data.subarray(2, -4)), out);
}
// Default algorithm for compression (used because having a known output size allows faster decompression)

// Default algorithm for compression (used because having a known output size allows faster decompression)

/**
 * Streaming GZIP, Zlib, or raw DEFLATE decompression
 */
var Decompress = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    /**
     * Creates a decompression stream
     * @param cb The callback to call whenever data is decompressed
     */
    function Decompress(cb) {
        this.G = Gunzip;
        this.I = Inflate;
        this.Z = Unzlib;
        this.ondata = cb;
    }
    /**
     * Pushes a chunk to be decompressed
     * @param chunk The chunk to push
     * @param final Whether this is the last chunk
     */
    Decompress.prototype.push = function (chunk, final) {
        if (!this.ondata)
            err(5);
        if (!this.s) {
            if (this.p && this.p.length) {
                var n = new u8(this.p.length + chunk.length);
                n.set(this.p), n.set(chunk, this.p.length);
            }
            else
                this.p = chunk;
            if (this.p.length > 2) {
                var _this_1 = this;
                var cb = function () { _this_1.ondata.apply(_this_1, arguments); };
                this.s = (this.p[0] == 31 && this.p[1] == 139 && this.p[2] == 8)
                    ? new this.G(cb)
                    : ((this.p[0] & 15) != 8 || (this.p[0] >> 4) > 7 || ((this.p[0] << 8 | this.p[1]) % 31))
                        ? new this.I(cb)
                        : new this.Z(cb);
                this.s.push(this.p, final);
                this.p = null;
            }
        }
        else
            this.s.push(chunk, final);
    };
    return Decompress;
}())));

/**
 * Asynchronous streaming GZIP, Zlib, or raw DEFLATE decompression
 */
var AsyncDecompress = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    /**
   * Creates an asynchronous decompression stream
   * @param cb The callback to call whenever data is decompressed
   */
    function AsyncDecompress(cb) {
        this.G = AsyncGunzip;
        this.I = AsyncInflate;
        this.Z = AsyncUnzlib;
        this.ondata = cb;
    }
    /**
     * Pushes a chunk to be decompressed
     * @param chunk The chunk to push
     * @param final Whether this is the last chunk
     */
    AsyncDecompress.prototype.push = function (chunk, final) {
        Decompress.prototype.push.call(this, chunk, final);
    };
    return AsyncDecompress;
}())));

function decompress(data, opts, cb) {
    if (!cb)
        cb = opts, opts = {};
    if (typeof cb != 'function')
        err(7);
    return (data[0] == 31 && data[1] == 139 && data[2] == 8)
        ? gunzip(data, opts, cb)
        : ((data[0] & 15) != 8 || (data[0] >> 4) > 7 || ((data[0] << 8 | data[1]) % 31))
            ? inflate(data, opts, cb)
            : unzlib(data, opts, cb);
}
/**
 * Expands compressed GZIP, Zlib, or raw DEFLATE data, automatically detecting the format
 * @param data The data to decompress
 * @param out Where to write the data. Saves memory if you know the decompressed size and provide an output buffer of that length.
 * @returns The decompressed version of the data
 */
function decompressSync(data, out) {
    return (data[0] == 31 && data[1] == 139 && data[2] == 8)
        ? gunzipSync(data, out)
        : ((data[0] & 15) != 8 || (data[0] >> 4) > 7 || ((data[0] << 8 | data[1]) % 31))
            ? inflateSync(data, out)
            : unzlibSync(data, out);
}
// flatten a directory structure
var fltn = function (d, p, t, o) {
    for (var k in d) {
        var val = d[k], n = p + k;
        if (val instanceof u8)
            t[n] = [val, o];
        else if (Array.isArray(val))
            t[n] = [val[0], mrg(o, val[1])];
        else
            fltn(val, n + '/', t, o);
    }
};
// text encoder
var te = typeof TextEncoder != 'undefined' && /*#__PURE__*/ new TextEncoder();
// text decoder
var td = typeof TextDecoder != 'undefined' && /*#__PURE__*/ new TextDecoder();
// text decoder stream
var tds = 0;
try {
    td.decode(et, { stream: true });
    tds = 1;
}
catch (e) { }
// decode UTF8
var dutf8 = function (d) {
    for (var r = '', i = 0;;) {
        var c = d[i++];
        var eb = (c > 127) + (c > 223) + (c > 239);
        if (i + eb > d.length)
            return [r, slc(d, i - 1)];
        if (!eb)
            r += String.fromCharCode(c);
        else if (eb == 3) {
            c = ((c & 15) << 18 | (d[i++] & 63) << 12 | (d[i++] & 63) << 6 | (d[i++] & 63)) - 65536,
                r += String.fromCharCode(55296 | (c >> 10), 56320 | (c & 1023));
        }
        else if (eb & 1)
            r += String.fromCharCode((c & 31) << 6 | (d[i++] & 63));
        else
            r += String.fromCharCode((c & 15) << 12 | (d[i++] & 63) << 6 | (d[i++] & 63));
    }
};
/**
 * Streaming UTF-8 decoding
 */
var DecodeUTF8 = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    /**
     * Creates a UTF-8 decoding stream
     * @param cb The callback to call whenever data is decoded
     */
    function DecodeUTF8(cb) {
        this.ondata = cb;
        if (tds)
            this.t = new TextDecoder();
        else
            this.p = et;
    }
    /**
     * Pushes a chunk to be decoded from UTF-8 binary
     * @param chunk The chunk to push
     * @param final Whether this is the last chunk
     */
    DecodeUTF8.prototype.push = function (chunk, final) {
        if (!this.ondata)
            err(5);
        final = !!final;
        if (this.t) {
            this.ondata(this.t.decode(chunk, { stream: true }), final);
            if (final) {
                if (this.t.decode().length)
                    err(8);
                this.t = null;
            }
            return;
        }
        if (!this.p)
            err(4);
        var dat = new u8(this.p.length + chunk.length);
        dat.set(this.p);
        dat.set(chunk, this.p.length);
        var _a = dutf8(dat), ch = _a[0], np = _a[1];
        if (final) {
            if (np.length)
                err(8);
            this.p = null;
        }
        else
            this.p = np;
        this.ondata(ch, final);
    };
    return DecodeUTF8;
}())));

/**
 * Streaming UTF-8 encoding
 */
var EncodeUTF8 = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    /**
     * Creates a UTF-8 decoding stream
     * @param cb The callback to call whenever data is encoded
     */
    function EncodeUTF8(cb) {
        this.ondata = cb;
    }
    /**
     * Pushes a chunk to be encoded to UTF-8
     * @param chunk The string data to push
     * @param final Whether this is the last chunk
     */
    EncodeUTF8.prototype.push = function (chunk, final) {
        if (!this.ondata)
            err(5);
        if (this.d)
            err(4);
        this.ondata(strToU8(chunk), this.d = final || false);
    };
    return EncodeUTF8;
}())));

/**
 * Converts a string into a Uint8Array for use with compression/decompression methods
 * @param str The string to encode
 * @param latin1 Whether or not to interpret the data as Latin-1. This should
 *               not need to be true unless decoding a binary string.
 * @returns The string encoded in UTF-8/Latin-1 binary
 */
function strToU8(str, latin1) {
    if (latin1) {
        var ar_1 = new u8(str.length);
        for (var i = 0; i < str.length; ++i)
            ar_1[i] = str.charCodeAt(i);
        return ar_1;
    }
    if (te)
        return te.encode(str);
    var l = str.length;
    var ar = new u8(str.length + (str.length >> 1));
    var ai = 0;
    var w = function (v) { ar[ai++] = v; };
    for (var i = 0; i < l; ++i) {
        if (ai + 5 > ar.length) {
            var n = new u8(ai + 8 + ((l - i) << 1));
            n.set(ar);
            ar = n;
        }
        var c = str.charCodeAt(i);
        if (c < 128 || latin1)
            w(c);
        else if (c < 2048)
            w(192 | (c >> 6)), w(128 | (c & 63));
        else if (c > 55295 && c < 57344)
            c = 65536 + (c & 1023 << 10) | (str.charCodeAt(++i) & 1023),
                w(240 | (c >> 18)), w(128 | ((c >> 12) & 63)), w(128 | ((c >> 6) & 63)), w(128 | (c & 63));
        else
            w(224 | (c >> 12)), w(128 | ((c >> 6) & 63)), w(128 | (c & 63));
    }
    return slc(ar, 0, ai);
}
/**
 * Converts a Uint8Array to a string
 * @param dat The data to decode to string
 * @param latin1 Whether or not to interpret the data as Latin-1. This should
 *               not need to be true unless encoding to binary string.
 * @returns The original UTF-8/Latin-1 string
 */
function strFromU8(dat, latin1) {
    if (latin1) {
        var r = '';
        for (var i = 0; i < dat.length; i += 16384)
            r += String.fromCharCode.apply(null, dat.subarray(i, i + 16384));
        return r;
    }
    else if (td)
        return td.decode(dat);
    else {
        var _a = dutf8(dat), out = _a[0], ext = _a[1];
        if (ext.length)
            err(8);
        return out;
    }
}
;
// deflate bit flag
var dbf = function (l) { return l == 1 ? 3 : l < 6 ? 2 : l == 9 ? 1 : 0; };
// skip local zip header
var slzh = function (d, b) { return b + 30 + b2(d, b + 26) + b2(d, b + 28); };
// read zip header
var zh = function (d, b, z) {
    var fnl = b2(d, b + 28), fn = strFromU8(d.subarray(b + 46, b + 46 + fnl), !(b2(d, b + 8) & 2048)), es = b + 46 + fnl, bs = b4(d, b + 20);
    var _a = z && bs == 4294967295 ? z64e(d, es) : [bs, b4(d, b + 24), b4(d, b + 42)], sc = _a[0], su = _a[1], off = _a[2];
    return [b2(d, b + 10), sc, su, fn, es + b2(d, b + 30) + b2(d, b + 32), off];
};
// read zip64 extra field
var z64e = function (d, b) {
    for (; b2(d, b) != 1; b += 4 + b2(d, b + 2))
        ;
    return [b8(d, b + 12), b8(d, b + 4), b8(d, b + 20)];
};
// extra field length
var exfl = function (ex) {
    var le = 0;
    if (ex) {
        for (var k in ex) {
            var l = ex[k].length;
            if (l > 65535)
                err(9);
            le += l + 4;
        }
    }
    return le;
};
// write zip header
var wzh = function (d, b, f, fn, u, c, ce, co) {
    var fl = fn.length, ex = f.extra, col = co && co.length;
    var exl = exfl(ex);
    wbytes(d, b, ce != null ? 0x2014B50 : 0x4034B50), b += 4;
    if (ce != null)
        d[b++] = 20, d[b++] = f.os;
    d[b] = 20, b += 2; // spec compliance? what's that?
    d[b++] = (f.flag << 1) | (c == null && 8), d[b++] = u && 8;
    d[b++] = f.compression & 255, d[b++] = f.compression >> 8;
    var dt = new Date(f.mtime == null ? Date.now() : f.mtime), y = dt.getFullYear() - 1980;
    if (y < 0 || y > 119)
        err(10);
    wbytes(d, b, (y << 25) | ((dt.getMonth() + 1) << 21) | (dt.getDate() << 16) | (dt.getHours() << 11) | (dt.getMinutes() << 5) | (dt.getSeconds() >>> 1)), b += 4;
    if (c != null) {
        wbytes(d, b, f.crc);
        wbytes(d, b + 4, c);
        wbytes(d, b + 8, f.size);
    }
    wbytes(d, b + 12, fl);
    wbytes(d, b + 14, exl), b += 16;
    if (ce != null) {
        wbytes(d, b, col);
        wbytes(d, b + 6, f.attrs);
        wbytes(d, b + 10, ce), b += 14;
    }
    d.set(fn, b);
    b += fl;
    if (exl) {
        for (var k in ex) {
            var exf = ex[k], l = exf.length;
            wbytes(d, b, +k);
            wbytes(d, b + 2, l);
            d.set(exf, b + 4), b += 4 + l;
        }
    }
    if (col)
        d.set(co, b), b += col;
    return b;
};
// write zip footer (end of central directory)
var wzf = function (o, b, c, d, e) {
    wbytes(o, b, 0x6054B50); // skip disk
    wbytes(o, b + 8, c);
    wbytes(o, b + 10, c);
    wbytes(o, b + 12, d);
    wbytes(o, b + 16, e);
};
/**
 * A pass-through stream to keep data uncompressed in a ZIP archive.
 */
var ZipPassThrough = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    /**
     * Creates a pass-through stream that can be added to ZIP archives
     * @param filename The filename to associate with this data stream
     */
    function ZipPassThrough(filename) {
        this.filename = filename;
        this.c = crc();
        this.size = 0;
        this.compression = 0;
    }
    /**
     * Processes a chunk and pushes to the output stream. You can override this
     * method in a subclass for custom behavior, but by default this passes
     * the data through. You must call this.ondata(err, chunk, final) at some
     * point in this method.
     * @param chunk The chunk to process
     * @param final Whether this is the last chunk
     */
    ZipPassThrough.prototype.process = function (chunk, final) {
        this.ondata(null, chunk, final);
    };
    /**
     * Pushes a chunk to be added. If you are subclassing this with a custom
     * compression algorithm, note that you must push data from the source
     * file only, pre-compression.
     * @param chunk The chunk to push
     * @param final Whether this is the last chunk
     */
    ZipPassThrough.prototype.push = function (chunk, final) {
        if (!this.ondata)
            err(5);
        this.c.p(chunk);
        this.size += chunk.length;
        if (final)
            this.crc = this.c.d();
        this.process(chunk, final || false);
    };
    return ZipPassThrough;
}())));

// I don't extend because TypeScript extension adds 1kB of runtime bloat
/**
 * Streaming DEFLATE compression for ZIP archives. Prefer using AsyncZipDeflate
 * for better performance
 */
var ZipDeflate = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    /**
     * Creates a DEFLATE stream that can be added to ZIP archives
     * @param filename The filename to associate with this data stream
     * @param opts The compression options
     */
    function ZipDeflate(filename, opts) {
        var _this_1 = this;
        if (!opts)
            opts = {};
        ZipPassThrough.call(this, filename);
        this.d = new Deflate(opts, function (dat, final) {
            _this_1.ondata(null, dat, final);
        });
        this.compression = 8;
        this.flag = dbf(opts.level);
    }
    ZipDeflate.prototype.process = function (chunk, final) {
        try {
            this.d.push(chunk, final);
        }
        catch (e) {
            this.ondata(e, null, final);
        }
    };
    /**
     * Pushes a chunk to be deflated
     * @param chunk The chunk to push
     * @param final Whether this is the last chunk
     */
    ZipDeflate.prototype.push = function (chunk, final) {
        ZipPassThrough.prototype.push.call(this, chunk, final);
    };
    return ZipDeflate;
}())));

/**
 * Asynchronous streaming DEFLATE compression for ZIP archives
 */
var AsyncZipDeflate = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    /**
     * Creates a DEFLATE stream that can be added to ZIP archives
     * @param filename The filename to associate with this data stream
     * @param opts The compression options
     */
    function AsyncZipDeflate(filename, opts) {
        var _this_1 = this;
        if (!opts)
            opts = {};
        ZipPassThrough.call(this, filename);
        this.d = new AsyncDeflate(opts, function (err, dat, final) {
            _this_1.ondata(err, dat, final);
        });
        this.compression = 8;
        this.flag = dbf(opts.level);
        this.terminate = this.d.terminate;
    }
    AsyncZipDeflate.prototype.process = function (chunk, final) {
        this.d.push(chunk, final);
    };
    /**
     * Pushes a chunk to be deflated
     * @param chunk The chunk to push
     * @param final Whether this is the last chunk
     */
    AsyncZipDeflate.prototype.push = function (chunk, final) {
        ZipPassThrough.prototype.push.call(this, chunk, final);
    };
    return AsyncZipDeflate;
}())));

// TODO: Better tree shaking
/**
 * A zippable archive to which files can incrementally be added
 */
var Zip = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    /**
     * Creates an empty ZIP archive to which files can be added
     * @param cb The callback to call whenever data for the generated ZIP archive
     *           is available
     */
    function Zip(cb) {
        this.ondata = cb;
        this.u = [];
        this.d = 1;
    }
    /**
     * Adds a file to the ZIP archive
     * @param file The file stream to add
     */
    Zip.prototype.add = function (file) {
        var _this_1 = this;
        if (!this.ondata)
            err(5);
        // finishing or finished
        if (this.d & 2)
            this.ondata(err(4 + (this.d & 1) * 8, 0, 1), null, false);
        else {
            var f = strToU8(file.filename), fl_1 = f.length;
            var com = file.comment, o = com && strToU8(com);
            var u = fl_1 != file.filename.length || (o && (com.length != o.length));
            var hl_1 = fl_1 + exfl(file.extra) + 30;
            if (fl_1 > 65535)
                this.ondata(err(11, 0, 1), null, false);
            var header = new u8(hl_1);
            wzh(header, 0, file, f, u);
            var chks_1 = [header];
            var pAll_1 = function () {
                for (var _i = 0, chks_2 = chks_1; _i < chks_2.length; _i++) {
                    var chk = chks_2[_i];
                    _this_1.ondata(null, chk, false);
                }
                chks_1 = [];
            };
            var tr_1 = this.d;
            this.d = 0;
            var ind_1 = this.u.length;
            var uf_1 = mrg(file, {
                f: f,
                u: u,
                o: o,
                t: function () {
                    if (file.terminate)
                        file.terminate();
                },
                r: function () {
                    pAll_1();
                    if (tr_1) {
                        var nxt = _this_1.u[ind_1 + 1];
                        if (nxt)
                            nxt.r();
                        else
                            _this_1.d = 1;
                    }
                    tr_1 = 1;
                }
            });
            var cl_1 = 0;
            file.ondata = function (err, dat, final) {
                if (err) {
                    _this_1.ondata(err, dat, final);
                    _this_1.terminate();
                }
                else {
                    cl_1 += dat.length;
                    chks_1.push(dat);
                    if (final) {
                        var dd = new u8(16);
                        wbytes(dd, 0, 0x8074B50);
                        wbytes(dd, 4, file.crc);
                        wbytes(dd, 8, cl_1);
                        wbytes(dd, 12, file.size);
                        chks_1.push(dd);
                        uf_1.c = cl_1, uf_1.b = hl_1 + cl_1 + 16, uf_1.crc = file.crc, uf_1.size = file.size;
                        if (tr_1)
                            uf_1.r();
                        tr_1 = 1;
                    }
                    else if (tr_1)
                        pAll_1();
                }
            };
            this.u.push(uf_1);
        }
    };
    /**
     * Ends the process of adding files and prepares to emit the final chunks.
     * This *must* be called after adding all desired files for the resulting
     * ZIP file to work properly.
     */
    Zip.prototype.end = function () {
        var _this_1 = this;
        if (this.d & 2) {
            this.ondata(err(4 + (this.d & 1) * 8, 0, 1), null, true);
            return;
        }
        if (this.d)
            this.e();
        else
            this.u.push({
                r: function () {
                    if (!(_this_1.d & 1))
                        return;
                    _this_1.u.splice(-1, 1);
                    _this_1.e();
                },
                t: function () { }
            });
        this.d = 3;
    };
    Zip.prototype.e = function () {
        var bt = 0, l = 0, tl = 0;
        for (var _i = 0, _a = this.u; _i < _a.length; _i++) {
            var f = _a[_i];
            tl += 46 + f.f.length + exfl(f.extra) + (f.o ? f.o.length : 0);
        }
        var out = new u8(tl + 22);
        for (var _b = 0, _c = this.u; _b < _c.length; _b++) {
            var f = _c[_b];
            wzh(out, bt, f, f.f, f.u, f.c, l, f.o);
            bt += 46 + f.f.length + exfl(f.extra) + (f.o ? f.o.length : 0), l += f.b;
        }
        wzf(out, bt, this.u.length, tl, l);
        this.ondata(null, out, true);
        this.d = 2;
    };
    /**
     * A method to terminate any internal workers used by the stream. Subsequent
     * calls to add() will fail.
     */
    Zip.prototype.terminate = function () {
        for (var _i = 0, _a = this.u; _i < _a.length; _i++) {
            var f = _a[_i];
            f.t();
        }
        this.d = 2;
    };
    return Zip;
}())));

function zip(data, opts, cb) {
    if (!cb)
        cb = opts, opts = {};
    if (typeof cb != 'function')
        err(7);
    var r = {};
    fltn(data, '', r, opts);
    var k = Object.keys(r);
    var lft = k.length, o = 0, tot = 0;
    var slft = lft, files = new Array(lft);
    var term = [];
    var tAll = function () {
        for (var i = 0; i < term.length; ++i)
            term[i]();
    };
    var cbd = function (a, b) {
        mt(function () { cb(a, b); });
    };
    mt(function () { cbd = cb; });
    var cbf = function () {
        var out = new u8(tot + 22), oe = o, cdl = tot - o;
        tot = 0;
        for (var i = 0; i < slft; ++i) {
            var f = files[i];
            try {
                var l = f.c.length;
                wzh(out, tot, f, f.f, f.u, l);
                var badd = 30 + f.f.length + exfl(f.extra);
                var loc = tot + badd;
                out.set(f.c, loc);
                wzh(out, o, f, f.f, f.u, l, tot, f.m), o += 16 + badd + (f.m ? f.m.length : 0), tot = loc + l;
            }
            catch (e) {
                return cbd(e, null);
            }
        }
        wzf(out, o, files.length, cdl, oe);
        cbd(null, out);
    };
    if (!lft)
        cbf();
    var _loop_1 = function (i) {
        var fn = k[i];
        var _a = r[fn], file = _a[0], p = _a[1];
        var c = crc(), size = file.length;
        c.p(file);
        var f = strToU8(fn), s = f.length;
        var com = p.comment, m = com && strToU8(com), ms = m && m.length;
        var exl = exfl(p.extra);
        var compression = p.level == 0 ? 0 : 8;
        var cbl = function (e, d) {
            if (e) {
                tAll();
                cbd(e, null);
            }
            else {
                var l = d.length;
                files[i] = mrg(p, {
                    size: size,
                    crc: c.d(),
                    c: d,
                    f: f,
                    m: m,
                    u: s != fn.length || (m && (com.length != ms)),
                    compression: compression
                });
                o += 30 + s + exl + l;
                tot += 76 + 2 * (s + exl) + (ms || 0) + l;
                if (!--lft)
                    cbf();
            }
        };
        if (s > 65535)
            cbl(err(11, 0, 1), null);
        if (!compression)
            cbl(null, file);
        else if (size < 160000) {
            try {
                cbl(null, deflateSync(file, p));
            }
            catch (e) {
                cbl(e, null);
            }
        }
        else
            term.push(deflate(file, p, cbl));
    };
    // Cannot use lft because it can decrease
    for (var i = 0; i < slft; ++i) {
        _loop_1(i);
    }
    return tAll;
}
/**
 * Synchronously creates a ZIP file. Prefer using `zip` for better performance
 * with more than one file.
 * @param data The directory structure for the ZIP archive
 * @param opts The main options, merged with per-file options
 * @returns The generated ZIP archive
 */
function zipSync(data, opts) {
    if (!opts)
        opts = {};
    var r = {};
    var files = [];
    fltn(data, '', r, opts);
    var o = 0;
    var tot = 0;
    for (var fn in r) {
        var _a = r[fn], file = _a[0], p = _a[1];
        var compression = p.level == 0 ? 0 : 8;
        var f = strToU8(fn), s = f.length;
        var com = p.comment, m = com && strToU8(com), ms = m && m.length;
        var exl = exfl(p.extra);
        if (s > 65535)
            err(11);
        var d = compression ? deflateSync(file, p) : file, l = d.length;
        var c = crc();
        c.p(file);
        files.push(mrg(p, {
            size: file.length,
            crc: c.d(),
            c: d,
            f: f,
            m: m,
            u: s != fn.length || (m && (com.length != ms)),
            o: o,
            compression: compression
        }));
        o += 30 + s + exl + l;
        tot += 76 + 2 * (s + exl) + (ms || 0) + l;
    }
    var out = new u8(tot + 22), oe = o, cdl = tot - o;
    for (var i = 0; i < files.length; ++i) {
        var f = files[i];
        wzh(out, f.o, f, f.f, f.u, f.c.length);
        var badd = 30 + f.f.length + exfl(f.extra);
        out.set(f.c, f.o + badd);
        wzh(out, o, f, f.f, f.u, f.c.length, f.o, f.m), o += 16 + badd + (f.m ? f.m.length : 0);
    }
    wzf(out, o, files.length, cdl, oe);
    return out;
}
/**
 * Streaming pass-through decompression for ZIP archives
 */
var UnzipPassThrough = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    function UnzipPassThrough() {
    }
    UnzipPassThrough.prototype.push = function (data, final) {
        this.ondata(null, data, final);
    };
    UnzipPassThrough.compression = 0;
    return UnzipPassThrough;
}())));

/**
 * Streaming DEFLATE decompression for ZIP archives. Prefer AsyncZipInflate for
 * better performance.
 */
var UnzipInflate = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    /**
     * Creates a DEFLATE decompression that can be used in ZIP archives
     */
    function UnzipInflate() {
        var _this_1 = this;
        this.i = new Inflate(function (dat, final) {
            _this_1.ondata(null, dat, final);
        });
    }
    UnzipInflate.prototype.push = function (data, final) {
        try {
            this.i.push(data, final);
        }
        catch (e) {
            this.ondata(e, null, final);
        }
    };
    UnzipInflate.compression = 8;
    return UnzipInflate;
}())));

/**
 * Asynchronous streaming DEFLATE decompression for ZIP archives
 */
var AsyncUnzipInflate = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    /**
     * Creates a DEFLATE decompression that can be used in ZIP archives
     */
    function AsyncUnzipInflate(_, sz) {
        var _this_1 = this;
        if (sz < 320000) {
            this.i = new Inflate(function (dat, final) {
                _this_1.ondata(null, dat, final);
            });
        }
        else {
            this.i = new AsyncInflate(function (err, dat, final) {
                _this_1.ondata(err, dat, final);
            });
            this.terminate = this.i.terminate;
        }
    }
    AsyncUnzipInflate.prototype.push = function (data, final) {
        if (this.i.terminate)
            data = slc(data, 0);
        this.i.push(data, final);
    };
    AsyncUnzipInflate.compression = 8;
    return AsyncUnzipInflate;
}())));

/**
 * A ZIP archive decompression stream that emits files as they are discovered
 */
var Unzip = /*#__PURE__*/ ((/* unused pure expression or super */ null && (function () {
    /**
     * Creates a ZIP decompression stream
     * @param cb The callback to call whenever a file in the ZIP archive is found
     */
    function Unzip(cb) {
        this.onfile = cb;
        this.k = [];
        this.o = {
            0: UnzipPassThrough
        };
        this.p = et;
    }
    /**
     * Pushes a chunk to be unzipped
     * @param chunk The chunk to push
     * @param final Whether this is the last chunk
     */
    Unzip.prototype.push = function (chunk, final) {
        var _this_1 = this;
        if (!this.onfile)
            err(5);
        if (!this.p)
            err(4);
        if (this.c > 0) {
            var len = Math.min(this.c, chunk.length);
            var toAdd = chunk.subarray(0, len);
            this.c -= len;
            if (this.d)
                this.d.push(toAdd, !this.c);
            else
                this.k[0].push(toAdd);
            chunk = chunk.subarray(len);
            if (chunk.length)
                return this.push(chunk, final);
        }
        else {
            var f = 0, i = 0, is = void 0, buf = void 0;
            if (!this.p.length)
                buf = chunk;
            else if (!chunk.length)
                buf = this.p;
            else {
                buf = new u8(this.p.length + chunk.length);
                buf.set(this.p), buf.set(chunk, this.p.length);
            }
            var l = buf.length, oc = this.c, add = oc && this.d;
            var _loop_2 = function () {
                var _a;
                var sig = b4(buf, i);
                if (sig == 0x4034B50) {
                    f = 1, is = i;
                    this_1.d = null;
                    this_1.c = 0;
                    var bf = b2(buf, i + 6), cmp_1 = b2(buf, i + 8), u = bf & 2048, dd = bf & 8, fnl = b2(buf, i + 26), es = b2(buf, i + 28);
                    if (l > i + 30 + fnl + es) {
                        var chks_3 = [];
                        this_1.k.unshift(chks_3);
                        f = 2;
                        var sc_1 = b4(buf, i + 18), su_1 = b4(buf, i + 22);
                        var fn_1 = strFromU8(buf.subarray(i + 30, i += 30 + fnl), !u);
                        if (sc_1 == 4294967295) {
                            _a = dd ? [-2] : z64e(buf, i), sc_1 = _a[0], su_1 = _a[1];
                        }
                        else if (dd)
                            sc_1 = -1;
                        i += es;
                        this_1.c = sc_1;
                        var d_1;
                        var file_1 = {
                            name: fn_1,
                            compression: cmp_1,
                            start: function () {
                                if (!file_1.ondata)
                                    err(5);
                                if (!sc_1)
                                    file_1.ondata(null, et, true);
                                else {
                                    var ctr = _this_1.o[cmp_1];
                                    if (!ctr)
                                        file_1.ondata(err(14, 'unknown compression type ' + cmp_1, 1), null, false);
                                    d_1 = sc_1 < 0 ? new ctr(fn_1) : new ctr(fn_1, sc_1, su_1);
                                    d_1.ondata = function (err, dat, final) { file_1.ondata(err, dat, final); };
                                    for (var _i = 0, chks_4 = chks_3; _i < chks_4.length; _i++) {
                                        var dat = chks_4[_i];
                                        d_1.push(dat, false);
                                    }
                                    if (_this_1.k[0] == chks_3 && _this_1.c)
                                        _this_1.d = d_1;
                                    else
                                        d_1.push(et, true);
                                }
                            },
                            terminate: function () {
                                if (d_1 && d_1.terminate)
                                    d_1.terminate();
                            }
                        };
                        if (sc_1 >= 0)
                            file_1.size = sc_1, file_1.originalSize = su_1;
                        this_1.onfile(file_1);
                    }
                    return "break";
                }
                else if (oc) {
                    if (sig == 0x8074B50) {
                        is = i += 12 + (oc == -2 && 8), f = 3, this_1.c = 0;
                        return "break";
                    }
                    else if (sig == 0x2014B50) {
                        is = i -= 4, f = 3, this_1.c = 0;
                        return "break";
                    }
                }
            };
            var this_1 = this;
            for (; i < l - 4; ++i) {
                var state_1 = _loop_2();
                if (state_1 === "break")
                    break;
            }
            this.p = et;
            if (oc < 0) {
                var dat = f ? buf.subarray(0, is - 12 - (oc == -2 && 8) - (b4(buf, is - 16) == 0x8074B50 && 4)) : buf.subarray(0, i);
                if (add)
                    add.push(dat, !!f);
                else
                    this.k[+(f == 2)].push(dat);
            }
            if (f & 2)
                return this.push(buf.subarray(i), final);
            this.p = buf.subarray(i);
        }
        if (final) {
            if (this.c)
                err(13);
            this.p = null;
        }
    };
    /**
     * Registers a decoder with the stream, allowing for files compressed with
     * the compression type provided to be expanded correctly
     * @param decoder The decoder constructor
     */
    Unzip.prototype.register = function (decoder) {
        this.o[decoder.compression] = decoder;
    };
    return Unzip;
}())));

var mt = typeof queueMicrotask == 'function' ? queueMicrotask : typeof setTimeout == 'function' ? setTimeout : function (fn) { fn(); };
function unzip(data, opts, cb) {
    if (!cb)
        cb = opts, opts = {};
    if (typeof cb != 'function')
        err(7);
    var term = [];
    var tAll = function () {
        for (var i = 0; i < term.length; ++i)
            term[i]();
    };
    var files = {};
    var cbd = function (a, b) {
        mt(function () { cb(a, b); });
    };
    mt(function () { cbd = cb; });
    var e = data.length - 22;
    for (; b4(data, e) != 0x6054B50; --e) {
        if (!e || data.length - e > 65558) {
            cbd(err(13, 0, 1), null);
            return tAll;
        }
    }
    ;
    var lft = b2(data, e + 8);
    if (lft) {
        var c = lft;
        var o = b4(data, e + 16);
        var z = o == 4294967295;
        if (z) {
            e = b4(data, e - 12);
            if (b4(data, e) != 0x6064B50) {
                cbd(err(13, 0, 1), null);
                return tAll;
            }
            c = lft = b4(data, e + 32);
            o = b4(data, e + 48);
        }
        var fltr = opts && opts.filter;
        var _loop_3 = function (i) {
            var _a = zh(data, o, z), c_1 = _a[0], sc = _a[1], su = _a[2], fn = _a[3], no = _a[4], off = _a[5], b = slzh(data, off);
            o = no;
            var cbl = function (e, d) {
                if (e) {
                    tAll();
                    cbd(e, null);
                }
                else {
                    if (d)
                        files[fn] = d;
                    if (!--lft)
                        cbd(null, files);
                }
            };
            if (!fltr || fltr({
                name: fn,
                size: sc,
                originalSize: su,
                compression: c_1
            })) {
                if (!c_1)
                    cbl(null, slc(data, b, b + sc));
                else if (c_1 == 8) {
                    var infl = data.subarray(b, b + sc);
                    if (sc < 320000) {
                        try {
                            cbl(null, inflateSync(infl, new u8(su)));
                        }
                        catch (e) {
                            cbl(e, null);
                        }
                    }
                    else
                        term.push(inflate(infl, { size: su }, cbl));
                }
                else
                    cbl(err(14, 'unknown compression type ' + c_1, 1), null);
            }
            else
                cbl(null, null);
        };
        for (var i = 0; i < c; ++i) {
            _loop_3(i);
        }
    }
    else
        cbd(null, {});
    return tAll;
}
/**
 * Synchronously decompresses a ZIP archive. Prefer using `unzip` for better
 * performance with more than one file.
 * @param data The raw compressed ZIP file
 * @param opts The ZIP extraction options
 * @returns The decompressed files
 */
function unzipSync(data, opts) {
    var files = {};
    var e = data.length - 22;
    for (; b4(data, e) != 0x6054B50; --e) {
        if (!e || data.length - e > 65558)
            err(13);
    }
    ;
    var c = b2(data, e + 8);
    if (!c)
        return {};
    var o = b4(data, e + 16);
    var z = o == 4294967295;
    if (z) {
        e = b4(data, e - 12);
        if (b4(data, e) != 0x6064B50)
            err(13);
        c = b4(data, e + 32);
        o = b4(data, e + 48);
    }
    var fltr = opts && opts.filter;
    for (var i = 0; i < c; ++i) {
        var _a = zh(data, o, z), c_2 = _a[0], sc = _a[1], su = _a[2], fn = _a[3], no = _a[4], off = _a[5], b = slzh(data, off);
        o = no;
        if (!fltr || fltr({
            name: fn,
            size: sc,
            originalSize: su,
            compression: c_2
        })) {
            if (!c_2)
                files[fn] = slc(data, b, b + sc);
            else if (c_2 == 8)
                files[fn] = inflateSync(data.subarray(b, b + sc), new u8(su));
            else
                err(14, 'unknown compression type ' + c_2);
        }
    }
    return files;
}

;// CONCATENATED MODULE: ./src/core/pings/database.ts





const pings_database_LOG_TAG = "core.Pings.Database";
function isDeletionRequest(ping) {
    return ping.path.split("/")[3] === DELETION_REQUEST_PING_NAME;
}
function getPingSize(ping) {
    return strToU8(JSON.stringify(ping)).length;
}
function isValidPingInternalRepresentation(v) {
    if (isObject(v) && (Object.keys(v).length === 2 || Object.keys(v).length === 3)) {
        const hasValidPath = "path" in v && isString(v.path);
        const hasValidPayload = "payload" in v && isJSONValue(v.payload) && isObject(v.payload);
        const hasValidHeaders = (!("headers" in v)) || (isJSONValue(v.headers) && isObject(v.headers));
        if (!hasValidPath || !hasValidPayload || !hasValidHeaders) {
            return false;
        }
        return true;
    }
    return false;
}
class PingsDatabase {
    constructor(store) {
        this.store = new store("pings");
    }
    attachObserver(observer) {
        this.observer = observer;
    }
    recordPing(path, identifier, payload, headers) {
        return __awaiter(this, void 0, void 0, function* () {
            const ping = {
                collectionDate: (new Date()).toISOString(),
                path,
                payload
            };
            if (headers) {
                ping.headers = headers;
            }
            yield this.store.update([identifier], () => ping);
            this.observer && this.observer.update(identifier, ping);
        });
    }
    deletePing(identifier) {
        return __awaiter(this, void 0, void 0, function* () {
            yield this.store.delete([identifier]);
        });
    }
    getAllPings() {
        return __awaiter(this, void 0, void 0, function* () {
            const allStoredPings = yield this.store.get();
            const finalPings = {};
            if (isObject(allStoredPings)) {
                for (const identifier in allStoredPings) {
                    const ping = allStoredPings[identifier];
                    if (isValidPingInternalRepresentation(ping)) {
                        finalPings[identifier] = ping;
                    }
                    else {
                        log(pings_database_LOG_TAG, "Unexpected data found in pings database. Deleting.", LoggingLevel.Warn);
                        yield this.store.delete([identifier]);
                    }
                }
            }
            return Object.entries(finalPings)
                .sort(([_idA, { collectionDate: dateA }], [_idB, { collectionDate: dateB }]) => {
                const timeA = (new Date(dateA)).getTime();
                const timeB = (new Date(dateB)).getTime();
                return timeA - timeB;
            });
        });
    }
    getAllPingsWithoutSurplus(maxCount = 250, maxSize = 10 * 1024 * 1024) {
        return __awaiter(this, void 0, void 0, function* () {
            const allPings = yield this.getAllPings();
            const pings = allPings
                .filter(([_, ping]) => !isDeletionRequest(ping))
                .reverse();
            const deletionRequestPings = allPings.filter(([_, ping]) => isDeletionRequest(ping));
            const total = pings.length;
            if (total > maxCount) {
                log(pings_database_LOG_TAG, [
                    `More than ${maxCount} pending pings in the pings database,`,
                    `will delete ${total - maxCount} old pings.`
                ], LoggingLevel.Warn);
            }
            let deleting = false;
            let pendingPingsCount = 0;
            let pendingPingsDatabaseSize = 0;
            const remainingPings = [];
            for (const [identifier, ping] of pings) {
                pendingPingsCount++;
                pendingPingsDatabaseSize += getPingSize(ping);
                if (!deleting && pendingPingsDatabaseSize > maxSize) {
                    log(pings_database_LOG_TAG, [
                        `Pending pings database has reached the size quota of ${maxSize} bytes,`,
                        "outstanding pings will be deleted."
                    ], LoggingLevel.Warn);
                    deleting = true;
                }
                if (pendingPingsCount > maxCount) {
                    deleting = true;
                }
                if (deleting) {
                    yield this.deletePing(identifier);
                }
                else {
                    remainingPings.unshift([identifier, ping]);
                }
            }
            return [...deletionRequestPings, ...remainingPings];
        });
    }
    scanPendingPings() {
        return __awaiter(this, void 0, void 0, function* () {
            if (!this.observer) {
                return;
            }
            const pings = yield this.getAllPingsWithoutSurplus();
            for (const [identifier, ping] of pings) {
                this.observer.update(identifier, ping);
            }
        });
    }
    clearAll() {
        return __awaiter(this, void 0, void 0, function* () {
            yield this.store.delete([]);
        });
    }
}
/* harmony default export */ const pings_database = (PingsDatabase);

;// CONCATENATED MODULE: ./src/core/upload/rate_limiter.ts

var RateLimiterState;
(function (RateLimiterState) {
    RateLimiterState[RateLimiterState["Incrementing"] = 0] = "Incrementing";
    RateLimiterState[RateLimiterState["Stopped"] = 1] = "Stopped";
    RateLimiterState[RateLimiterState["Throttled"] = 2] = "Throttled";
})(RateLimiterState || (RateLimiterState = {}));
class RateLimiter {
    constructor(interval, maxCount, count = 0, started) {
        this.interval = interval;
        this.maxCount = maxCount;
        this.count = count;
        this.started = started;
        this.stopped = false;
    }
    get elapsed() {
        if (isUndefined(this.started)) {
            return NaN;
        }
        const now = getMonotonicNow();
        const result = now - this.started;
        if (result < 0) {
            return NaN;
        }
        return result;
    }
    reset() {
        this.started = getMonotonicNow();
        this.count = 0;
        this.stopped = false;
    }
    shouldReset() {
        if (isUndefined(this.started)) {
            return true;
        }
        if (isNaN(this.elapsed) || this.elapsed > this.interval) {
            return true;
        }
        return false;
    }
    getState() {
        if (this.shouldReset()) {
            this.reset();
        }
        const remainingTime = this.interval - this.elapsed;
        if (this.stopped) {
            return {
                state: 1,
                remainingTime,
            };
        }
        if (this.count >= this.maxCount) {
            return {
                state: 2,
                remainingTime,
            };
        }
        this.count++;
        return {
            state: 0
        };
    }
    stop() {
        this.stopped = true;
    }
}
/* harmony default export */ const rate_limiter = (RateLimiter);

;// CONCATENATED MODULE: ./src/core/upload/index.ts









const upload_LOG_TAG = "core.Upload";
const RATE_LIMITER_INTERVAL_MS = 60 * 1000;
const MAX_PINGS_PER_INTERVAL = 15;
function createAndInitializeDispatcher() {
    const dispatcher = new core_dispatcher(100, `${upload_LOG_TAG}.Dispatcher`);
    dispatcher.flushInit(undefined, `${upload_LOG_TAG}.createAndInitializeDispatcher`);
    return dispatcher;
}
class Policy {
    constructor(maxRecoverableFailures = 3, maxPingBodySize = 1024 * 1024) {
        this.maxRecoverableFailures = maxRecoverableFailures;
        this.maxPingBodySize = maxPingBodySize;
    }
}
class PingBodyOverflowError extends Error {
    constructor(message) {
        super(message);
        this.name = "PingBodyOverflow";
    }
}
class PingUploader {
    constructor(config, platform, pingsDatabase, policy = new Policy(), rateLimiter = new rate_limiter(RATE_LIMITER_INTERVAL_MS, MAX_PINGS_PER_INTERVAL)) {
        this.pingsDatabase = pingsDatabase;
        this.policy = policy;
        this.rateLimiter = rateLimiter;
        this.processing = [];
        this.uploader = config.httpClient ? config.httpClient : platform.uploader;
        this.platformInfo = platform.info;
        this.serverEndpoint = config.serverEndpoint;
        this.dispatcher = createAndInitializeDispatcher();
    }
    enqueuePing(ping) {
        for (const queuedPing of this.processing) {
            if (queuedPing.identifier === ping.identifier) {
                return;
            }
        }
        this.processing.push(ping);
        const { state: rateLimiterState, remainingTime } = this.rateLimiter.getState();
        if (rateLimiterState === 0) {
            this.dispatcher.resume();
        }
        else {
            this.dispatcher.stop(false);
            if (rateLimiterState === 2) {
                log(upload_LOG_TAG, [
                    "Attempted to upload a ping, but Glean is currently throttled.",
                    `Pending pings will be processed in ${(remainingTime || 0) / 1000}s.`
                ], LoggingLevel.Debug);
            }
            else if (rateLimiterState === 1) {
                log(upload_LOG_TAG, [
                    "Attempted to upload a ping, but Glean has reached maximum recoverable upload failures",
                    "for the current uploading window.",
                    `Will retry in ${(remainingTime || 0) / 1000}s.`
                ], LoggingLevel.Debug);
            }
        }
        const launchFn = isDeletionRequest(ping) ? this.dispatcher.launchPersistent : this.dispatcher.launch;
        launchFn.bind(this.dispatcher)(() => __awaiter(this, void 0, void 0, function* () {
            const status = yield this.attemptPingUpload(ping);
            const shouldRetry = yield this.processPingUploadResponse(ping.identifier, status);
            if (shouldRetry) {
                ping.retries++;
                this.enqueuePing(ping);
            }
            if (ping.retries >= this.policy.maxRecoverableFailures) {
                log(upload_LOG_TAG, `Reached maximum recoverable failures for ping "${JSON.stringify(ping.name)}". You are done.`, LoggingLevel.Info);
                this.rateLimiter.stop();
                this.dispatcher.stop();
                ping.retries = 0;
            }
        }), `${upload_LOG_TAG}.${ping.identifier}.enqueuePing`);
    }
    preparePingForUpload(ping) {
        return __awaiter(this, void 0, void 0, function* () {
            let headers = ping.headers || {};
            headers = Object.assign(Object.assign({}, ping.headers), { "Content-Type": "application/json; charset=utf-8", "Date": (new Date()).toISOString(), "X-Client-Type": "Glean.js", "X-Client-Version": GLEAN_VERSION, "X-Telemetry-Agent": `Glean/${GLEAN_VERSION} (JS on ${yield this.platformInfo.os()})` });
            const stringifiedBody = JSON.stringify(ping.payload);
            const encodedBody = strToU8(stringifiedBody);
            let finalBody;
            let bodySizeInBytes;
            try {
                finalBody = gzipSync(encodedBody);
                bodySizeInBytes = finalBody.length;
                headers["Content-Encoding"] = "gzip";
            }
            catch (_a) {
                finalBody = stringifiedBody;
                bodySizeInBytes = encodedBody.length;
            }
            if (bodySizeInBytes > this.policy.maxPingBodySize) {
                throw new PingBodyOverflowError(`Body for ping ${ping.identifier} exceeds ${this.policy.maxPingBodySize}bytes. Discarding.`);
            }
            headers["Content-Length"] = bodySizeInBytes.toString();
            return {
                headers,
                payload: finalBody
            };
        });
    }
    attemptPingUpload(ping) {
        return __awaiter(this, void 0, void 0, function* () {
            if (!Context.initialized) {
                log(upload_LOG_TAG, "Attempted to upload a ping, but Glean is not initialized yet. Ignoring.", LoggingLevel.Warn);
                return new UploadResult(0);
            }
            try {
                const finalPing = yield this.preparePingForUpload(ping);
                return yield this.uploader.post(`${this.serverEndpoint}${ping.path}`, finalPing.payload, finalPing.headers);
            }
            catch (e) {
                log(upload_LOG_TAG, [
                    "Error trying to build ping request:",
                    e.message
                ], LoggingLevel.Warn);
                return new UploadResult(0);
            }
        });
    }
    concludePingProcessing(identifier) {
        this.processing = this.processing.filter(ping => ping.identifier !== identifier);
    }
    processPingUploadResponse(identifier, response) {
        return __awaiter(this, void 0, void 0, function* () {
            this.concludePingProcessing(identifier);
            const { status, result } = response;
            if (status && status >= 200 && status < 300) {
                log(upload_LOG_TAG, `Ping ${identifier} succesfully sent ${status}.`, LoggingLevel.Info);
                yield this.pingsDatabase.deletePing(identifier);
                return false;
            }
            if (result === 1 || (status && status >= 400 && status < 500)) {
                log(upload_LOG_TAG, `Unrecoverable upload failure while attempting to send ping ${identifier}. Error was: ${status !== null && status !== void 0 ? status : "no status"}.`, LoggingLevel.Warn);
                yield this.pingsDatabase.deletePing(identifier);
                return false;
            }
            log(upload_LOG_TAG, [
                `Recoverable upload failure while attempting to send ping ${identifier}, will retry.`,
                `Error was ${status !== null && status !== void 0 ? status : "no status"}.`
            ], LoggingLevel.Warn);
            return true;
        });
    }
    update(identifier, ping) {
        this.dispatcher.resume();
        this.enqueuePing(Object.assign({ identifier, retries: 0 }, ping));
    }
    shutdown() {
        return this.dispatcher.shutdown();
    }
    clearPendingPingsQueue() {
        return __awaiter(this, void 0, void 0, function* () {
            this.dispatcher.clear();
            yield this.dispatcher.shutdown();
            this.processing = [];
            this.dispatcher = createAndInitializeDispatcher();
        });
    }
    testBlockOnPingsQueue() {
        return __awaiter(this, void 0, void 0, function* () {
            return this.dispatcher.testBlockOnQueue();
        });
    }
}
/* harmony default export */ const upload = (PingUploader);

;// CONCATENATED MODULE: ./src/core/internal_metrics.ts










const internal_metrics_LOG_TAG = "core.InternalMetrics";
class CoreMetrics {
    constructor() {
        this.clientId = new uuid({
            name: "client_id",
            category: "",
            sendInPings: ["glean_client_info"],
            lifetime: "user",
            disabled: false,
        });
        this.firstRunDate = new datetime({
            name: "first_run_date",
            category: "",
            sendInPings: ["glean_client_info"],
            lifetime: "user",
            disabled: false,
        }, time_unit.Day);
        this.os = new string({
            name: "os",
            category: "",
            sendInPings: ["glean_client_info"],
            lifetime: "application",
            disabled: false,
        });
        this.osVersion = new string({
            name: "os_version",
            category: "",
            sendInPings: ["glean_client_info"],
            lifetime: "application",
            disabled: false,
        });
        this.architecture = new string({
            name: "architecture",
            category: "",
            sendInPings: ["glean_client_info"],
            lifetime: "application",
            disabled: false,
        });
        this.locale = new string({
            name: "locale",
            category: "",
            sendInPings: ["glean_client_info"],
            lifetime: "application",
            disabled: false,
        });
        this.appBuild = new string({
            name: "app_build",
            category: "",
            sendInPings: ["glean_client_info"],
            lifetime: "application",
            disabled: false,
        });
        this.appDisplayVersion = new string({
            name: "app_display_version",
            category: "",
            sendInPings: ["glean_client_info"],
            lifetime: "application",
            disabled: false,
        });
    }
    initialize(config, platform) {
        return __awaiter(this, void 0, void 0, function* () {
            yield this.initializeClientId();
            yield this.initializeFirstRunDate();
            yield string._private_setUndispatched(this.os, yield platform.info.os());
            yield string._private_setUndispatched(this.osVersion, yield platform.info.osVersion());
            yield string._private_setUndispatched(this.architecture, yield platform.info.arch());
            yield string._private_setUndispatched(this.locale, yield platform.info.locale());
            yield string._private_setUndispatched(this.appBuild, config.appBuild || "Unknown");
            yield string._private_setUndispatched(this.appDisplayVersion, config.appDisplayVersion || "Unknown");
        });
    }
    initializeClientId() {
        return __awaiter(this, void 0, void 0, function* () {
            let needNewClientId = false;
            const clientIdData = yield Context.metricsDatabase.getMetric(CLIENT_INFO_STORAGE, this.clientId);
            if (clientIdData) {
                try {
                    const currentClientId = createMetric("uuid", clientIdData);
                    if (currentClientId.payload() === KNOWN_CLIENT_ID) {
                        needNewClientId = true;
                    }
                }
                catch (_a) {
                    log(internal_metrics_LOG_TAG, "Unexpected value found for Glean clientId. Ignoring.", LoggingLevel.Warn);
                    needNewClientId = true;
                }
            }
            else {
                needNewClientId = true;
            }
            if (needNewClientId) {
                yield uuid._private_setUndispatched(this.clientId, generateUUIDv4());
            }
        });
    }
    initializeFirstRunDate() {
        return __awaiter(this, void 0, void 0, function* () {
            const firstRunDate = yield Context.metricsDatabase.getMetric(CLIENT_INFO_STORAGE, this.firstRunDate);
            if (!firstRunDate) {
                yield datetime._private_setUndispatched(this.firstRunDate);
            }
        });
    }
}

;// CONCATENATED MODULE: ./src/core/metrics/events_database/recorded_event.ts

class RecordedEvent {
    constructor(category, name, timestamp, extra) {
        this.category = category;
        this.name = name;
        this.timestamp = timestamp;
        this.extra = extra;
    }
    static toJSONObject(e) {
        return {
            "category": e.category,
            "name": e.name,
            "timestamp": e.timestamp,
            "extra": e.extra,
        };
    }
    static fromJSONObject(e) {
        return new RecordedEvent(e["category"], e["name"], e["timestamp"], e["extra"]);
    }
    static withTransformedExtras(e, transformFn) {
        const extras = e.extra || {};
        const transformedExtras = transformFn(extras);
        return new RecordedEvent(e.category, e.name, e.timestamp, (transformedExtras && Object.keys(transformedExtras).length > 0) ? transformedExtras : undefined);
    }
    addExtra(key, value) {
        if (!this.extra) {
            this.extra = {};
        }
        this.extra[key] = value;
    }
    withoutReservedExtras() {
        return RecordedEvent.withTransformedExtras(this, (extras) => {
            return Object.keys(extras)
                .filter(key => !GLEAN_RESERVED_EXTRA_KEYS.includes(key))
                .reduce((obj, key) => {
                obj[key] = extras[key];
                return obj;
            }, {});
        });
    }
    payload() {
        return RecordedEvent.withTransformedExtras(this.withoutReservedExtras(), (extras) => {
            return Object.keys(extras)
                .reduce((extra, key) => {
                extra[key] = extras[key].toString();
                return extra;
            }, {});
        });
    }
}

;// CONCATENATED MODULE: ./src/core/metrics/types/event.ts






const event_LOG_TAG = "core.metrics.EventMetricType";
const MAX_LENGTH_EXTRA_KEY_VALUE = 100;
class EventMetricType extends MetricType {
    constructor(meta, allowedExtraKeys) {
        super("event", meta);
        this.allowedExtraKeys = allowedExtraKeys;
    }
    static _private_recordUndispatched(instance, extra, timestamp = getMonotonicNow()) {
        return __awaiter(this, void 0, void 0, function* () {
            if (!instance.shouldRecord(Context.uploadEnabled)) {
                return;
            }
            let truncatedExtra = undefined;
            if (extra && instance.allowedExtraKeys) {
                truncatedExtra = {};
                for (const [name, value] of Object.entries(extra)) {
                    if (instance.allowedExtraKeys.includes(name)) {
                        if (isString(value)) {
                            truncatedExtra[name] = yield truncateStringAtBoundaryWithError(instance, value, MAX_LENGTH_EXTRA_KEY_VALUE);
                        }
                        else {
                            truncatedExtra[name] = value;
                        }
                    }
                    else {
                        yield Context.errorManager.record(instance, ErrorType.InvalidValue, `Invalid key index: ${name}`);
                        continue;
                    }
                }
            }
            return Context.eventsDatabase.record(instance, new RecordedEvent(instance.category, instance.name, timestamp, truncatedExtra));
        });
    }
    record(extra) {
        const timestamp = getMonotonicNow();
        Context.dispatcher.launch(() => __awaiter(this, void 0, void 0, function* () {
            yield EventMetricType._private_recordUndispatched(this, extra, timestamp);
        }), `${event_LOG_TAG}.${this.baseIdentifier()}.record`);
    }
    testGetValue(ping = this.sendInPings[0]) {
        return __awaiter(this, void 0, void 0, function* () {
            let events;
            yield Context.dispatcher.testLaunch(() => __awaiter(this, void 0, void 0, function* () {
                events = yield Context.eventsDatabase.getEvents(ping, this);
            }));
            return events;
        });
    }
}
/* harmony default export */ const types_event = (EventMetricType);

;// CONCATENATED MODULE: ./src/core/metrics/events_database/index.ts











const events_database_LOG_TAG = "core.Metric.EventsDatabase";
function createDateObject(str) {
    if (!isString(str)) {
        str = "";
    }
    const date = new Date(str);
    if (isNaN(date.getTime())) {
        throw new Error(`Error attempting to generate Date object from string: ${str}`);
    }
    return date;
}
function getExecutionCounterMetric(sendInPings) {
    return new counter(Object.assign(Object.assign({}, generateReservedMetricIdentifiers("execution_counter")), { sendInPings: sendInPings, lifetime: "ping", disabled: false }));
}
function getGleanRestartedEventMetric(sendInPings) {
    return new types_event({
        category: "glean",
        name: "restarted",
        sendInPings: sendInPings,
        lifetime: "ping",
        disabled: false
    }, [GLEAN_REFERENCE_TIME_EXTRA_KEY]);
}
function recordGleanRestartedEvent(sendInPings, time = Context.startTime) {
    return __awaiter(this, void 0, void 0, function* () {
        const metric = getGleanRestartedEventMetric(sendInPings);
        yield types_event._private_recordUndispatched(metric, {
            [GLEAN_REFERENCE_TIME_EXTRA_KEY]: time.toISOString()
        }, 0);
    });
}
class EventsDatabase {
    constructor(storage) {
        this.initialized = false;
        this.eventsStore = new storage("events");
    }
    getAvailableStoreNames() {
        return __awaiter(this, void 0, void 0, function* () {
            const data = yield this.eventsStore.get([]);
            if (isUndefined(data)) {
                return [];
            }
            return Object.keys(data);
        });
    }
    initialize() {
        return __awaiter(this, void 0, void 0, function* () {
            if (this.initialized) {
                return;
            }
            const storeNames = yield this.getAvailableStoreNames();
            yield counter._private_addUndispatched(getExecutionCounterMetric(storeNames), 1);
            yield recordGleanRestartedEvent(storeNames);
            this.initialized = true;
        });
    }
    record(metric, value) {
        return __awaiter(this, void 0, void 0, function* () {
            if (metric.disabled) {
                return;
            }
            for (const ping of metric.sendInPings) {
                const executionCounter = getExecutionCounterMetric([ping]);
                let currentExecutionCount = yield Context.metricsDatabase.getMetric(ping, executionCounter);
                if (!currentExecutionCount) {
                    yield counter._private_addUndispatched(executionCounter, 1);
                    currentExecutionCount = 1;
                    yield recordGleanRestartedEvent([ping], new Date());
                }
                value.addExtra(GLEAN_EXECUTION_COUNTER_EXTRA_KEY, currentExecutionCount);
                const transformFn = (v) => {
                    var _a;
                    const existing = (_a = v) !== null && _a !== void 0 ? _a : [];
                    existing.push(RecordedEvent.toJSONObject(value));
                    return existing;
                };
                yield this.eventsStore.update([ping], transformFn);
            }
        });
    }
    getEvents(ping, metric) {
        return __awaiter(this, void 0, void 0, function* () {
            const events = yield this.getAndValidatePingData(ping);
            if (events.length === 0) {
                return;
            }
            return events
                .filter((e) => (e.category === metric.category) && (e.name === metric.name))
                .map(e => e.withoutReservedExtras());
        });
    }
    getAndValidatePingData(ping) {
        return __awaiter(this, void 0, void 0, function* () {
            const data = yield this.eventsStore.get([ping]);
            if (isUndefined(data)) {
                return [];
            }
            if (!Array.isArray(data)) {
                log(events_database_LOG_TAG, `Unexpected value found for ping ${ping}: ${JSON.stringify(data)}. Clearing.`, LoggingLevel.Error);
                yield this.eventsStore.delete([ping]);
                return [];
            }
            return data.map((e) => RecordedEvent.fromJSONObject(e));
        });
    }
    getPingEvents(ping, clearPingLifetimeData) {
        return __awaiter(this, void 0, void 0, function* () {
            const pingData = yield this.getAndValidatePingData(ping);
            if (clearPingLifetimeData && Object.keys(pingData).length > 0) {
                yield this.eventsStore.delete([ping]);
            }
            if (pingData.length === 0) {
                return;
            }
            const payload = yield this.prepareEventsPayload(ping, pingData);
            if (payload.length > 0) {
                return payload;
            }
        });
    }
    prepareEventsPayload(pingName, pingData) {
        var _a, _b, _c, _d;
        return __awaiter(this, void 0, void 0, function* () {
            const sortedEvents = pingData.sort((a, b) => {
                var _a, _b;
                const executionCounterA = Number((_a = a.extra) === null || _a === void 0 ? void 0 : _a[GLEAN_EXECUTION_COUNTER_EXTRA_KEY]);
                const executionCounterB = Number((_b = b.extra) === null || _b === void 0 ? void 0 : _b[GLEAN_EXECUTION_COUNTER_EXTRA_KEY]);
                if (executionCounterA !== executionCounterB) {
                    return executionCounterA - executionCounterB;
                }
                return a.timestamp - b.timestamp;
            });
            let lastRestartDate;
            try {
                lastRestartDate = createDateObject((_a = sortedEvents[0].extra) === null || _a === void 0 ? void 0 : _a[GLEAN_REFERENCE_TIME_EXTRA_KEY]);
                sortedEvents.shift();
            }
            catch (_e) {
                lastRestartDate = Context.startTime;
            }
            const firstEventOffset = ((_b = sortedEvents[0]) === null || _b === void 0 ? void 0 : _b.timestamp) || 0;
            let restartedOffset = 0;
            for (const [index, event] of sortedEvents.entries()) {
                try {
                    const nextRestartDate = createDateObject((_c = event.extra) === null || _c === void 0 ? void 0 : _c[GLEAN_REFERENCE_TIME_EXTRA_KEY]);
                    const dateOffset = nextRestartDate.getTime() - lastRestartDate.getTime();
                    lastRestartDate = nextRestartDate;
                    const newRestartedOffset = restartedOffset + dateOffset;
                    const previousEventTimestamp = sortedEvents[index - 1].timestamp;
                    if (newRestartedOffset <= previousEventTimestamp) {
                        restartedOffset = previousEventTimestamp + 1;
                        yield Context.errorManager.record(getGleanRestartedEventMetric([pingName]), ErrorType.InvalidValue, `Invalid time offset between application sessions found for ping "${pingName}". Ignoring.`);
                    }
                    else {
                        restartedOffset = newRestartedOffset;
                    }
                }
                catch (_f) {
                }
                const executionCount = Number(((_d = event.extra) === null || _d === void 0 ? void 0 : _d[GLEAN_EXECUTION_COUNTER_EXTRA_KEY]) || 1);
                let adjustedTimestamp;
                if (executionCount === 1) {
                    adjustedTimestamp = event.timestamp - firstEventOffset;
                }
                else {
                    adjustedTimestamp = event.timestamp + restartedOffset;
                }
                sortedEvents[index] = new RecordedEvent(event.category, event.name, adjustedTimestamp, event.extra);
            }
            return sortedEvents.map((e) => RecordedEvent.toJSONObject(e.payload()));
        });
    }
    clearAll() {
        return __awaiter(this, void 0, void 0, function* () {
            yield this.eventsStore.delete([]);
        });
    }
}
/* harmony default export */ const events_database = (EventsDatabase);

;// CONCATENATED MODULE: ./src/core/events/index.ts

const events_LOG_TAG = "core.Events";
class CoreEvent {
    constructor(name) {
        this.name = name;
    }
    get registeredPluginIdentifier() {
        var _a;
        return (_a = this.plugin) === null || _a === void 0 ? void 0 : _a.name;
    }
    registerPlugin(plugin) {
        if (this.plugin) {
            log(events_LOG_TAG, [
                `Attempted to register plugin '${plugin.name}', which listens to the event '${plugin.event}'.`,
                `That event is already watched by plugin '${this.plugin.name}'`,
                `Plugin '${plugin.name}' will be ignored.`
            ], LoggingLevel.Error);
            return;
        }
        this.plugin = plugin;
    }
    deregisterPlugin() {
        this.plugin = undefined;
    }
    trigger(...args) {
        if (this.plugin) {
            return this.plugin.action(...args);
        }
    }
}
const CoreEvents = {
    afterPingCollection: new CoreEvent("afterPingCollection")
};
/* harmony default export */ const events = (CoreEvents);

;// CONCATENATED MODULE: ./src/core/pings/maker.ts








const maker_LOG_TAG = "core.Pings.Maker";
function getStartTimeMetricAndData(ping) {
    return __awaiter(this, void 0, void 0, function* () {
        const startTimeMetric = new datetime({
            category: "",
            name: `${ping.name}#start`,
            sendInPings: [PING_INFO_STORAGE],
            lifetime: "user",
            disabled: false
        }, time_unit.Minute);
        const startTimeData = yield Context.metricsDatabase.getMetric(PING_INFO_STORAGE, startTimeMetric);
        let startTime;
        if (startTimeData) {
            startTime = new DatetimeMetric(startTimeData);
        }
        else {
            startTime = DatetimeMetric.fromDate(Context.startTime, time_unit.Minute);
        }
        return {
            startTimeMetric,
            startTime,
        };
    });
}
function getSequenceNumber(ping) {
    return __awaiter(this, void 0, void 0, function* () {
        const seq = new counter({
            category: "",
            name: `${ping.name}#sequence`,
            sendInPings: [PING_INFO_STORAGE],
            lifetime: "user",
            disabled: false
        });
        const currentSeqData = yield Context.metricsDatabase.getMetric(PING_INFO_STORAGE, seq);
        yield counter._private_addUndispatched(seq, 1);
        if (currentSeqData) {
            try {
                const metric = new CounterMetric(currentSeqData);
                return metric.payload();
            }
            catch (e) {
                log(maker_LOG_TAG, `Unexpected value found for sequence number in ping ${ping.name}. Ignoring.`, LoggingLevel.Warn);
            }
        }
        return 0;
    });
}
function getStartEndTimes(ping) {
    return __awaiter(this, void 0, void 0, function* () {
        const { startTimeMetric, startTime } = yield getStartTimeMetricAndData(ping);
        const endTimeData = new Date();
        yield datetime._private_setUndispatched(startTimeMetric, endTimeData);
        const endTime = DatetimeMetric.fromDate(endTimeData, time_unit.Minute);
        return {
            startTime: startTime.payload(),
            endTime: endTime.payload()
        };
    });
}
function buildPingInfoSection(ping, reason) {
    return __awaiter(this, void 0, void 0, function* () {
        const seq = yield getSequenceNumber(ping);
        const { startTime, endTime } = yield getStartEndTimes(ping);
        const pingInfo = {
            seq,
            start_time: startTime,
            end_time: endTime
        };
        if (reason) {
            pingInfo.reason = reason;
        }
        return pingInfo;
    });
}
function buildClientInfoSection(ping) {
    return __awaiter(this, void 0, void 0, function* () {
        let clientInfo = yield Context.metricsDatabase.getPingMetrics(CLIENT_INFO_STORAGE, true);
        if (!clientInfo) {
            log(maker_LOG_TAG, "Empty client info data. Will submit anyways.", LoggingLevel.Warn);
            clientInfo = {};
        }
        let finalClientInfo = {
            "telemetry_sdk_build": GLEAN_VERSION
        };
        for (const metricType in clientInfo) {
            finalClientInfo = Object.assign(Object.assign({}, finalClientInfo), clientInfo[metricType]);
        }
        if (!ping.includeClientId) {
            delete finalClientInfo["client_id"];
        }
        return finalClientInfo;
    });
}
function getPingHeaders() {
    var _a, _b;
    const headers = {};
    if ((_a = Context.debugOptions) === null || _a === void 0 ? void 0 : _a.debugViewTag) {
        headers["X-Debug-ID"] = Context.debugOptions.debugViewTag;
    }
    if ((_b = Context.debugOptions) === null || _b === void 0 ? void 0 : _b.sourceTags) {
        headers["X-Source-Tags"] = Context.debugOptions.sourceTags.toString();
    }
    if (Object.keys(headers).length > 0) {
        return headers;
    }
}
function collectPing(ping, reason) {
    return __awaiter(this, void 0, void 0, function* () {
        const eventsData = yield Context.eventsDatabase.getPingEvents(ping.name, true);
        const metricsData = yield Context.metricsDatabase.getPingMetrics(ping.name, true);
        if (!metricsData && !eventsData) {
            if (!ping.sendIfEmpty) {
                log(maker_LOG_TAG, `Storage for ${ping.name} empty. Bailing out.`, LoggingLevel.Info);
                return;
            }
            log(maker_LOG_TAG, `Storage for ${ping.name} empty. Ping will still be sent.`, LoggingLevel.Info);
        }
        const metrics = metricsData ? { metrics: metricsData } : {};
        const events = eventsData ? { events: eventsData } : {};
        const pingInfo = yield buildPingInfoSection(ping, reason);
        const clientInfo = yield buildClientInfoSection(ping);
        return Object.assign(Object.assign(Object.assign({}, metrics), events), { ping_info: pingInfo, client_info: clientInfo });
    });
}
function makePath(identifier, ping) {
    return `/submit/${Context.applicationId}/${ping.name}/${GLEAN_SCHEMA_VERSION}/${identifier}`;
}
function collectAndStorePing(identifier, ping, reason) {
    return __awaiter(this, void 0, void 0, function* () {
        const collectedPayload = yield collectPing(ping, reason);
        if (!collectedPayload) {
            return;
        }
        let modifiedPayload;
        try {
            modifiedPayload = yield events.afterPingCollection.trigger(collectedPayload);
        }
        catch (e) {
            log(maker_LOG_TAG, [
                `Error while attempting to modify ping payload for the "${ping.name}" ping using`,
                `the ${JSON.stringify(events.afterPingCollection.registeredPluginIdentifier)} plugin.`,
                "Ping will not be submitted. See more logs below.\n\n",
                JSON.stringify(e)
            ], LoggingLevel.Error);
            return;
        }
        if (Context.debugOptions.logPings) {
            log(maker_LOG_TAG, JSON.stringify(collectedPayload, null, 2), LoggingLevel.Info);
        }
        const finalPayload = modifiedPayload ? modifiedPayload : collectedPayload;
        const headers = getPingHeaders();
        return Context.pingsDatabase.recordPing(makePath(identifier, ping), identifier, finalPayload, headers);
    });
}
/* harmony default export */ const maker = (collectAndStorePing);

;// CONCATENATED MODULE: ./src/core/pings/ping_type.ts






const ping_type_LOG_TAG = "core.Pings.PingType";
class PingType {
    constructor(meta) {
        var _a;
        this.name = meta.name;
        this.includeClientId = meta.includeClientId;
        this.sendIfEmpty = meta.sendIfEmpty;
        this.reasonCodes = (_a = meta.reasonCodes) !== null && _a !== void 0 ? _a : [];
    }
    isDeletionRequest() {
        return this.name === DELETION_REQUEST_PING_NAME;
    }
    submit(reason) {
        if (this.testCallback) {
            this.testCallback(reason)
                .then(() => {
                PingType._private_internalSubmit(this, reason, this.resolveTestPromiseFunction);
            })
                .catch(e => {
                log(ping_type_LOG_TAG, [`There was an error validating "${this.name}" (${reason !== null && reason !== void 0 ? reason : "no reason"}):`, e], LoggingLevel.Error);
                PingType._private_internalSubmit(this, reason, this.rejectTestPromiseFunction);
            });
        }
        else {
            PingType._private_internalSubmit(this, reason);
        }
    }
    static _private_submitUndispatched(instance, reason, testResolver) {
        return __awaiter(this, void 0, void 0, function* () {
            if (!Context.initialized) {
                log(ping_type_LOG_TAG, "Glean must be initialized before submitting pings.", LoggingLevel.Info);
                return;
            }
            if (!Context.uploadEnabled && !instance.isDeletionRequest()) {
                log(ping_type_LOG_TAG, "Glean disabled: not submitting pings. Glean may still submit the deletion-request ping.", LoggingLevel.Info);
                return;
            }
            let correctedReason = reason;
            if (reason && !instance.reasonCodes.includes(reason)) {
                log(ping_type_LOG_TAG, `Invalid reason code ${reason} from ${this.name}. Ignoring.`, LoggingLevel.Warn);
                correctedReason = undefined;
            }
            const identifier = generateUUIDv4();
            yield maker(identifier, instance, correctedReason);
            if (testResolver) {
                testResolver();
                instance.resolveTestPromiseFunction = undefined;
                instance.rejectTestPromiseFunction = undefined;
                instance.testCallback = undefined;
            }
        });
    }
    static _private_internalSubmit(instance, reason, testResolver) {
        Context.dispatcher.launch(() => __awaiter(this, void 0, void 0, function* () {
            yield PingType._private_submitUndispatched(instance, reason, testResolver);
        }), `${ping_type_LOG_TAG}.${instance.name}._private_internalSubmit`);
    }
    testBeforeNextSubmit(callbackFn) {
        return __awaiter(this, void 0, void 0, function* () {
            if (this.testCallback) {
                log(ping_type_LOG_TAG, `There is an existing test call for ping "${this.name}". Ignoring.`, LoggingLevel.Error);
                return;
            }
            return new Promise((resolve, reject) => {
                this.resolveTestPromiseFunction = resolve;
                this.rejectTestPromiseFunction = reject;
                this.testCallback = callbackFn;
            });
        });
    }
}
/* harmony default export */ const ping_type = (PingType);

;// CONCATENATED MODULE: ./src/core/internal_pings.ts


class CorePings {
    constructor() {
        this.deletionRequest = new ping_type({
            name: DELETION_REQUEST_PING_NAME,
            includeClientId: true,
            sendIfEmpty: true,
        });
    }
}
/* harmony default export */ const internal_pings = (CorePings);

;// CONCATENATED MODULE: ./src/core/events/utils.ts


const events_utils_LOG_TAG = "core.Events.Utils";
function registerPluginToEvent(plugin) {
    const eventName = plugin.event;
    if (eventName in events) {
        const event = events[eventName];
        event.registerPlugin(plugin);
        return;
    }
    log(events_utils_LOG_TAG, [
        `Attempted to register plugin '${plugin.name}', which listens to the event '${plugin.event}'.`,
        "That is not a valid Glean event. Ignoring"
    ], LoggingLevel.Error);
}
function testResetEvents() {
    for (const event in events) {
        events[event].deregisterPlugin();
    }
}

;// CONCATENATED MODULE: ./src/core/error/index.ts




function createLogTag(metric) {
    const capitalizedType = metric.type.charAt(0).toUpperCase() + metric.type.slice(1);
    return `core.Metrics.${capitalizedType}`;
}
function getErrorMetricForMetric(metric, error) {
    const identifier = metric.baseIdentifier();
    const name = stripLabel(identifier);
    return new counter({
        name: combineIdentifierAndLabel(error, name),
        category: "glean.error",
        lifetime: "ping",
        sendInPings: metric.sendInPings,
        disabled: false,
    });
}
class ErrorManager {
    record(metric, error, message, numErrors = 1) {
        return __awaiter(this, void 0, void 0, function* () {
            const errorMetric = getErrorMetricForMetric(metric, error);
            log(createLogTag(metric), `${metric.baseIdentifier()}: ${message}`);
            if (numErrors > 0) {
                yield counter._private_addUndispatched(errorMetric, numErrors);
            }
            else {
            }
        });
    }
    testGetNumRecordedErrors(metric, error, ping) {
        return __awaiter(this, void 0, void 0, function* () {
            const errorMetric = getErrorMetricForMetric(metric, error);
            const numErrors = yield errorMetric.testGetValue(ping);
            return numErrors || 0;
        });
    }
}

;// CONCATENATED MODULE: ./src/platform/test/storage.ts


const test_storage_LOG_TAG = "plaftom.test.Storage";
let globalStore = {};
class MockStore {
    constructor(rootKey) {
        this.rootKey = rootKey;
    }
    get(index = []) {
        try {
            const value = getValueFromNestedObject(globalStore, [this.rootKey, ...index]);
            return Promise.resolve(value);
        }
        catch (e) {
            return Promise.reject(e);
        }
    }
    update(index, transformFn) {
        try {
            globalStore = updateNestedObject(globalStore, [this.rootKey, ...index], transformFn);
            return Promise.resolve();
        }
        catch (e) {
            return Promise.reject(e);
        }
    }
    delete(index) {
        try {
            globalStore = deleteKeyFromNestedObject(globalStore, [this.rootKey, ...index]);
        }
        catch (e) {
            log(test_storage_LOG_TAG, [e.message, "Ignoring."], LoggingLevel.Warn);
        }
        return Promise.resolve();
    }
}
/* harmony default export */ const test_storage = (MockStore);

;// CONCATENATED MODULE: ./src/platform/test/index.ts



class MockUploader extends uploader {
    post(_url, _body, _headers) {
        const result = new UploadResult(2, 200);
        return Promise.resolve(result);
    }
}
const MockPlatformInfo = {
    os() {
        return Promise.resolve("Unknown");
    },
    osVersion() {
        return Promise.resolve("Unknown");
    },
    arch() {
        return Promise.resolve("Unknown");
    },
    locale() {
        return Promise.resolve("Unknown");
    },
};
const TestPlatform = {
    Storage: test_storage,
    uploader: new MockUploader(),
    info: MockPlatformInfo,
    name: "test"
};
/* harmony default export */ const test = (TestPlatform);

;// CONCATENATED MODULE: ./src/core/glean.ts


















const glean_LOG_TAG = "core.Glean";
class Glean {
    constructor() {
        if (!isUndefined(Glean._instance)) {
            throw new Error(`Tried to instantiate Glean through \`new\`.
      Use Glean.instance instead to access the Glean singleton.`);
        }
        this._coreMetrics = new CoreMetrics();
        this._corePings = new internal_pings();
    }
    static get instance() {
        if (!Glean._instance) {
            Glean._instance = new Glean();
        }
        return Glean._instance;
    }
    static get pingUploader() {
        return Glean.instance._pingUploader;
    }
    static get coreMetrics() {
        return Glean.instance._coreMetrics;
    }
    static get corePings() {
        return Glean.instance._corePings;
    }
    static onUploadEnabled() {
        return __awaiter(this, void 0, void 0, function* () {
            Context.uploadEnabled = true;
            yield Glean.coreMetrics.initialize(Glean.instance._config, Glean.platform);
        });
    }
    static onUploadDisabled() {
        return __awaiter(this, void 0, void 0, function* () {
            Context.uploadEnabled = false;
            yield ping_type._private_submitUndispatched(Glean.corePings.deletionRequest);
            yield Glean.clearMetrics();
        });
    }
    static clearMetrics() {
        return __awaiter(this, void 0, void 0, function* () {
            yield Glean.pingUploader.clearPendingPingsQueue();
            let firstRunDate;
            try {
                firstRunDate = new DatetimeMetric(yield Context.metricsDatabase.getMetric(CLIENT_INFO_STORAGE, Glean.coreMetrics.firstRunDate)).date;
            }
            catch (_a) {
                firstRunDate = new Date();
            }
            yield Context.eventsDatabase.clearAll();
            yield Context.metricsDatabase.clearAll();
            yield Context.pingsDatabase.clearAll();
            Context.uploadEnabled = true;
            yield uuid._private_setUndispatched(Glean.coreMetrics.clientId, KNOWN_CLIENT_ID);
            yield datetime._private_setUndispatched(Glean.coreMetrics.firstRunDate, firstRunDate);
            Context.uploadEnabled = false;
        });
    }
    static initialize(applicationId, uploadEnabled, config) {
        if (Context.initialized) {
            log(glean_LOG_TAG, "Attempted to initialize Glean, but it has already been initialized. Ignoring.", LoggingLevel.Warn);
            return;
        }
        if (applicationId.length === 0) {
            log(glean_LOG_TAG, "Unable to initialize Glean, applicationId cannot be an empty string.", LoggingLevel.Error);
            return;
        }
        if (!Glean.instance._platform) {
            log(glean_LOG_TAG, "Unable to initialize Glean, environment has not been set.", LoggingLevel.Error);
            return;
        }
        Context.applicationId = sanitizeApplicationId(applicationId);
        const correctConfig = new Configuration(config);
        Context.debugOptions = correctConfig.debug;
        Glean.instance._config = correctConfig;
        Context.metricsDatabase = new database(Glean.platform.Storage);
        Context.eventsDatabase = new events_database(Glean.platform.Storage);
        Context.pingsDatabase = new pings_database(Glean.platform.Storage);
        Context.errorManager = new ErrorManager();
        Glean.instance._pingUploader = new upload(correctConfig, Glean.platform, Context.pingsDatabase);
        Context.pingsDatabase.attachObserver(Glean.pingUploader);
        if (config === null || config === void 0 ? void 0 : config.plugins) {
            for (const plugin of config.plugins) {
                registerPluginToEvent(plugin);
            }
        }
        Context.dispatcher.flushInit(() => __awaiter(this, void 0, void 0, function* () {
            Context.initialized = true;
            yield Context.metricsDatabase.clear("application");
            if (uploadEnabled) {
                yield Glean.onUploadEnabled();
            }
            else {
                const clientId = yield Context.metricsDatabase.getMetric(CLIENT_INFO_STORAGE, Glean.coreMetrics.clientId);
                if (clientId) {
                    if (clientId !== KNOWN_CLIENT_ID) {
                        yield Glean.onUploadDisabled();
                    }
                }
                else {
                    yield Glean.clearMetrics();
                }
            }
            yield Context.eventsDatabase.initialize();
            yield Context.pingsDatabase.scanPendingPings();
        }), `${glean_LOG_TAG}.initialize`);
    }
    static get serverEndpoint() {
        var _a;
        return (_a = Glean.instance._config) === null || _a === void 0 ? void 0 : _a.serverEndpoint;
    }
    static get logPings() {
        var _a, _b;
        return ((_b = (_a = Glean.instance._config) === null || _a === void 0 ? void 0 : _a.debug) === null || _b === void 0 ? void 0 : _b.logPings) || false;
    }
    static get debugViewTag() {
        var _a;
        return (_a = Glean.instance._config) === null || _a === void 0 ? void 0 : _a.debug.debugViewTag;
    }
    static get sourceTags() {
        var _a, _b;
        return (_b = (_a = Glean.instance._config) === null || _a === void 0 ? void 0 : _a.debug.sourceTags) === null || _b === void 0 ? void 0 : _b.toString();
    }
    static get platform() {
        if (!Glean.instance._platform) {
            throw new Error("IMPOSSIBLE: Attempted to access environment specific APIs before Glean was initialized.");
        }
        return Glean.instance._platform;
    }
    static setUploadEnabled(flag) {
        Context.dispatcher.launch(() => __awaiter(this, void 0, void 0, function* () {
            if (!Context.initialized) {
                log(glean_LOG_TAG, [
                    "Changing upload enabled before Glean is initialized is not supported.\n",
                    "Pass the correct state into `Glean.initialize\n`.",
                    "See documentation at https://mozilla.github.io/glean/book/user/general-api.html#initializing-the-glean-sdk`"
                ], LoggingLevel.Error);
                return;
            }
            if (Context.uploadEnabled !== flag) {
                if (flag) {
                    yield Glean.onUploadEnabled();
                }
                else {
                    yield Glean.onUploadDisabled();
                }
            }
        }), `${glean_LOG_TAG}.setUploadEnabled`);
    }
    static setLogPings(flag) {
        Context.dispatcher.launch(() => {
            Glean.instance._config.debug.logPings = flag;
            return Promise.resolve();
        }, `${glean_LOG_TAG}.setLogPings`);
    }
    static setDebugViewTag(value) {
        if (!Configuration.validateDebugViewTag(value)) {
            log(glean_LOG_TAG, `Invalid \`debugViewTag\` ${value}. Ignoring.`, LoggingLevel.Error);
            return;
        }
        Context.dispatcher.launch(() => {
            Glean.instance._config.debug.debugViewTag = value;
            return Promise.resolve();
        }, `${glean_LOG_TAG}.setDebugViewTag`);
    }
    static setSourceTags(value) {
        if (!Configuration.validateSourceTags(value)) {
            log(glean_LOG_TAG, `Invalid \`sourceTags\` ${value.toString()}. Ignoring.`, LoggingLevel.Error);
            return;
        }
        Context.dispatcher.launch(() => {
            Glean.instance._config.debug.sourceTags = value;
            return Promise.resolve();
        }, `${glean_LOG_TAG}.setSourceTags`);
    }
    static shutdown() {
        return __awaiter(this, void 0, void 0, function* () {
            yield Context.dispatcher.shutdown();
            yield Glean.pingUploader.shutdown();
        });
    }
    static setPlatform(platform) {
        if (Context.initialized) {
            return;
        }
        if (Glean.instance._platform && Glean.instance._platform.name !== platform.name) {
            log(glean_LOG_TAG, `Changing Glean platform from "${Glean.platform.name}" to "${platform.name}".`);
        }
        Glean.instance._platform = platform;
    }
    static testInitialize(applicationId, uploadEnabled = true, config) {
        return __awaiter(this, void 0, void 0, function* () {
            Glean.setPlatform(test);
            Glean.initialize(applicationId, uploadEnabled, config);
            yield Context.dispatcher.testBlockOnQueue();
        });
    }
    static testUninitialize(clearStores = true) {
        return __awaiter(this, void 0, void 0, function* () {
            if (Context.initialized) {
                yield Glean.shutdown();
                if (clearStores) {
                    yield Context.eventsDatabase.clearAll();
                    yield Context.metricsDatabase.clearAll();
                    yield Context.pingsDatabase.clearAll();
                }
                Context.testUninitialize();
                testResetEvents();
            }
        });
    }
    static testResetGlean(applicationId, uploadEnabled = true, config, clearStores = true) {
        return __awaiter(this, void 0, void 0, function* () {
            yield Glean.testUninitialize(clearStores);
            yield Glean.testInitialize(applicationId, uploadEnabled, config);
        });
    }
}
/* harmony default export */ const glean = (Glean);

;// CONCATENATED MODULE: ./src/index/base.ts


/* harmony default export */ const base = ((platform) => {
    return {
        initialize(applicationId, uploadEnabled, config) {
            glean.setPlatform(platform);
            glean.initialize(applicationId, uploadEnabled, config);
        },
        setUploadEnabled(flag) {
            glean.setUploadEnabled(flag);
        },
        setLogPings(flag) {
            glean.setLogPings(flag);
        },
        setDebugViewTag(value) {
            glean.setDebugViewTag(value);
        },
        shutdown() {
            return glean.shutdown();
        },
        setSourceTags(value) {
            glean.setSourceTags(value);
        },
        testResetGlean(applicationId, uploadEnabled = true, config) {
            return __awaiter(this, void 0, void 0, function* () {
                return glean.testResetGlean(applicationId, uploadEnabled, config);
            });
        }
    };
});

;// CONCATENATED MODULE: ./src/index/qt.ts
















/* harmony default export */ const index_qt = (Object.assign(Object.assign({}, base(qt)), { ErrorType: ErrorType, _private: {
        PingType: ping_type,
        BooleanMetricType: types_boolean,
        CounterMetricType: counter,
        DatetimeMetricType: datetime,
        EventMetricType: types_event,
        LabeledMetricType: labeled,
        QuantityMetricType: quantity,
        StringMetricType: string,
        StringListMetricType: string_list,
        TimespanMetricType: timespan,
        TextMetricType: types_text,
        UUIDMetricType: uuid,
        URLMetricType: url
    } }));

})();

Glean = __webpack_exports__;
/******/ })()
;