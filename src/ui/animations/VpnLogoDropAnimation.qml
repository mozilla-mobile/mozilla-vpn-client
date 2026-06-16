// Generated from Lottie file vpnlogo-drop_animation.json
import QtQuick
import QtQuick.VectorImage
import QtQuick.VectorImage.Helpers
import QtQuick.Shapes
import QtQuick.Effects
import Qt.labs.lottieqt.VectorImageHelpers

Item {
    implicitWidth: 1240
    implicitHeight: 1240
    component AnimationsInfo : QtObject
    {
        property bool paused: false
        property int loops: 1
        signal restart()
    }
    property AnimationsInfo animations : AnimationsInfo {}
    transform: [
        Scale { xScale: width / 1240; yScale: height / 1240 }
    ]
    objectName: "_q_animation"
    id: _qt_node0
    transformOrigin: Item.TopLeft
    LayerItem {
        objectName: "Connector"
        id: _qt_node1
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node1_transform_base_group
            TransformGroup {
                id: _qt_node1_transform_group_0
                Translate { x: 619.795; y: 890.713 }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node1_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node1_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node1.opacity = 0
                    }
                    PauseAnimation { duration: 1000 }
                    PropertyAnimation {
                        duration: 500
                        target: _qt_node1
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 1500 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Connector"
            id: _qt_node2
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node3_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -129.602 -56.003 L -93.068 -92.1935 L -56.534 -128.384 Q -46.8484 -103.955 -22.188 -94.36 L -93.158 -24.057 L 96.129 -24.057 Q 91.075 -12.5992 91.075 0.004 Q 91.075 18.8426 101.879 34.2603 Q 112.436 49.3264 129.6 56.005 L 93.0675 92.1945 L 67.9514 117.075 L 56.535 128.384 Q 46.8477 103.957 22.186 94.363 L 57.672 59.211 L 93.158 24.059 L -96.124 24.059 Q -91.072 12.6073 -91.072 0 Q -91.072 -18.84 -101.877 -34.258 Q -112.436 -49.3244 -129.602 -56.003 M 208.324 -21.978 L 208.652 -22.3028 L 209.374 -23.0175 L 210.424 -24.057 L 207.454 -24.057 L 208.324 -21.978 M -207.452 24.059 L -208.323 21.979 L -209.374 23.019 L -210.424 24.059 L -207.452 24.059 " }
            }
        }
    }
    LayerItem {
        objectName: "Circle Top"
        id: _qt_node4
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node4_transform_base_group
            TransformGroup {
                id: _qt_node4_transform_group_0
                Translate { id: _qt_node4_transform_0_1 }
                Translate { x: 619.796; y: 0 }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node4_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node4_transform_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                loops: 1
                ParallelAnimation {
                    SequentialAnimation {
                        ParallelAnimation {
                            ScriptAction {
                                script:_qt_node4_transform_0_1.x = 0
                            }
                            ScriptAction {
                                script:_qt_node4_transform_0_1.y = 209.355
                            }
                        }
                        PauseAnimation { duration: 250 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node4_transform_0_1
                                property: "x"
                                to: 0
                                easing: _qt_node0.easing_01
                            }
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node4_transform_0_1
                                property: "y"
                                to: 740.4
                                easing: _qt_node0.easing_01
                            }
                        }
                        PauseAnimation { duration: 2250 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node4_transform_base_group.deactivateOverride(_qt_node4_transform_group_0)
                    }
                }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node4_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node4_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node4.opacity = 0
                    }
                    PauseAnimation { duration: 83 }
                    PropertyAnimation {
                        duration: 500
                        target: _qt_node4
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 2417 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Circle Top"
            id: _qt_node5
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node6_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -36.429 0 Q -36.429 -14.9483 -25.7595 -25.5176 Q -15.09 -36.087 0 -36.087 Q 15.0894 -36.087 25.7591 -25.5176 Q 36.429 -14.9481 36.429 0 Q 36.429 14.9475 25.7591 25.5173 Q 15.0892 36.087 0 36.087 Q -15.0898 36.087 -25.7595 25.5173 Q -36.429 14.9477 -36.429 0 M 0 -84.203 Q -35.2095 -84.203 -60.1063 -59.5405 Q -85.003 -34.878 -85.003 0 Q -85.003 34.878 -60.1063 59.5405 Q -35.2095 84.203 0 84.203 Q 35.2089 84.203 60.1059 59.5405 Q 85.003 34.8779 85.003 0 Q 85.003 -34.8779 60.1059 -59.5405 Q 35.2089 -84.203 0 -84.203 " }
            }
        }
    }
    LayerItem {
        objectName: "Cirlce Bottom"
        id: _qt_node7
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node7_transform_base_group
            TransformGroup {
                id: _qt_node7_transform_group_0
                Translate { id: _qt_node7_transform_0_1 }
                Translate { x: 619.796; y: 0 }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node7_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node7_transform_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                loops: 1
                ParallelAnimation {
                    SequentialAnimation {
                        ParallelAnimation {
                            ScriptAction {
                                script:_qt_node7_transform_0_1.x = 0
                            }
                            ScriptAction {
                                script:_qt_node7_transform_0_1.y = 276.082
                            }
                        }
                        PauseAnimation { duration: 83 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node7_transform_0_1
                                property: "x"
                                to: 0
                                easing: _qt_node0.easing_01
                            }
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node7_transform_0_1
                                property: "y"
                                to: 1041.1
                                easing: _qt_node0.easing_01
                            }
                        }
                        PauseAnimation { duration: 2417 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node7_transform_base_group.deactivateOverride(_qt_node7_transform_group_0)
                    }
                }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node7_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node7_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node7.opacity = 0
                    }
                    PauseAnimation { duration: 83 }
                    PropertyAnimation {
                        duration: 500
                        target: _qt_node7
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 2417 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Cirlce Bottom"
            id: _qt_node8
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node9_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -36.43 0 Q -36.43 -14.9474 -25.7596 -25.5173 Q -15.0893 -36.087 0 -36.087 Q 15.0893 -36.087 25.7596 -25.5173 Q 36.43 -14.9474 36.43 0 Q 36.43 14.9474 25.7596 25.5173 Q 15.0893 36.087 0 36.087 Q -15.0893 36.087 -25.7596 25.5173 Q -36.43 14.9474 -36.43 0 M 0 -84.203 Q -35.2089 -84.203 -60.1059 -59.5405 Q -85.003 -34.8779 -85.003 0 Q -85.003 34.8779 -60.1059 59.5405 Q -35.2089 84.203 0 84.203 Q 35.2095 84.203 60.1063 59.5405 Q 85.003 34.878 85.003 0 Q 85.003 -34.878 60.1063 -59.5405 Q 35.2095 -84.203 0 -84.203 " }
            }
        }
    }
    LayerItem {
        objectName: "Circle Left"
        id: _qt_node10
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node10_transform_base_group
            TransformGroup {
                id: _qt_node10_transform_group_0
                Translate { id: _qt_node10_transform_0_1 }
                Translate { x: 468.002; y: 0 }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node10_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node10_transform_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                loops: 1
                ParallelAnimation {
                    SequentialAnimation {
                        ParallelAnimation {
                            ScriptAction {
                                script:_qt_node10_transform_0_1.x = 0
                            }
                            ScriptAction {
                                script:_qt_node10_transform_0_1.y = 157.716
                            }
                        }
                        PauseAnimation { duration: 117 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node10_transform_0_1
                                property: "x"
                                to: 0
                                easing: _qt_node0.easing_01
                            }
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node10_transform_0_1
                                property: "y"
                                to: 890.7
                                easing: _qt_node0.easing_01
                            }
                        }
                        PauseAnimation { duration: 2383 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node10_transform_base_group.deactivateOverride(_qt_node10_transform_group_0)
                    }
                }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node10_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node10_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node10.opacity = 0
                    }
                    PauseAnimation { duration: 83 }
                    PropertyAnimation {
                        duration: 500
                        target: _qt_node10
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 2417 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Circle Left"
            id: _qt_node11
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node12_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -36.43 0 Q -36.43 -14.9476 -25.76 -25.5173 Q -15.0899 -36.087 0 -36.087 Q 15.0893 -36.087 25.7596 -25.5173 Q 36.43 -14.9474 36.43 0 Q 36.43 14.9474 25.7596 25.5173 Q 15.0893 36.087 0 36.087 Q -15.0899 36.087 -25.76 25.5173 Q -36.43 14.9476 -36.43 0 M 0 -84.203 Q -35.2095 -84.203 -60.1063 -59.5405 Q -85.003 -34.878 -85.003 0 Q -85.003 34.878 -60.1063 59.5405 Q -35.2095 84.203 0 84.203 Q 35.2095 84.203 60.1063 59.5405 Q 85.003 34.878 85.003 0 Q 85.003 -34.878 60.1063 -59.5405 Q 35.2095 -84.203 0 -84.203 " }
            }
        }
    }
    LayerItem {
        objectName: "Circle Right"
        id: _qt_node13
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node13_transform_base_group
            TransformGroup {
                id: _qt_node13_transform_group_0
                Translate { id: _qt_node13_transform_0_1 }
                Translate { x: 771.58; y: 0 }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node13_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node13_transform_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                loops: 1
                ParallelAnimation {
                    SequentialAnimation {
                        ParallelAnimation {
                            ScriptAction {
                                script:_qt_node13_transform_0_1.x = 0
                            }
                            ScriptAction {
                                script:_qt_node13_transform_0_1.y = 89.716
                            }
                        }
                        PauseAnimation { duration: 167 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node13_transform_0_1
                                property: "x"
                                to: 0
                                easing: _qt_node0.easing_01
                            }
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node13_transform_0_1
                                property: "y"
                                to: 890.7
                                easing: _qt_node0.easing_01
                            }
                        }
                        PauseAnimation { duration: 2333 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node13_transform_base_group.deactivateOverride(_qt_node13_transform_group_0)
                    }
                }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node13_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node13_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node13.opacity = 0
                    }
                    PauseAnimation { duration: 83 }
                    PropertyAnimation {
                        duration: 500
                        target: _qt_node13
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 2417 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Circle Right"
            id: _qt_node14
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node15_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -36.43 0 Q -36.43 -14.9474 -25.7596 -25.5173 Q -15.0893 -36.087 0 -36.087 Q 15.0893 -36.087 25.7596 -25.5173 Q 36.43 -14.9474 36.43 0 Q 36.43 14.9474 25.7596 25.5173 Q 15.0893 36.087 0 36.087 Q -15.0893 36.087 -25.7596 25.5173 Q -36.43 14.9474 -36.43 0 M 0 -84.203 Q -35.2095 -84.203 -60.1063 -59.5405 Q -85.003 -34.878 -85.003 0 Q -85.003 34.878 -60.1063 59.5405 Q -35.2095 84.203 0 84.203 Q 35.2095 84.203 60.1063 59.5405 Q 85.003 34.878 85.003 0 Q 85.003 -34.878 60.1063 -59.5405 Q 35.2095 -84.203 0 -84.203 " }
            }
        }
    }
    readonly property easingCurve easing_01: ({ type: Easing.BezierSpline, bezierCurve: [ 0.17, 0.89, 0.32, 1.27, 1, 1 ] })
    readonly property easingCurve easing_00: ({ type: Easing.BezierSpline, bezierCurve: [ 0.42, 0, 0.58, 1, 1, 1 ] })
}
