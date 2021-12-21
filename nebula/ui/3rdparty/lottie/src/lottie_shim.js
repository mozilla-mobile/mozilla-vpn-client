/*
 * Copyright 2018 Kai Uwe Broulik <kde@broulik.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

// Shims to simulate a browser environment
var navigator;
var window;
var document;

// so we can parent Timer {} objects to it...
var canvasItem;

var highestInterval = 0;
var timers = {}; // we need arbitrary index, hence no Array

function setInterval(callback, interval) {
    ++highestInterval;

    var timer = Qt.createQmlObject("import QtQuick 2.0; Timer {}", canvasItem, "setInterval");
    timer.interval = interval;
    timer.repeat = true;
    timer.triggered.connect(callback);
    timers[highestInterval] = timer;
    timer.start();

    return highestInterval;
}

function clearInterval(id) {
    var timer = timers[id];
    if (!timer) {
        return; // doesn't throw
    }

    timer.stop();
    timer.destroy();
    delete timers[id];
}

function setTimeout(callback, interval) {
    if (interval === 0) {
        // We don't want Qt to compress these hence not just passing "callback"
        Qt.callLater(function() {
            callback();
        });
        return;
    }

    // TODO can we re-use the timers here or is there a Qt.callLater(delay)?
    var timer = Qt.createQmlObject("import QtQuick 2.0; Timer {}", canvasItem, "setTimeout");
    timer.interval = interval;
    timer.triggered.connect(function () {
        callback();
        timer.destroy();
    })
    timer.start();
}

// Public API
function initialize(canvas) {
    if (canvasItem && canvas !== canvasItem) {
        throw new TypeError("Cannot re-initialize with a different canvas");
    }

    // Already initialized?
    if (canvasItem && typeof window === "object" && window.lottie) {
        return window.lottie;
    }

    canvasItem = canvas;

    navigator = {
        "userAgent": "%1/%2 (%3; %4)".arg(Qt.application.name).arg(Qt.application.version).arg(Qt.platform.pluginName).arg(Qt.platform.os),
        "language": Qt.locale().name
    };
    window = {
        // QTBUG-68278: Canvas.requestAnimationTime callback is given seconds but browsers return milliseconds
        // and as such animations don't advance properly in Lottie.
        // https://codereview.qt-project.org/#/c/244301/
        // Lottie can fall back to use setTimeout instead but this causes repaint issues and CPU stress.
        requestAnimationFrame: function (cb) {
            return canvas.requestAnimationFrame(function (timestamp) {
                // Creating our own timestamp here to workaround the aforementioned Qt bug
                cb(Date.now());
            });
        },
        cancelAnimationFrame: function (id) {
            canvas.cancelAnimationFrame(id);
        }
    };
    document = {
        readyState: "complete",
        createElement: function (type) {
            switch (type) {
            case "canvas":
                return canvas;
            case "img":
                var imageTag = Qt.createQmlObject("HtmlImg {}", canvasItem, "createElement(img)");
                return imageTag;
            }

            throw new TypeError("Cannot create element of type '" + type + "'");
        },
        createElementNS: function (namespace, type) {
            //throw new TypeError("Cannot create element of type '" + type + "' in namespace '" + namespace + "'");
            return {};
        },
        getElementsByTagName: function (tagName) {
            var elements = [];
            if (tagName === "canvas") {
                elements.push(canvas);
            }
            return elements;
        },
        getElementsByClassName: function (className) {
            var elements = [];
            // find certain tags?
            return elements;
        }
    };

    // First check whether there is a non-minified version for debugging purposes
    var url = "./lottie.js";
    var lottieJs = Qt.include(url);
    // Don't check exception here as we don't want to fall back to minified
    // if you did a typo while developing.
    if (lottieJs.status === 2) {
        url = "./lottie.min.js";
        lottieJs = Qt.include(url);
    }

    // FIXME Qt docs mention "result.EXCEPTION" as "3" but how to use that enum value here?
    if (lottieJs.status === 3) {
        // forward thrown exception to caller
        throw lottieJs.exception;
    } else if (lottieJs.status === 2) { // result.NETWORK_ERROR
        console.warn("Failed to load", url, lottieJs.statusText);
        throw new Error("Failed to load lottie.min.js");
    }

    console.log(d.log, "Using", url);

    return window.lottie;
}
