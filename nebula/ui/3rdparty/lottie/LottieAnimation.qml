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

import QtQuick 2.9

import "./src/lottie_shim.js" as Lottie

/**
 * LottieAnimation
 */
Item {
    id: lottieItem

    /**
     * Animation to display, can be
     * \li URL: absolute URL to an animation JSON file, including qrc:/
     * \li filename: relative path to an animation JSON file
     * \li JSON data (must be an Object at root level)
     * \li a JavaScript Object
     */
    property var source

    /**
     * Load the image asynchronously, default is false.
     *
     * @note the animation is always created/processed synchronously,
     * this is only about loading the file itself, e.g. downloading it from a remote location
     */
    // TODO
    //property bool asynchronous: false

    /**
     * Load status of the animation file
     * \li Image.Null: no source set
     * \li Image.Ready: animation file was successfully loaded and processed
     * \li Image.Loading: animation file is currently being downloaded
     * \li Image.Error: animation file failed to load or process,
                        \c errorString will contain more details
     */
    property int status: Image.Null

    /**
     * Error string explaining why the animation file failed to load
     * Only set when \c status is \c Image.Error
     */
    property string errorString

    /**
     * Whether the animation is and should be running
     * Setting this to true or false is the same as calling start() or pause(), respectively.
     */
    property bool running: false

    /**
     * How often to repeat the animation, default is 0, i.e. it runs only once.
     *
     * Set to Animation.Infinite to repeat the animation indefinitely
     *
     * @note Changing this property causes the animation to be recreated
     */
    property int loops: 0

    /**
     * Speed modifier at which the animation is played, default is 1x speed.
     */
    property real speed: 1

    /**
     * Play the animation in reverse (from end to start)
     */
    property bool reverse: false

    /**
     * Whether to clear the canvas before rendering, default is true.
     *
     * Disable if you are painting a full screen scene anyway.
     * @note Changing this property causes the animation to be recreated
     * @note Disabling this can cause unwanted side-effects in the QtQuick
     * canvas when it tries to re-render only specific areas.
     */
    property bool clearBeforeRendering: true

    /**
     * How to behave when the image item has a sice different from the animation's native size,
     * \li Image.Stretch (default): the animation is scaled to fit
     * \li Image.PreserveAspectFit: the image is scaled uniformly to fit without cropping
     * \li Image.PreserveAspectCrop: the image is scaled uniformly to fill, cropping if necessary
     * \li Image.Pad: the image is not transformed
     *
     * @note implicitWidth and implicitHeight of this item are set to the animation's native size
     * so by default this item will have the correct size
     * @note Changing this property causes the animation to be recreated
     */
    property int fillMode: Image.Stretch

    /**
     * When the animation finishes, call stop(), default is false.
     *
     * This will revert the animation to its first frame when finished
     */
    //property bool stopWhenComplete: false

    property alias renderStrategy: canvas.renderStrategy

    property alias renderTarget: canvas.renderTarget

    /**
     * Emitted when the last loop of the animation finishes.
     * @note This does not work right now
     */
    signal finished()

    /**
     * Emitted when a loop of the animation finishes.
     * @param currentLoop The number of the loop that just finished.
     * @note This does not work right now
     */
    signal loopFinished(int currentLoop)

    /**
     * Start the animation.
     *
     * This is the same as setting running to true.
     */
    // Start the animation, restarts if already running
    function start() {
        if (d.animationItem) {
            d.animationItem.play();
            running = true;
        }
    }

    /**
     * Pause the animation.
     *
     * This is the same as setting running to false.
     */
    function pause() {
        if (d.animationItem) {
            d.animationItem.pause();
            running = false;
        }
    }

    /**
     * Stop the animation.
     *
     * Stops playback and rewinds the animation to the beginning.
     */
    function stop() {
        if (d.animationItem) {
            d.animationItem.stop();
            running = false;
        }
    }

    /**
     * Clear the animation canvas
     */
    function clear() {
        if (!canvas.available) {
            return;
        }

        var ctx = canvas.getContext("2d");
        if (ctx) {
            ctx.reset();
        }
        canvas.requestPaint();
    }

    // Private API
    QtObject {
        id: d

        property bool componentComplete: false

        // The actual animation data used
        property var animationData

        // The "AnimationItem" created by lottie.js that does the animation and everything
        property var animationItem

        // When recreating the animation when changing properties, jump to this frame
        // to provide a seamless experience
        property real pendingRawFrame: -1

        readonly property LoggingCategory log: LoggingCategory {
            name: "org.kde.lottie"
            // TODO needs bump to Qt 5.12, is it worth it?
            //defaultLogLevel: LoggingCategory.Info
        }

        onAnimationDataChanged: {
            destroyAnimation();

            // Avoid repeated access to this property containing lots of data
            var data = animationData;

            if (!data) {
                errorString = "";
                status = Image.Null;
                return;
            }

            if (typeof data !== "object") {
                errorString = "animationData is not an object, this should not happen";
                status = Image.Error;
                return;
            }

            var width = data.w || 0;
            var height = data.h || 0;

            if (width <= 0 || height <= 0) {
                errorString = "Animation data does not contain valid size information";
                status = Image.Error;
                return;
            }

            lottieItem.implicitWidth = width;
            lottieItem.implicitHeight = height;

            playIfShould();
        }

        // TODO clean that up a bit
        readonly property bool shouldPlay: canvas.available && componentComplete
                                           && lottieItem.width > 0 && lottieItem.height > 0

        onShouldPlayChanged: {
            if (!shouldPlay) {
                // TODO stop
                return;
            }

            playIfShould();
        }

        function setAnimationDataJson(json) {
            animationData = undefined;
            try {
                animationData = JSON.parse(json);
            } catch (e) {
                errorString = e.toString();
                status = Image.Error;
            }
        }

        function playIfShould() { // TODO better name
            if (!shouldPlay) {
                return;
            }

            var data = animationData;
            if (!data) {
                return;
            }

            console.log(d.log, "Initializing Lottie Animation");
            var lottie = Lottie.initialize(canvas, d.log);

            var aspectRatio = "none";

            switch (lottieItem.fillMode) {
            case Image.Pad:
                aspectRatio = "orig"; // something other than empty string
                break;
            case Image.PreserveAspectCrop:
                // TODO make position also configurable like Image has it
                aspectRatio = "xMidYMid slice";
                break;
            case Image.PreserveAspectFit:
                aspectRatio = "xMidYMid meet";
                break;
            }

            var loop = false;
            if (lottieItem.loops === Animation.Infinite) {
                loop = true;
            } else if (lottieItem.loops > 0) {
                loop = lottieItem.loops;
            }

            animationItem = lottie.loadAnimation({
                container: container,
                renderer: "canvas",
                rendererSettings: {
                    clearCanvas: lottieItem.clearBeforeRendering,
                    preserveAspectRatio: aspectRatio
                },
                loop: loop,
                autoplay: lottieItem.running,
                animationData: data
            });

            animationItem.setSpeed(lottieItem.speed);
            animationItem.setDirection(lottieItem.reverse ? -1 : 1);

            // TODO should we expose enterFrame event?
            animationItem.addEventListener("complete", function(e) {
                running = false;
                // FIXME throws "lottieAnim is not defined" at times
                //lottieAnim.finished();

                // Cannot do "Play" again when complete
                // Figure out a better way than this, or call stop/rewind automatically when complete
            });

            animationItem.addEventListener("loopComplete", function(e) {
                // FIXME throws "lottieAnim is not defined" at times
                //lottieAnim.loopFinished(e.currentLoop);
            });

            if (pendingRawFrame >= 0) {
                animationItem.setCurrentRawFrameValue(pendingRawFrame);
            }
            pendingRawFrame = -1;

            status = Image.Ready;
            errorString = "";
        }

        function destroyAndRecreate() {
            console.log(d.log, "destroy and recreate");
            if (animationItem) {
                d.pendingRawFrame = animationItem.currentRawFrame;
            }

            destroyAnimation();
            playIfShould();
        }

        function destroyAnimation() {
            if (animationItem) {
                animationItem.destroy();
                animationItem = null;
            }
            lottieItem.clear();
        }

        function updateAnimationSize() {
            if (animationItem) {
                lottieItem.clear();
                animationItem.resize();
            }
        }

        Component.onCompleted: {
            componentComplete = true;
        }
    }

    // Should we move these handlers into a Connections {} within the private?
    onRunningChanged: {
        if (running) {
            start();
        } else {
            pause();
        }
    }

    onWidthChanged: Qt.callLater(d.updateAnimationSize)
    onHeightChanged: Qt.callLater(d.updateAnimationSize)

    // TODO Would be lovely if we could change those at runtime without recreating the animation
    onLoopsChanged: Qt.callLater(d.destroyAndRecreate)
    onClearBeforeRenderingChanged: Qt.callLater(d.destroyAndRecreate)
    onFillModeChanged: Qt.callLater(d.destroyAndRecreate)

    onSpeedChanged: {
        if (d.animationItem) {
            d.animationItem.setSpeed(speed);
        }
    }
    onReverseChanged: {
        if (d.animationItem) {
            d.animationItem.setDirection(reverse ? -1 : 1);
        }
    }

    onSourceChanged: {
        // is already JS object, use verbatim
        if (typeof source === "object") { // TODO what about QUrl, I think it is treated as {} here
            console.log(d.log, "Using source verbatim as it is an object");
            d.animationData = source;
            return;
        }

        var sourceString = source.toString();

        if (sourceString.indexOf("{") === 0) { // startsWith("{"), assume JSON
            console.log(d.log, "Using source as JSON");
            d.setAnimationDataJson(sourceString);
            return;
        }

        d.animationData = null;
        if (!source) {
            return;
        }

        var url = source.toString(); // toString in case is QUrl
        if (url.indexOf("/") === 0) { // assume local file
            url = "file://" + url;
        } else if (url.indexOf(":/") === -1) { // assume relative url
            // FIXME figure out how to do relative URLs with Ajax
            // Qt.resolvedUrl is relative to *this* file, not the one where the item is actually used from
        }

        // NOTE QML LoggingCategory {} has its internal QLoggingCategory created in
        // componentCompleted(). There seems to be a situation where this console.log
        // is executed before the LoggingCategory {} object above has completed.
        //console.log(d.log, "Fetching source from", url);

        var xhr = new XMLHttpRequest()
        // FIXME allow asynchronous
        xhr.open("GET", url, false /*synchronous*/);
        xhr.send(null /*payload*/);

        // NOTE KIO AccessManager in contrast to QNetworkAccessManager doesn't set a HTTP status code
        // when loading local files via Ajax so we can't check xhr.status === 200 here

        var text = xhr.responseText;
        if (text.length < 10) {
            errorString = xhr.statusText || xhr.status || "Failed to load " + url;
            status = Image.Error;
            return;
        }

        d.setAnimationDataJson(text);
    }

    // When re-parenting the item, re-initialize the animation
    // as the drawing context might become invalidated and since it's
    // stored in a variable by Lottie, we would crash somewhere in Qt.
    onParentChanged: Qt.callLater(d.destroyAndRecreate);

    Item {
        id: container
        anchors.fill: parent

        // HTML DOM compatibility API
        property real offsetWidth: width
        property real offsetHeight: height
        property var style: ({})
        property string innerHTML
        property string innerText
        property var itemData: []
        function appendChild(item) {
            item.parent = this;
        }
    }

    Canvas {
        id: canvas
        anchors.fill: parent

        // HTML DOM compatibility API
        property real offsetWidth: width
        property real offsetHeight: height
        property var style: ({})
        property var itemData: []
        function setAttribute(name, value) {
            // does nothing
        }
    }

}
