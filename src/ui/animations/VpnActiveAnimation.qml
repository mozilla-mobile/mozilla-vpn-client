// Generated from Lottie file vpnactive_animation.json
import QtQuick
import QtQuick.VectorImage
import QtQuick.VectorImage.Helpers
import QtQuick.Shapes
import QtQuick.Effects
import Qt.labs.lottieqt.VectorImageHelpers

Item {
    implicitWidth: 1240
    implicitHeight: 708
    component AnimationsInfo : QtObject
    {
        property bool paused: false
        property int loops: 1
        signal restart()
    }
    property AnimationsInfo animations : AnimationsInfo {}
    transform: [
        Scale { xScale: width / 1240; yScale: height / 708 }
    ]
    objectName: "_q_animation"
    id: _qt_node0
    transformOrigin: Item.TopLeft
    LayerItem {
        objectName: "Vector"
        id: _qt_node1
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node1_transform_base_group
            TransformGroup {
                id: _qt_node1_transform_group_0
            }
            Translate {
                x: _qt_node1_motion_animation.currentInterpolator.x
                y: _qt_node1_motion_animation.currentInterpolator.y
            }
            Matrix4x4 { matrix: _qt_node2.transformMatrix }
        }
        QtObject {
            id: _qt_node1_pathInterpolator_0
            property real x: 42.262
            property real y: 41.06
            property real angle: -132.285
        }
        PathInterpolator {
            id: _qt_node1_pathInterpolator_2
            path: Path { PathSvg { path: "M 42.262 41.06 C 40.595 39.227 32.262 30.06 32.262 30.06 " } }
        }
        PathInterpolator {
            id: _qt_node1_pathInterpolator_3
            path: Path { PathSvg { path: "M 32.262 30.06 C 32.262 30.06 40.595 39.227 42.262 41.06 " } }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node1_motion_animation.restart() } }
        SequentialAnimation {
            id: _qt_node1_motion_animation
            property var currentInterpolator: _qt_node1_pathInterpolator_0
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            PauseAnimation { duration: 2500 }
            ScriptAction {
                script: {
                    _qt_node1_motion_animation.currentInterpolator = _qt_node1_pathInterpolator_2
                }
            }
            PropertyAnimation {
                id: _qt_node1_motionAnimation_2
                duration: 167
                target: _qt_node1_pathInterpolator_2
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_00
            }
            ScriptAction {
                script: {
                    _qt_node1_motion_animation.currentInterpolator = _qt_node1_pathInterpolator_3
                }
            }
            PropertyAnimation {
                id: _qt_node1_motionAnimation_3
                duration: 183
                target: _qt_node1_pathInterpolator_3
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_00
            }
            PauseAnimation { duration: 5150 }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node3
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node4_path_fill_stroke
                objectName: "svg_path:Rectangle Path 1"
                strokeColor: "transparent"
                fillColor: "#ffa77ffa"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -33 -33 L 33 -33 L 33 33 L -33 33 L -33 -33 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node5
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node5_transform_base_group
            Translate { x: 31.782; y: 31.782}
            Matrix4x4 { matrix: _qt_node2.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node6
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node7_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "#00000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 31.782 -31.782 L -31.782 -31.782 L -31.782 31.782 L 31.782 31.782 L 31.782 -31.782 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node8
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node8_transform_base_group
            Translate { x: 32.844; y: 32.841}
            Matrix4x4 { matrix: _qt_node2.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node9
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node10_path_stroke
                objectName: "svg_stroke_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.RoundJoin
                miterLimit: 0
                fillColor: "transparent"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic
                PathSvg { path: "M 10.595 -8.475 L 4.2375 0 L -2.12 8.475 L -6.3575 4.2375 L -9.27078 1.32422 L -10.595 0 " }
            }
        }
    }
    LayerItem {
        objectName: " Check"
        id: _qt_node2
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node2_transform_base_group
            TransformGroup {
                id: _qt_node2_transform_group_0
                Translate { x: -37.631; y: -37.03 }
                Scale { id: _qt_node2_transform_0_3}
                Translate { x: 0; y: 365.97 }
                Translate { id: _qt_node2_transform_0_0 }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node2_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node2_transform_animation
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
                                script:_qt_node2_transform_0_0.x = 620.369
                            }
                            ScriptAction {
                                script:_qt_node2_transform_0_0.y = 0
                            }
                        }
                        PauseAnimation { duration: 750 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node2_transform_0_0
                                property: "x"
                                to: 457.369
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node2_transform_0_0
                                property: "y"
                                to: 0
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 4250 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node2_transform_0_0
                                property: "x"
                                to: 620.369
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node2_transform_0_0
                                property: "y"
                                to: 0
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 2500 }
                    }
                    SequentialAnimation {
                        ParallelAnimation {
                            ScriptAction {
                                script:_qt_node2_transform_0_3.xScale = 0.1
                            }
                            ScriptAction {
                                script:_qt_node2_transform_0_3.yScale = 0.1
                            }
                        }
                        PauseAnimation { duration: 750 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node2_transform_0_3
                                property: "xScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node2_transform_0_3
                                property: "yScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 4250 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node2_transform_0_3
                                property: "xScale"
                                to: 0.1
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node2_transform_0_3
                                property: "yScale"
                                to: 0.1
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 2500 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node2_transform_base_group.deactivateOverride(_qt_node2_transform_group_0)
                    }
                }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node2_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node2_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node2.opacity = 0
                    }
                    PauseAnimation { duration: 750 }
                    PropertyAnimation {
                        duration: 250
                        target: _qt_node2
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 4250 }
                    PropertyAnimation {
                        duration: 250
                        target: _qt_node2
                        property: "opacity"
                        to: 0
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 2500 }
                }
            }
        }
    }
    LayerItem {
        objectName: "Shadow"
        id: _qt_node11
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node11_transform_base_group
            Translate { x: 670.5; y: 378}
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Shadow"
            id: _qt_node12
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node13_path_fill_stroke
                objectName: "svg_path:Rectangle Path 1"
                strokeColor: "transparent"
                fillColor: "#ffc5c8fb"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -121.5 -150 Q -121.5 -159.113 -115.056 -165.556 Q -108.613 -172 -99.5 -172 L 99.5 -172 Q 108.613 -172 115.056 -165.556 Q 121.5 -159.113 121.5 -150 L 121.5 150 Q 121.5 159.113 115.056 165.556 Q 108.613 172 99.5 172 L -99.5 172 Q -108.613 172 -115.056 165.556 Q -121.5 159.113 -121.5 150 L -121.5 -150 " }
            }
        }
    }
    LayerItem {
        objectName: "Ellipse 1"
        id: _qt_node14
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node14_transform_base_group
            Translate { x: 42.956; y: 17.876}
            Matrix4x4 { matrix: _qt_node15.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Ellipse 1"
            id: _qt_node16
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node17_path_fill_stroke
                objectName: "svg_path:Ellipse Path 1"
                strokeColor: "transparent"
                fillColor: "#ffa77ffa"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 0 -17.5 Q 7.24874 -17.5 12.3744 -12.3744 Q 17.5 -7.24874 17.5 0 Q 17.5 7.24874 12.3744 12.3744 Q 7.24874 17.5 0 17.5 Q -7.24874 17.5 -12.3744 12.3744 Q -17.5 7.24874 -17.5 0 Q -17.5 -7.24874 -12.3744 -12.3744 Q -7.24874 -17.5 0 -17.5 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node18
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node18_transform_base_group
            Translate { x: 39.386; y: 39.92}
            Matrix4x4 { matrix: _qt_node15.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node19
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node20_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 37.092 -27.622 Q 37.3745 -27.5364 37.605 -27.344 Q 37.8483 -27.1665 37.998 -26.912 Q 38.1495 -26.6544 38.186 -26.358 Q 38.195 -26.2645 38.186 -26.171 L 37.847 1.389 Q 37.8408 1.87013 37.551 2.254 Q 37.262 2.63678 36.801 2.774 L -16.118 18.654 L -14.642 20.741 Q -13.3791 22.4432 -11.434 23.289 Q -9.48944 24.1346 -7.383 23.898 Q -7.14578 23.8728 -6.902 23.8311 L -6.439 23.741 L 11.4635 19.5185 L 29.366 15.296 Q 29.8549 15.1802 30.34 15.127 Q 32.7943 14.857 34.985 15.993 Q 37.1772 17.1297 38.37 19.29 Q 39.5634 21.4513 39.358 23.91 Q 39.1524 26.3716 37.618 28.303 Q 36.0826 30.2357 33.734 30.993 Q 31.3839 31.7508 29.01 31.078 Q 26.6342 30.4047 25.032 28.528 Q 23.4276 26.6487 23.135 24.201 Q 23.0296 23.266 23.135 22.331 L 23.432 19.761 L 12.156 22.4375 L 0.88 25.114 L 1.588 26.194 L 2.296 27.274 Q 3.36458 28.8906 3.585 30.812 Q 3.91649 33.8372 2.199 36.353 Q 0.43147 38.9685 -2.64 39.687 Q -3.14608 39.8042 -3.675 39.862 Q -5.31054 40.0621 -6.889 39.589 Q -8.46645 39.1162 -9.724 38.047 Q -12.2913 35.9159 -12.676 32.603 Q -12.9129 30.3684 -11.962 28.332 L -11.206 26.687 L -12.0225 26.306 L -12.839 25.925 Q -16.9045 24.0265 -18.453 19.815 Q -18.453 19.797 -18.444 19.8086 Q -18.441 19.8125 -18.453 19.779 L -18.604 19.331 L -28.0502 -8.94622 L -31.876 -20.4 L -32.136 -21.1865 L -32.396 -21.973 L -33.206 -21.7839 L -34.988 -21.368 L -37.58 -20.763 L -37.768 -20.763 Q -38.3192 -20.6993 -38.772 -21.023 Q -39.2418 -21.3377 -39.377 -21.888 Q -39.3861 -21.979 -39.377 -22.07 Q -39.4407 -22.6212 -39.117 -23.074 Q -38.8023 -23.5438 -38.252 -23.679 L -31.749 -25.215 L -31.592 -25.245 Q -31.0739 -25.3071 -30.636 -25.028 Q -30.1999 -24.7429 -30.031 -24.247 L -26.226 -12.796 L -13.873 -15.724 L -14.303 -17.496 Q -15.1536 -21.0178 -14.585 -24.602 Q -14.0173 -28.1809 -12.115 -31.27 Q -10.2115 -34.3612 -7.273 -36.478 Q -4.33016 -38.598 -0.801 -39.425 Q 0.287446 -39.6812 1.377 -39.812 Q 8.2306 -40.563 13.867 -36.586 Q 19.5017 -32.6102 21.091 -25.899 L 21.508 -24.126 L 28.9245 -25.8835 L 36.341 -27.641 Q 36.4255 -27.6651 36.51 -27.677 Q 36.8056 -27.7088 37.092 -27.622 M 29.261 18.361 Q 27.8325 18.9382 26.946 20.202 Q 25.8195 21.8328 26.057 23.801 Q 26.0935 24.0976 26.166 24.406 Q 26.6299 26.3221 28.243 27.457 Q 29.8561 28.5919 31.816 28.381 Q 32.1272 28.3429 32.421 28.272 Q 34.3359 27.8084 35.471 26.195 Q 36.6059 24.5819 36.395 22.622 Q 36.3581 22.3222 36.286 22.017 Q 35.9382 20.5139 34.829 19.443 Q 33.7202 18.3724 32.205 18.078 Q 30.69 17.7836 29.261 18.361 M -5.749 26.584 Q -7.68377 27.0405 -8.822 28.671 L -8.822 28.683 Q -9.92015 30.2961 -9.7 32.234 Q -9.66307 32.5338 -9.591 32.839 Q -9.13354 34.7572 -7.52 35.894 Q -5.90928 37.0288 -3.947 36.813 Q -3.81055 36.7956 -3.65125 36.7682 L -3.342 36.71 Q -1.42665 36.2444 -0.298 34.632 Q 0.831291 33.0187 0.614 31.06 Q 0.576174 30.7513 0.499 30.456 Q 0.0483715 28.5307 -1.566 27.394 Q -3.18224 26.256 -5.144 26.481 Q -5.28533 26.4985 -5.44425 26.5257 L -5.749 26.584 M -0.118 -36.569 Q -5.77302 -35.2391 -9.083 -30.466 L -9.083 -30.454 Q -12.3729 -25.7519 -11.732 -20.049 Q -11.6267 -19.1296 -11.411 -18.234 Q -10.0703 -12.623 -5.354 -9.303 Q -0.636909 -5.98242 5.097 -6.614 Q 6.01227 -6.71736 6.912 -6.928 Q 12.5224 -8.26972 15.845 -12.989 Q 19.1666 -17.7068 18.538 -23.443 Q 18.4316 -24.3703 18.224 -25.257 Q 16.8833 -30.8742 12.16 -34.199 Q 7.43755 -37.5232 1.697 -36.89 Q 0.777522 -36.7817 -0.118 -36.569 M 34.901 0.27 L 34.901 -1.061 L 35.222 -24.314 L 28.4345 -22.711 L 21.647 -21.108 L 21.6253 -20.9075 L 21.5775 -20.4665 L 21.508 -19.825 Q 20.8922 -14.1427 17.153 -9.82 Q 12.4958 -4.44236 5.423 -3.68 Q 0.272453 -3.1157 -4.408 -5.345 Q -9.08769 -7.57393 -11.895 -11.931 L -12.591 -13.008 L -18.9275 -11.4955 L -25.264 -9.983 L -20.966 2.8805 L -16.668 15.744 L 34.901 0.27 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node21
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node21_transform_base_group
            Translate { x: 42.397; y: 17.802}
            Matrix4x4 { matrix: _qt_node15.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node22
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node23_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 5.746 -3.249 Q 6.01966 -3.68037 5.958 -4.186 Q 5.92611 -4.52312 5.747 -4.81 Q 5.56799 -5.09672 5.28 -5.275 Q 4.84863 -5.54866 4.343 -5.487 Q 4.00462 -5.45362 3.717 -5.271 Q 3.428 -5.08751 3.254 -4.797 L 1.152 -1.4035 L -0.95 1.99 L -3.721 0.284 Q -4.14931 0.0134047 -4.652 0.072 Q -4.9898 0.10987 -5.279 0.291 Q -5.5662 0.470876 -5.747 0.762 Q -6.0176 1.19031 -5.959 1.693 Q -5.92248 2.0339 -5.739 2.322 Q -5.5559 2.60952 -5.263 2.788 L -3.252 4.0315 L -1.241 5.275 Q -0.812076 5.54963 -0.303 5.486 Q 0.0352427 5.44943 0.321 5.268 Q 0.608285 5.0856 0.786 4.797 L 3.266 0.774 L 5.746 -3.249 " }
            }
        }
    }
    LayerItem {
        objectName: " Cart Only"
        id: _qt_node15
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node15_transform_base_group
            TransformGroup {
                id: _qt_node15_transform_group_0
                Translate { x: -39.386; y: -39.92 }
            }
            Translate {
                x: _qt_node15_motion_animation.currentInterpolator.x
                y: _qt_node15_motion_animation.currentInterpolator.y
            }
            Matrix4x4 { matrix: _qt_node24.transformMatrix }
        }
        QtObject {
            id: _qt_node15_pathInterpolator_0
            property real x: 94.379
            property real y: 201.949
            property real angle: -270
        }
        PathInterpolator {
            id: _qt_node15_pathInterpolator_2
            path: Path { PathSvg { path: "M 94.379 201.949 C 94.379 202.782 94.379 206.949 94.379 206.949 " } }
        }
        PathInterpolator {
            id: _qt_node15_pathInterpolator_3
            path: Path { PathSvg { path: "M 94.379 206.949 C 94.379 206.949 94.379 202.782 94.379 201.949 " } }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node15_motion_animation.restart() } }
        SequentialAnimation {
            id: _qt_node15_motion_animation
            property var currentInterpolator: _qt_node15_pathInterpolator_0
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            PauseAnimation { duration: 983 }
            ScriptAction {
                script: {
                    _qt_node15_motion_animation.currentInterpolator = _qt_node15_pathInterpolator_2
                }
            }
            PropertyAnimation {
                id: _qt_node15_motionAnimation_2
                duration: 2017
                target: _qt_node15_pathInterpolator_2
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_01
            }
            ScriptAction {
                script: {
                    _qt_node15_motion_animation.currentInterpolator = _qt_node15_pathInterpolator_3
                }
            }
            PropertyAnimation {
                id: _qt_node15_motionAnimation_3
                duration: 2000
                target: _qt_node15_pathInterpolator_3
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_01
            }
            PauseAnimation { duration: 3000 }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node25
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node25_transform_base_group
            Translate { x: 60.435; y: 82.289}
            Matrix4x4 { matrix: _qt_node26.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node27
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node28_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 58.685 -58.409 Q 60.2747 -62.4453 60.367 -66.787 L 60.434 -66.769 Q 60.4794 -69.8454 59.29 -72.687 Q 58.1001 -75.5298 55.879 -77.657 Q 53.0759 -80.3029 49.345 -81.257 Q 45.1819 -82.3588 40.877 -82.285 Q 35.3863 -82.2808 28.3528 -80.9642 Q 22.4643 -79.862 15.47 -77.833 Q 5.16859 -74.786 -4.746 -70.652 L -23.922 -62.304 L -24.2245 -62.165 L -24.527 -62.026 L -32.736 -58.112 L -35.458 -56.745 L -37.878 -55.535 L -40.418 -54.199 L -41.023 -53.869 L -41.4389 -53.6421 L -41.628 -53.539 L -42.983 -52.795 L -49.4162 -49.0882 Q -52.6758 -47.1276 -55.65 -45.155 L -57.187 -44.127 L -57.961 -43.606 L -58.566 -43.1935 L -59.171 -42.772 L -59.4735 -42.557 L -59.776 -42.342 Q -60.0474 -42.1589 -60.381 -41.907 L -60.381 -38.912 L -59.6355 -39.4579 L -59.171 -39.801 L -58.566 -40.2335 L -57.961 -40.666 Q -55.4355 -42.445 -52.831 -44.127 L -52.2655 -44.4865 L -51.8767 -44.7337 L -51.7 -44.846 Q -47.5666 -47.4705 -42.941 -50.061 L -40.406 -51.452 L -40.316 -51.507 L -38.534 -52.46 L -37.775 -52.862 L -35.162 -54.217 L -31.2134 -56.1994 L -27.237 -58.118 L -24.66 -59.328 L -24.576 -59.371 L -23.971 -59.649 L -3.857 -68.432 Q 5.93478 -72.5142 16.105 -75.522 Q 30.9254 -79.877 40.81 -79.877 Q 44.8016 -79.9555 48.65 -78.946 Q 51.7831 -78.1597 54.149 -75.957 Q 56.0476 -74.119 57.064 -71.675 Q 57.9977 -69.3249 57.978 -66.793 Q 57.8402 -62.2542 55.994 -58.106 Q 54.3746 -54.1799 51.405 -49.6577 Q 48.6597 -45.4773 44.797 -40.842 Q 35.6647 -29.8788 19.638 -15.695 L 8.132 -5.738 L 7.91428 -5.55264 L 0.492 0.916 L -11.9405 12.8807 L -24.019 25.258 L -24.2308 25.4838 L -24.624 25.899 L -26.167 27.568 L -26.2684 27.6788 L -28.133 29.734 L -29.984 31.803 L -32.778 35.009 L -32.986 35.251 L -34.333 36.824 L -34.7415 37.311 L -35.15 37.798 L -35.2963 37.9717 L -36.589 39.54 L -39.42 43.054 L -39.6366 43.331 L -40.113 43.9405 L -40.806 44.827 L -41.078 45.178 L -43.497 48.408 L -46.123 52.037 L -46.761 52.9445 L -47.1996 53.5684 L -47.399 53.852 L -48.0295 54.7595 L -48.651 55.667 L -48.742 55.8 L -48.833 55.933 L -49.359 56.7285 L -49.885 57.524 L -50.611 58.6467 L -51.095 59.399 L -51.985 60.809 L -53.715 63.652 L -54.555 65.085 L -55.11 66.0447 L -55.656 67.009 L -56.0752 67.7813 Q -56.5151 68.6052 -56.727 68.951 Q -56.9879 69.3767 -57.4499 70.2812 L -57.677 70.723 L -58.016 71.377 L -58.4015 72.1158 L -58.778 72.859 L -59.225 73.754 L -59.5985 74.5078 L -59.963 75.266 L -60.435 76.252 L -60.435 82.289 Q -60.382 82.1428 -60.3133 81.9775 L -60.187 81.684 L -60.078 81.43 L -59.656 80.458 L -59.225 79.489 Q -59.2007 79.4375 -59.1668 79.3537 Q -59.1136 79.2223 -59.056 79.107 L -58.936 78.8586 L -58.8153 78.5973 L -58.772 78.503 L -58.457 77.831 Q -58.3416 77.5613 -58.1079 77.0776 L -58.016 76.887 L -57.8481 76.5399 L -57.749 76.337 L -56.866 74.522 L -56.7337 74.2601 L -55.88 72.586 L -55.845 72.5189 L -54.924 70.772 L -54.168 69.38 L -53.9043 68.9003 L -53.636 68.425 L -52.535 66.489 L -51.845 65.279 L -51.5604 64.8053 L -51.192 64.19 L -50.8155 63.5588 L -50.43 62.932 L -49.48 61.402 Q -48.8574 60.3639 -47.726 58.679 Q -47.5334 58.3589 -47.1017 57.7252 L -46.855 57.361 L -46.776 57.24 L -46.3435 56.6015 L -45.911 55.963 L -44.242 53.544 L -40.146 47.948 L -39.733 47.4005 L -39.311 46.853 L -37.11 44.028 L -35.9 42.552 L -35.1209 41.5928 L -34.841 41.252 L -33.4968 39.618 L -33.165 39.219 L -32.9625 38.977 L -32.8233 38.8106 L -32.76 38.735 L -31.7425 37.5255 L -30.722 36.316 L -29.8145 35.2905 L -28.907 34.265 Q -28.7112 34.0338 -28.495 33.799 L -26.269 31.295 L -25.8232 30.7912 Q -25.0014 29.8574 -24.557 29.413 L -24.2528 29.0858 L -23.953 28.754 Q -18.1327 22.4352 -11.1036 15.458 L 2.222 2.615 Q 3.68267 1.23481 7.527 -2.054 L 9.85506 -4.0374 L 24.0055 -16.3386 Q 33.1014 -24.5318 39.77 -31.568 Q 49.1054 -41.4172 54.215 -49.571 Q 56.9009 -53.7646 58.685 -58.409 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node29
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node29_transform_base_group
            Translate { x: 35.121; y: 224.027}
            Matrix4x4 { matrix: _qt_node26.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node30
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node31_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 27.654 -20.755 L 31.353 -20.9 L 35.052 -21.045 L 35.0125 -22.261 L 34.973 -23.477 L 27.575 -23.181 Q 23.2311 -22.951 19.5306 -21.2873 Q 15.8301 -19.6235 12.773 -16.526 Q 9.56404 -13.4069 7.38779 -9.3934 Q 5.21154 -5.3799 4.068 -0.472 Q 3.17811 3.43388 1.376 7.011 L 1.08025 7.57875 L 0.771 8.142 Q 0.317229 8.97141 -0.191 9.739 L -0.35925 10.002 Q -0.45228 10.1427 -0.541 10.265 Q -2.03963 12.5107 -3.965 14.391 Q -6.88097 17.2274 -10.184 18.7187 Q -13.487 20.2099 -17.177 20.356 L -24.9045 20.658 L -30.2172 20.8656 L -32.632 20.96 L -35.052 21.057 L -35.052 23.477 L -32.632 23.38 L -17.08 22.775 Q -8.76929 22.4398 -2.278 16.121 L -1.473 15.298 Q -0.67061 14.4579 0.051 13.538 L 0.245739 13.2911 L 0.759 12.625 L 1.364 11.772 Q 4.98691 6.40271 6.409 0.085 Q 8.54371 -9.03985 14.449 -14.784 Q 20.278 -20.4539 27.654 -20.755 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node32
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node32_transform_base_group
            Translate { x: 17.973; y: 239.849}
            Matrix4x4 { matrix: _qt_node26.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node33
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node34_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 17.906 -3.191 L 17.906 -7.661 Q 17.4522 -6.83157 16.956 -6.076 Q 16.7699 -5.77728 16.605 -5.55 Q 15.1083 -3.30719 13.181 -1.425 Q 10.265 1.41143 6.96225 2.90268 Q 3.6595 4.39393 -0.03 4.54 L -15.486 5.145 L -17.906 5.242 L -17.906 7.661 L -16.696 7.6125 L -15.486 7.564 L 0.067 6.96 Q 8.37664 6.6248 14.869 0.305 L 15.2683 -0.0964081 L 15.6631 -0.506704 L 15.673 -0.517 Q 16.472 -1.37201 17.198 -2.278 L 17.3927 -2.52486 L 17.906 -3.191 " }
            }
        }
    }
    LayerItem {
        objectName: " Wire"
        id: _qt_node26
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node26_transform_base_group
            Matrix4x4 { matrix: _qt_node24.transformMatrix }
        }
    }
    LayerItem {
        objectName: " Cart"
        id: _qt_node24
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node24_transform_base_group
            TransformGroup {
                id: _qt_node24_transform_group_0
                Translate { x: -66.882; y: -123.755 }
                Scale { id: _qt_node24_transform_0_1}
            }
            Translate {
                x: _qt_node24_motion_animation.currentInterpolator.x
                y: _qt_node24_motion_animation.currentInterpolator.y
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node24_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node24_transform_animation
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
                                script:_qt_node24_transform_0_1.xScale = 0.1
                            }
                            ScriptAction {
                                script:_qt_node24_transform_0_1.yScale = 0.1
                            }
                        }
                        PauseAnimation { duration: 750 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node24_transform_0_1
                                property: "xScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node24_transform_0_1
                                property: "yScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 4250 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node24_transform_0_1
                                property: "xScale"
                                to: 0.1
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node24_transform_0_1
                                property: "yScale"
                                to: 0.1
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 2500 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node24_transform_base_group.deactivateOverride(_qt_node24_transform_group_0)
                    }
                }
            }
        }
        QtObject {
            id: _qt_node24_pathInterpolator_0
            property real x: 717.435
            property real y: 376.351
            property real angle: -180
        }
        PathInterpolator {
            id: _qt_node24_pathInterpolator_1
            path: Path { PathSvg { path: "M 717.435 376.351 C 700.768 376.351 599.935 376.351 617.435 376.351 " } }
        }
        PathInterpolator {
            id: _qt_node24_pathInterpolator_2
            path: Path { PathSvg { path: "M 617.435 376.351 C 634.935 376.351 788.268 376.351 822.435 376.351 " } }
        }
        PathInterpolator {
            id: _qt_node24_pathInterpolator_4
            path: Path { PathSvg { path: "M 822.435 376.351 C 804.935 376.351 734.935 376.351 717.435 376.351 " } }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node24_motion_animation.restart() } }
        SequentialAnimation {
            id: _qt_node24_motion_animation
            property var currentInterpolator: _qt_node24_pathInterpolator_0
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            ScriptAction {
                script: {
                    _qt_node24_motion_animation.currentInterpolator = _qt_node24_pathInterpolator_1
                }
            }
            PropertyAnimation {
                id: _qt_node24_motionAnimation_1
                duration: 750
                target: _qt_node24_pathInterpolator_1
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_00
            }
            ScriptAction {
                script: {
                    _qt_node24_motion_animation.currentInterpolator = _qt_node24_pathInterpolator_2
                }
            }
            PropertyAnimation {
                id: _qt_node24_motionAnimation_2
                duration: 250
                target: _qt_node24_pathInterpolator_2
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_00
            }
            PauseAnimation { duration: 4250 }
            ScriptAction {
                script: {
                    _qt_node24_motion_animation.currentInterpolator = _qt_node24_pathInterpolator_4
                }
            }
            PropertyAnimation {
                id: _qt_node24_motionAnimation_4
                duration: 250
                target: _qt_node24_pathInterpolator_4
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_00
            }
            PauseAnimation { duration: 2500 }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node35
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node35_transform_base_group
            Translate { x: 71.039; y: 47.671}
            Matrix4x4 { matrix: _qt_node36.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node37
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node38_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "#00000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 68.31 -47.671 L -68.311 -47.671 Q -69.4412 -47.671 -70.2401 -46.8716 Q -71.039 -46.0723 -71.039 -44.942 L -71.039 44.942 Q -71.039 46.0723 -70.2401 46.8716 Q -69.4412 47.671 -68.311 47.671 L 68.31 47.671 Q 69.4402 47.671 70.2396 46.8716 Q 71.039 46.0723 71.039 44.942 L 71.039 -44.942 Q 71.039 -46.0723 70.2396 -46.8716 Q 69.4402 -47.671 68.31 -47.671 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node39
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node39_transform_base_group
            Translate { x: 71.973; y: 15.886}
            Matrix4x4 { matrix: _qt_node36.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node40
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node41_path_stroke
                objectName: "svg_stroke_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "transparent"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic
                PathSvg { path: "M 71.036 0 L -71.036 0 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node42
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node42_transform_base_group
            TransformGroup {
                id: _qt_node42_transform_group_0
                Translate { x: 28.976; y: 0 }
            }
            Matrix4x4 { matrix: _qt_node43.transformMatrix }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node42_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node42_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node42.opacity = 1
                    }
                    PauseAnimation { duration: 3667 }
                    PropertyAnimation {
                        duration: 33
                        target: _qt_node42
                        property: "opacity"
                        to: 0
                        easing: _qt_node0.easing_02
                    }
                    PauseAnimation { duration: 4300 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node44
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node45_path_stroke
                objectName: "svg_stroke_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "transparent"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic
                PathSvg { path: "M 28.976 0 L -28.976 0 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node46
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node46_transform_base_group
            TransformGroup {
                id: _qt_node46_transform_group_0
                Translate { x: 28.976; y: 16.823 }
            }
            Matrix4x4 { matrix: _qt_node43.transformMatrix }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node46_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node46_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node46.opacity = 1
                    }
                    PauseAnimation { duration: 3833 }
                    PropertyAnimation {
                        duration: 34
                        target: _qt_node46
                        property: "opacity"
                        to: 0
                        easing: _qt_node0.easing_02
                    }
                    PauseAnimation { duration: 4133 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node47
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node48_path_stroke
                objectName: "svg_stroke_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "transparent"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic
                PathSvg { path: "M 28.976 0 L -28.976 0 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node49
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node49_transform_base_group
            TransformGroup {
                id: _qt_node49_transform_group_0
                Translate { x: 28.976; y: 33.646 }
            }
            Matrix4x4 { matrix: _qt_node43.transformMatrix }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node49_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node49_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node49.opacity = 1
                    }
                    PauseAnimation { duration: 3967 }
                    PropertyAnimation {
                        duration: 33
                        target: _qt_node49
                        property: "opacity"
                        to: 0
                        easing: _qt_node0.easing_02
                    }
                    PauseAnimation { duration: 4000 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node50
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node51_path_stroke
                objectName: "svg_stroke_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "transparent"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic
                PathSvg { path: "M 28.976 0 L -28.976 0 " }
            }
        }
    }
    LayerItem {
        objectName: " Text Top"
        id: _qt_node43
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node43_transform_base_group
            TransformGroup {
                id: _qt_node43_transform_group_0
                Translate { x: -28.976; y: -16.823 }
            }
            Translate {
                x: _qt_node43_motion_animation.currentInterpolator.x
                y: _qt_node43_motion_animation.currentInterpolator.y
            }
            Matrix4x4 { matrix: _qt_node36.transformMatrix }
        }
        QtObject {
            id: _qt_node43_pathInterpolator_0
            property real x: 91.603
            property real y: 51.704
            property real angle: -90
        }
        PathInterpolator {
            id: _qt_node43_pathInterpolator_2
            path: Path { PathSvg { path: "M 91.603 51.704 C 91.603 42.871 91.603 7.537 91.603 -1.296 " } }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node43_motion_animation.restart() } }
        SequentialAnimation {
            id: _qt_node43_motion_animation
            property var currentInterpolator: _qt_node43_pathInterpolator_0
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            PauseAnimation { duration: 3517 }
            ScriptAction {
                script: {
                    _qt_node43_motion_animation.currentInterpolator = _qt_node43_pathInterpolator_2
                }
            }
            PropertyAnimation {
                id: _qt_node43_motionAnimation_2
                duration: 483
                target: _qt_node43_pathInterpolator_2
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_02
            }
            PauseAnimation { duration: 4000 }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node52
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node52_transform_base_group
            TransformGroup {
                id: _qt_node52_transform_group_0
                Translate { x: 28.976; y: 0 }
            }
            Matrix4x4 { matrix: _qt_node53.transformMatrix }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node52_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node52_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node52.opacity = 0
                    }
                    PauseAnimation { duration: 4083 }
                    PropertyAnimation {
                        duration: 34
                        target: _qt_node52
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_02
                    }
                    PauseAnimation { duration: 3883 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node54
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node55_path_stroke
                objectName: "svg_stroke_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "transparent"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic
                PathSvg { path: "M 28.976 0 L -28.976 0 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node56
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node56_transform_base_group
            TransformGroup {
                id: _qt_node56_transform_group_0
                Translate { x: 28.976; y: 16.823 }
            }
            Matrix4x4 { matrix: _qt_node53.transformMatrix }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node56_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node56_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node56.opacity = 0
                    }
                    PauseAnimation { duration: 4167 }
                    PropertyAnimation {
                        duration: 33
                        target: _qt_node56
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_02
                    }
                    PauseAnimation { duration: 3800 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node57
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node58_path_stroke
                objectName: "svg_stroke_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "transparent"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic
                PathSvg { path: "M 28.976 0 L -28.976 0 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node59
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node59_transform_base_group
            TransformGroup {
                id: _qt_node59_transform_group_0
                Translate { x: 28.976; y: 33.646 }
            }
            Matrix4x4 { matrix: _qt_node53.transformMatrix }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node59_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node59_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node59.opacity = 0
                    }
                    PauseAnimation { duration: 4300 }
                    PropertyAnimation {
                        duration: 33
                        target: _qt_node59
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_02
                    }
                    PauseAnimation { duration: 3667 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node60
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node61_path_stroke
                objectName: "svg_stroke_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "transparent"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic
                PathSvg { path: "M 28.976 0 L -28.976 0 " }
            }
        }
    }
    LayerItem {
        objectName: " Text Bottom"
        id: _qt_node53
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node53_transform_base_group
            TransformGroup {
                id: _qt_node53_transform_group_0
                Translate { x: -28.976; y: -16.823 }
            }
            Translate {
                x: _qt_node53_motion_animation.currentInterpolator.x
                y: _qt_node53_motion_animation.currentInterpolator.y
            }
            Matrix4x4 { matrix: _qt_node36.transformMatrix }
        }
        QtObject {
            id: _qt_node53_pathInterpolator_0
            property real x: 91.281
            property real y: 122.169
            property real angle: -90
        }
        PathInterpolator {
            id: _qt_node53_pathInterpolator_2
            path: Path { PathSvg { path: "M 91.281 122.169 C 91.281 110.836 91.281 65.502 91.281 54.169 " } }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node53_motion_animation.restart() } }
        SequentialAnimation {
            id: _qt_node53_motion_animation
            property var currentInterpolator: _qt_node53_pathInterpolator_0
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            PauseAnimation { duration: 4000 }
            ScriptAction {
                script: {
                    _qt_node53_motion_animation.currentInterpolator = _qt_node53_pathInterpolator_2
                }
            }
            PropertyAnimation {
                id: _qt_node53_motionAnimation_2
                duration: 500
                target: _qt_node53_pathInterpolator_2
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_02
            }
            PauseAnimation { duration: 3500 }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node62
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node62_transform_base_group
            Translate { x: 31.784; y: 50.462}
            Matrix4x4 { matrix: _qt_node36.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node63
            transformOrigin: Item.TopLeft
            transform: TransformGroup {
                id: _qt_node63_transform_base_group
                Matrix4x4 { matrix: PlanarTransform.fromAffineMatrix(0.22971, -0.973259, 0.973259, 0.22971, 0, 0)}
            }
            ShapePath {
                id: _qt_node64_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "#ffa77ffa"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 0 16.823 Q 6.96825 16.823 11.8956 11.8956 Q 16.823 6.96825 16.823 0 Q 16.823 -6.96825 11.8956 -11.8956 Q 6.96825 -16.823 0 -16.823 Q -6.96825 -16.823 -11.8956 -11.8956 Q -16.823 -6.96825 -16.823 0 Q -16.823 6.96825 -11.8956 11.8956 Q -6.96825 16.823 0 16.823 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node65
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node65_transform_base_group
            Translate { x: 33.661; y: 50.547}
            Matrix4x4 { matrix: _qt_node36.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node66
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node67_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "#ff000000"
                strokeWidth: 2
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.RoundJoin
                miterLimit: 0
                fillColor: "#00000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -4.83 -7.277 L -4.83 7.277 L 0 3.6385 L 3.32063 1.13703 L 4.83 0 L -4.83 -7.277 " }
            }
        }
    }
    LayerItem {
        objectName: " Card Only"
        id: _qt_node36
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node36_transform_base_group
            TransformGroup {
                id: _qt_node36_transform_group_0
                Translate { x: -71.504; y: -60.996 }
            }
            Translate {
                x: _qt_node36_motion_animation.currentInterpolator.x
                y: _qt_node36_motion_animation.currentInterpolator.y
            }
            Matrix4x4 { matrix: _qt_node68.transformMatrix }
        }
        QtObject {
            id: _qt_node36_pathInterpolator_0
            property real x: 71.504
            property real y: 60.996
            property real angle: -213.703
        }
        PathInterpolator {
            id: _qt_node36_pathInterpolator_2
            path: Path { PathSvg { path: "M 71.504 60.996 C 70.504 61.663 65.504 64.829 65.504 64.996 " } }
        }
        PathInterpolator {
            id: _qt_node36_pathInterpolator_3
            path: Path { PathSvg { path: "M 65.504 64.996 C 65.504 65.163 70.504 62.496 71.504 61.996 " } }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node36_motion_animation.restart() } }
        SequentialAnimation {
            id: _qt_node36_motion_animation
            property var currentInterpolator: _qt_node36_pathInterpolator_0
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            PauseAnimation { duration: 983 }
            ScriptAction {
                script: {
                    _qt_node36_motion_animation.currentInterpolator = _qt_node36_pathInterpolator_2
                }
            }
            PropertyAnimation {
                id: _qt_node36_motionAnimation_2
                duration: 2017
                target: _qt_node36_pathInterpolator_2
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_01
            }
            ScriptAction {
                script: {
                    _qt_node36_motion_animation.currentInterpolator = _qt_node36_pathInterpolator_3
                }
            }
            PropertyAnimation {
                id: _qt_node36_motionAnimation_3
                duration: 2000
                target: _qt_node36_pathInterpolator_3
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_01
            }
            PauseAnimation { duration: 3000 }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node69
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node69_transform_base_group
            Translate { x: 112.433; y: 122.267}
            Matrix4x4 { matrix: _qt_node68.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node70
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node71_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 55.256 25.086 L 55.256 22.182 L 54.1121 21.4885 L 54.047 21.45 L 53.4442 21.1047 L 52.837 20.773 Q 48.4504 18.4286 43.666 17.053 Q 38.1978 15.4628 31.5042 14.8009 Q 26.0748 14.264 18.798 14.264 L 8.26761 14.4301 L -2.326 14.597 Q -9.6732 14.66 -16.995 13.877 Q -21.6068 13.3443 -25.5826 12.1484 Q -29.3776 11.0069 -32.632 9.249 Q -41.3515 4.56775 -46.455 -5.039 Q -50.5374 -12.7237 -52.879 -25.086 L -54.0675 -24.859 L -55.256 -24.632 Q -53.6908 -16.4037 -51.4852 -10.424 Q -48.8105 -3.17216 -44.797 2.014 Q -40.2954 7.87652 -33.788 11.379 Q -26.7534 15.1827 -17.28 16.278 Q -9.81169 17.0904 -2.332 17.035 L 8.37607 16.8668 L 18.792 16.702 Q 25.9446 16.702 31.2647 17.2267 Q 37.7541 17.8668 42.989 19.394 Q 48.1663 20.8749 52.837 23.562 L 53.682 24.0749 L 54.047 24.3 Q 54.5975 24.6371 55.256 25.086 " }
            }
        }
    }
    LayerItem {
        objectName: " Card"
        id: _qt_node68
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node68_transform_base_group
            TransformGroup {
                id: _qt_node68_transform_group_0
                Translate { x: -83.845; y: -73.676 }
                Scale { id: _qt_node68_transform_0_1}
            }
            Translate {
                x: _qt_node68_motion_animation.currentInterpolator.x
                y: _qt_node68_motion_animation.currentInterpolator.y
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node68_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node68_transform_animation
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
                                script:_qt_node68_transform_0_1.xScale = 0.1
                            }
                            ScriptAction {
                                script:_qt_node68_transform_0_1.yScale = 0.1
                            }
                        }
                        PauseAnimation { duration: 750 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node68_transform_0_1
                                property: "xScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node68_transform_0_1
                                property: "yScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 4250 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node68_transform_0_1
                                property: "xScale"
                                to: 0.1
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node68_transform_0_1
                                property: "yScale"
                                to: 0.1
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 2500 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node68_transform_base_group.deactivateOverride(_qt_node68_transform_group_0)
                    }
                }
            }
        }
        QtObject {
            id: _qt_node68_pathInterpolator_0
            property real x: 660.539
            property real y: 355.33
            property real angle: -145.331
        }
        PathInterpolator {
            id: _qt_node68_pathInterpolator_2
            path: Path { PathSvg { path: "M 660.539 355.33 C 622.706 329.163 471.372 224.497 433.539 198.33 " } }
        }
        PathInterpolator {
            id: _qt_node68_pathInterpolator_4
            path: Path { PathSvg { path: "M 433.539 198.33 C 471.372 224.497 622.706 329.163 660.539 355.33 " } }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node68_motion_animation.restart() } }
        SequentialAnimation {
            id: _qt_node68_motion_animation
            property var currentInterpolator: _qt_node68_pathInterpolator_0
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            PauseAnimation { duration: 750 }
            ScriptAction {
                script: {
                    _qt_node68_motion_animation.currentInterpolator = _qt_node68_pathInterpolator_2
                }
            }
            PropertyAnimation {
                id: _qt_node68_motionAnimation_2
                duration: 250
                target: _qt_node68_pathInterpolator_2
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_00
            }
            PauseAnimation { duration: 4250 }
            ScriptAction {
                script: {
                    _qt_node68_motion_animation.currentInterpolator = _qt_node68_pathInterpolator_4
                }
            }
            PropertyAnimation {
                id: _qt_node68_motionAnimation_4
                duration: 250
                target: _qt_node68_pathInterpolator_4
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_00
            }
            PauseAnimation { duration: 2500 }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node68_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node68_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node68.opacity = 0
                    }
                    PauseAnimation { duration: 750 }
                    PropertyAnimation {
                        duration: 250
                        target: _qt_node68
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 4250 }
                    PropertyAnimation {
                        duration: 250
                        target: _qt_node68
                        property: "opacity"
                        to: 0
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 2500 }
                }
            }
        }
    }
    LayerItem {
        objectName: "Phone"
        id: _qt_node72
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node72_transform_base_group
            Translate { x: 636.5; y: 352}
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Phone"
            id: _qt_node73
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node74_path_fill_stroke
                objectName: "svg_path:Rectangle Path 1"
                strokeColor: "#ff000000"
                strokeWidth: 4
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -121.5 -150 Q -121.5 -159.113 -115.056 -165.556 Q -108.613 -172 -99.5 -172 L 99.5 -172 Q 108.613 -172 115.056 -165.556 Q 121.5 -159.113 121.5 -150 L 121.5 150 Q 121.5 159.113 115.056 165.556 Q 108.613 172 99.5 172 L -99.5 172 Q -108.613 172 -115.056 165.556 Q -121.5 159.113 -121.5 150 L -121.5 -150 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node75
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node75_transform_base_group
            Translate { x: 816.462; y: 226.51}
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node76
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node77_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "#00000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 0 3.944 Q 1.6335 3.944 2.78875 2.78875 Q 3.944 1.6335 3.944 0 Q 3.944 -1.6335 2.78875 -2.78875 Q 1.6335 -3.944 0 -3.944 Q -1.6335 -3.944 -2.78875 -2.78875 Q -3.944 -1.6335 -3.944 0 Q -3.944 1.6335 -2.78875 2.78875 Q -1.6335 3.944 0 3.944 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node78
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node78_transform_base_group
            Translate { x: 467.423; y: 97.057}
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node79
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node80_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "#00000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 0 3.944 Q 1.6335 3.944 2.78875 2.78875 Q 3.944 1.6335 3.944 0 Q 3.944 -1.6335 2.78875 -2.78875 Q 1.6335 -3.944 0 -3.944 Q -1.6335 -3.944 -2.78875 -2.78875 Q -3.944 -1.6335 -3.944 0 Q -3.944 1.6335 -2.78875 2.78875 Q -1.6335 3.944 0 3.944 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node81
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node81_transform_base_group
            Translate { x: 405.117; y: 454.564}
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node82
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node83_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "#00000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 0 3.944 Q 1.6335 3.944 2.78875 2.78875 Q 3.944 1.6335 3.944 0 Q 3.944 -1.6335 2.78875 -2.78875 Q 1.6335 -3.944 0 -3.944 Q -1.6335 -3.944 -2.78875 -2.78875 Q -3.944 -1.6335 -3.944 0 Q -3.944 1.6335 -2.78875 2.78875 Q -1.6335 3.944 0 3.944 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node84
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node84_transform_base_group
            Translate { x: 440.172; y: 286.454}
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node85
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node86_path_stroke
                objectName: "svg_stroke_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "transparent"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic
                PathSvg { path: "M 0 3.875 L 0 -3.875 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node87
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node87_transform_base_group
            Translate { x: 440.169; y: 286.451}
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node88
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node89_path_stroke
                objectName: "svg_stroke_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "transparent"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic
                PathSvg { path: "M 3.875 0 L -3.875 0 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node90
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node90_transform_base_group
            Translate { x: 885.392; y: 377.191}
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node91
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node92_path_stroke
                objectName: "svg_stroke_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "transparent"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic
                PathSvg { path: "M 0 3.875 L 0 -3.875 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node93
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node93_transform_base_group
            Translate { x: 885.389; y: 377.189}
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node94
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node95_path_stroke
                objectName: "svg_stroke_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "transparent"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic
                PathSvg { path: "M 3.874 0 L -3.874 0 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node96
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node96_transform_base_group
            Translate { x: 516.428; y: 137.444}
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node97
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node98_path_stroke
                objectName: "svg_stroke_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "transparent"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic
                PathSvg { path: "M 0 4.177 L 0 -4.177 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node99
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node99_transform_base_group
            Translate { x: 516.425; y: 137.447}
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node100
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node101_path_stroke
                objectName: "svg_stroke_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "transparent"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic
                PathSvg { path: "M 4.177 0 L -4.177 0 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node102
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node102_transform_base_group
            Translate { x: 791.733; y: 486.446}
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node103
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node104_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -0.302 -1.667 L 0.00675 -2.231 L 0.302 -2.804 L 0.302 1.951 L -0.250713 2.73332 L -0.302 2.804 L -0.302 -1.667 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node105
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node105_transform_base_group
            Translate { x: 635.845; y: 288.6}
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node106
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node107_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 48.115 -18.065 Q 53.5574 -14.4531 56.1 -8.434 Q 57.8117 -4.40772 57.842 -0.032 Q 57.8139 8.94698 51.466 15.298 Q 45.1188 21.6483 36.138 21.678 Q 30.3496 21.6902 25.328 18.817 L 25.298 18.817 L 14.409 29.706 Q 14.7185 30.2592 15.0208 30.8746 Q 15.3076 31.4588 15.583 32.089 Q 17.2919 36.128 17.301 40.516 Q 17.2903 46.5315 14.189 51.683 Q 11.0873 56.8352 5.777 59.66 Q 0.467834 62.4842 -5.539 62.177 Q -11.543 61.8699 -16.538 58.518 Q -21.8692 54.9046 -24.402 48.978 Q -26.1306 44.9188 -26.12 40.51 Q -26.1261 36.1918 -24.475 32.202 Q -22.8248 28.2142 -19.768 25.162 Q -16.7139 22.1124 -12.72 20.467 Q -8.72564 18.8214 -4.41 18.835 Q 1.17377 18.8182 6.067 21.515 L 17.113 10.469 Q 16.5622 9.48896 16.121 8.455 Q 15.574 7.19766 15.22 5.866 L -15.274 5.866 Q -16.174 9.2457 -18.166 12.127 Q -21.1889 16.6104 -25.96 19.152 Q -30.7315 21.6939 -36.138 21.703 Q -42.7719 21.7199 -48.266 18.007 Q -53.6084 14.3959 -56.13 8.461 Q -57.8556 4.40369 -57.842 -0.008 Q -57.845 -4.32378 -56.194 -8.312 Q -54.5438 -12.2982 -51.489 -15.35 Q -48.4384 -18.3976 -44.444 -20.046 Q -40.4576 -21.6911 -36.138 -21.682 Q -30.5421 -21.7033 -25.636 -19.014 L -20.1195 -24.531 L -14.603 -30.048 Q -15.155 -31.0248 -15.601 -32.063 Q -17.3266 -36.1203 -17.313 -40.531 Q -17.316 -44.8478 -15.665 -48.836 Q -14.0155 -52.8205 -10.959 -55.874 Q -7.90937 -58.9206 -3.915 -60.569 Q 0.0737798 -62.2151 4.392 -62.206 Q 11.0498 -62.229 16.569 -58.504 Q 21.0516 -55.4805 23.592 -50.709 Q 26.1329 -45.9364 26.139 -40.531 Q 26.139 -33.9864 22.516 -28.532 Q 18.8929 -23.0775 12.861 -20.539 Q 8.80333 -18.8028 4.392 -18.821 Q -1.40778 -18.7966 -6.442 -21.67 L -17.331 -10.782 Q -16.6204 -9.66658 -16.121 -8.434 Q -15.5737 -7.18047 -15.25 -5.845 L 15.274 -5.845 Q 16.1817 -9.22666 18.13 -12.137 Q 21.1511 -16.6344 25.936 -19.17 Q 30.7223 -21.7064 36.138 -21.682 Q 42.6719 -21.6774 48.115 -18.065 M -1.173 -48.813 Q -3.63604 -47.1516 -4.803 -44.421 Q -5.78064 -42.0881 -5.546 -39.568 Q -5.3113 -37.0473 -3.92 -34.936 Q -2.52382 -32.8623 -0.316 -31.686 Q 1.88932 -30.511 4.392 -30.508 Q 7.47007 -30.4896 10.011 -32.226 Q 12.4815 -33.8868 13.641 -36.624 Q 14.4362 -38.4991 14.421 -40.538 Q 14.4287 -43.5529 12.761 -46.069 Q 11.0951 -48.5825 8.312 -49.75 Q 5.97194 -50.7318 3.45 -50.482 Q 0.926737 -50.2321 -1.173 -48.813 M -30.196 8.079 Q -27.9725 6.44496 -26.901 3.9 L -26.901 3.906 Q -26.0944 2.02857 -26.102 -0.008 Q -26.0943 -3.02393 -27.762 -5.54 Q -29.4276 -8.05288 -32.212 -9.221 Q -34.5499 -10.2019 -37.073 -9.952 Q -39.5974 -9.702 -41.697 -8.283 Q -44.1548 -6.61584 -45.327 -3.892 Q -46.3027 -1.56152 -46.068 0.958 Q -45.8334 3.47568 -44.443 5.588 Q -42.9063 7.88082 -40.411 9.062 Q -37.9154 10.2433 -35.168 9.979 Q -32.4219 9.71477 -30.196 8.079 M 1.174 48.827 Q 3.63356 47.1633 4.803 44.429 L 4.797 44.426 L 4.791 44.423 Q 5.76559 42.0886 5.525 39.57 Q 5.28442 37.0515 3.884 34.944 Q 2.21008 32.4768 -0.538 31.315 Q -3.59564 30.0325 -6.817 30.829 Q -10.0358 31.6249 -12.145 34.187 Q -14.2538 36.7486 -14.417 40.061 Q -14.5802 43.3728 -12.733 46.129 Q -11.046 48.5897 -8.305 49.759 Q -6.44209 50.5375 -4.422 50.539 Q -1.36025 50.5405 1.174 48.827 M 41.703 8.298 Q 44.164 6.63336 45.333 3.9 L 45.327 3.906 Q 46.131 2.03303 46.131 -0.008 Q 46.1325 -3.02597 44.465 -5.538 Q 42.7975 -8.04999 40.015 -9.221 Q 37.6771 -10.2019 35.154 -9.952 Q 32.6296 -9.702 30.53 -8.283 Q 28.0678 -6.6221 26.901 -3.892 Q 25.9203 -1.56049 26.158 0.96 Q 26.3956 3.47899 27.796 5.588 Q 29.1918 7.66336 31.399 8.837 Q 33.6079 10.0115 36.108 10.01 Q 39.1687 10.0115 41.703 8.298 " }
            }
        }
    }
    LayerItem {
        objectName: "BG"
        id: _qt_node108
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node108_transform_base_group
            Translate { x: 58.98; y: 26.012}
            Matrix4x4 { matrix: _qt_node109.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "BG"
            id: _qt_node110
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node111_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "#ff000000"
                strokeWidth: 4
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "#ffc4c4c4"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 32.968 -26.012 L -32.968 -26.012 Q -43.7425 -26.012 -51.3613 -18.3932 Q -58.98 -10.7745 -58.98 0 Q -58.98 10.7745 -51.3613 18.3932 Q -43.7425 26.012 -32.968 26.012 L 32.968 26.012 Q 43.7425 26.012 51.3613 18.3932 Q 58.98 10.7745 58.98 0 Q 58.98 -10.7745 51.3613 -18.3932 Q 43.7425 -26.012 32.968 -26.012 " }
            }
            Connections { target: _qt_node0.animations; function onRestart() {_qt_node111_path_fill_stroke_fillColor_animation.restart() } }
            ParallelAnimation {
                id: _qt_node111_path_fill_stroke_fillColor_animation
                loops: _qt_node0.animations.loops
                paused: _qt_node0.animations.paused
                running: true
                onLoopsChanged: { if (running) { restart() } }
                SequentialAnimation {
                    SequentialAnimation {
                        loops: 1
                        ScriptAction {
                            script:_qt_node111_path_fill_stroke.fillColor = "#c4c4c4"
                        }
                        PauseAnimation { duration: 750 }
                        ColorAnimation {
                            duration: 250
                            target: _qt_node111_path_fill_stroke
                            property: "fillColor"
                            to: "#49ffba"
                            easing: _qt_node0.easing_00
                        }
                        PauseAnimation { duration: 4250 }
                        ColorAnimation {
                            duration: 250
                            target: _qt_node111_path_fill_stroke
                            property: "fillColor"
                            to: "#c4c4c4"
                            easing: _qt_node0.easing_00
                        }
                        PauseAnimation { duration: 2500 }
                    }
                }
            }
        }
    }
    LayerItem {
        objectName: "Switch"
        id: _qt_node112
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node112_transform_base_group
            TransformGroup {
                id: _qt_node112_transform_group_0
            }
            Translate {
                x: _qt_node112_motion_animation.currentInterpolator.x
                y: _qt_node112_motion_animation.currentInterpolator.y
            }
            Matrix4x4 { matrix: _qt_node109.transformMatrix }
        }
        QtObject {
            id: _qt_node112_pathInterpolator_0
            property real x: 25.709
            property real y: 25.709
            property real angle: 0
        }
        PathInterpolator {
            id: _qt_node112_pathInterpolator_2
            path: Path { PathSvg { path: "M 25.709 25.709 C 36.876 25.709 81.542 25.709 92.709 25.709 " } }
        }
        PathInterpolator {
            id: _qt_node112_pathInterpolator_4
            path: Path { PathSvg { path: "M 92.709 25.709 C 81.541 25.709 36.868 25.709 25.7 25.709 " } }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node112_motion_animation.restart() } }
        SequentialAnimation {
            id: _qt_node112_motion_animation
            property var currentInterpolator: _qt_node112_pathInterpolator_0
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            PauseAnimation { duration: 500 }
            ScriptAction {
                script: {
                    _qt_node112_motion_animation.currentInterpolator = _qt_node112_pathInterpolator_2
                }
            }
            PropertyAnimation {
                id: _qt_node112_motionAnimation_2
                duration: 517
                target: _qt_node112_pathInterpolator_2
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_03
            }
            PauseAnimation { duration: 3983 }
            ScriptAction {
                script: {
                    _qt_node112_motion_animation.currentInterpolator = _qt_node112_pathInterpolator_4
                }
            }
            PropertyAnimation {
                id: _qt_node112_motionAnimation_4
                duration: 500
                target: _qt_node112_pathInterpolator_4
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_03
            }
            PauseAnimation { duration: 2500 }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Switch"
            id: _qt_node113
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node114_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "#ff000000"
                strokeWidth: 4
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 0 25.709 Q 10.6493 25.709 18.1791 18.1791 Q 25.709 10.6493 25.709 0 Q 25.709 -10.6493 18.1791 -18.1791 Q 10.6493 -25.709 0 -25.709 Q -10.6493 -25.709 -18.1791 -18.1791 Q -25.709 -10.6493 -25.709 0 Q -25.709 10.6493 -18.1791 18.1791 Q -10.6493 25.709 0 25.709 " }
            }
        }
    }
    LayerItem {
        objectName: " Toggle"
        id: _qt_node109
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node109_transform_base_group
            Translate { x: 577.022; y: 411.439}
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node115
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node115_transform_base_group
            Translate { x: 138.49; y: 52.41}
            Matrix4x4 { matrix: _qt_node116.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node117
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node118_path_stroke
                objectName: "svg_stroke_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "transparent"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic
                PathSvg { path: "M 71.266 0 L -71.266 0 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node119
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node119_transform_base_group
            Translate { x: 71.266; y: 67.389}
            Matrix4x4 { matrix: _qt_node116.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node120
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node121_path_stroke
                objectName: "svg_stroke_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "transparent"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic
                PathSvg { path: "M 71.266 0 L -71.266 0 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node122
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node122_transform_base_group
            Translate { x: 148.369; y: 20.483}
            Matrix4x4 { matrix: _qt_node116.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node123
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node124_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffa883f8"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -63.958 20.483 L 63.958 20.483 L 63.958 20.477 Q 63.9565 14.8383 60.873 10.122 Q 57.7882 5.40362 52.626 3.142 Q 47.4629 0.879984 41.903 1.812 Q 36.3433 2.74397 32.2 6.564 Q 28.9434 -4.35886 20.197 -11.67 Q 11.4522 -18.9798 0.123 -20.248 Q -11.2048 -21.516 -21.35 -16.32 Q -31.4962 -11.1235 -37.088 -1.191 L -38.092 -1.191 Q -47.6438 -1.18945 -54.964 4.945 Q -62.2848 11.08 -63.958 20.483 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node125
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node125_transform_base_group
            Translate { x: 106.895; y: 30.113}
            Matrix4x4 { matrix: _qt_node116.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node126
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node127_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "#ffffffff"
                strokeWidth: 4
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.RoundJoin
                miterLimit: 0
                fillColor: "#00000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -69.662 22.298 L 69.662 22.298 L 69.662 22.292 Q 69.662 16.1501 66.304 11.012 Q 62.945 5.87238 57.322 3.408 Q 51.6993 0.943777 45.643 1.957 Q 39.5887 2.96989 35.073 7.133 Q 31.5189 -4.7575 21.994 -12.711 Q 12.4669 -20.6663 0.135 -22.043 Q -12.2006 -23.4201 -23.243 -17.764 Q -34.2878 -12.1067 -40.378 -1.294 L -41.479 -1.294 Q -51.8801 -1.29555 -59.857 5.381 Q -67.8337 12.0574 -69.662 22.298 " }
            }
        }
    }
    LayerItem {
        objectName: " Cloud"
        id: _qt_node116
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node116_transform_base_group
            TransformGroup {
                id: _qt_node116_transform_group_0
                Translate { x: -106.163; y: -33.694 }
            }
            Translate {
                x: _qt_node116_motion_animation.currentInterpolator.x
                y: _qt_node116_motion_animation.currentInterpolator.y
            }
        }
        QtObject {
            id: _qt_node116_pathInterpolator_0
            property real x: 437.375
            property real y: 594.193
            property real angle: -225
        }
        PathInterpolator {
            id: _qt_node116_pathInterpolator_2
            path: Path { PathSvg { path: "M 437.375 594.193 C 432.375 599.193 412.375 619.193 407.375 624.193 " } }
        }
        PathInterpolator {
            id: _qt_node116_pathInterpolator_4
            path: Path { PathSvg { path: "M 407.375 624.193 C 412.375 619.193 432.375 599.193 437.375 594.193 " } }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node116_motion_animation.restart() } }
        SequentialAnimation {
            id: _qt_node116_motion_animation
            property var currentInterpolator: _qt_node116_pathInterpolator_0
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            PauseAnimation { duration: 750 }
            ScriptAction {
                script: {
                    _qt_node116_motion_animation.currentInterpolator = _qt_node116_pathInterpolator_2
                }
            }
            PropertyAnimation {
                id: _qt_node116_motionAnimation_2
                duration: 250
                target: _qt_node116_pathInterpolator_2
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_04
            }
            PauseAnimation { duration: 4233 }
            ScriptAction {
                script: {
                    _qt_node116_motion_animation.currentInterpolator = _qt_node116_pathInterpolator_4
                }
            }
            PropertyAnimation {
                id: _qt_node116_motionAnimation_4
                duration: 267
                target: _qt_node116_pathInterpolator_4
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_04
            }
            PauseAnimation { duration: 2500 }
        }
    }
    LayerItem {
        objectName: "Clould Medium"
        id: _qt_node128
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node128_transform_base_group
            Translate { x: 59; y: 18.5}
            Matrix4x4 { matrix: _qt_node129.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Clould Medium"
            id: _qt_node130
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node131_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff6a3de6"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -59 18.5 L 59 18.5 Q 59 11.4218 53.8549 6.39888 Q 48.7017 1.368 41.419 1.368 Q 34.617 1.368 29.629 5.93 Q 26.4086 -4.7717 17.2706 -11.5688 Q 7.95221 -18.5 -3.775 -18.5 Q -13.4016 -18.5 -21.6149 -13.6974 Q -29.6073 -9.02388 -34.18 -1.064 L -35.11 -1.064 Q -44.0468 -1.064 -50.8166 4.53638 Q -57.516 10.0785 -59 18.5 " }
            }
        }
    }
    LayerItem {
        objectName: "Cloud Small"
        id: _qt_node132
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node132_transform_base_group
            Translate { x: 146.5; y: 55}
            Matrix4x4 { matrix: _qt_node129.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Cloud Small"
            id: _qt_node133
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node134_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff6a3de6"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -36.5 12 L 36.5 12 Q 36.5 7.48874 33.3424 4.2295 Q 30.1929 0.978646 25.777 0.899 Q 23.7426 0.899 21.8071 1.70512 Q 19.9698 2.47038 18.456 3.859 Q 16.4217 -3.12025 10.7618 -7.51975 Q 4.9979 -12 -2.268 -12 Q -8.27505 -12 -13.3649 -8.88113 Q -18.3015 -5.85613 -21.137 -0.687 L -21.756 -0.687 Q -27.2113 -0.687 -31.4095 2.96062 Q -35.566 6.57201 -36.5 12 " }
            }
        }
    }
    LayerItem {
        objectName: " Double Cloud"
        id: _qt_node129
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node129_transform_base_group
            TransformGroup {
                id: _qt_node129_transform_group_0
                Translate { x: -91.5; y: -33.5 }
                Translate { id: _qt_node129_transform_0_1 }
                Translate { id: _qt_node129_transform_0_0 }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node129_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node129_transform_animation
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
                                script:_qt_node129_transform_0_0.x = 805.5
                            }
                            ScriptAction {
                                script:_qt_node129_transform_0_0.y = 0
                            }
                        }
                        PauseAnimation { duration: 750 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node129_transform_0_0
                                property: "x"
                                to: 835.5
                                easing: _qt_node0.easing_04
                            }
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node129_transform_0_0
                                property: "y"
                                to: 0
                                easing: _qt_node0.easing_04
                            }
                        }
                        PauseAnimation { duration: 4250 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node129_transform_0_0
                                property: "x"
                                to: 805.5
                                easing: _qt_node0.easing_04
                            }
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node129_transform_0_0
                                property: "y"
                                to: 0
                                easing: _qt_node0.easing_04
                            }
                        }
                        PauseAnimation { duration: 2500 }
                    }
                    SequentialAnimation {
                        ParallelAnimation {
                            ScriptAction {
                                script:_qt_node129_transform_0_1.x = 0
                            }
                            ScriptAction {
                                script:_qt_node129_transform_0_1.y = 107.5
                            }
                        }
                        PauseAnimation { duration: 750 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node129_transform_0_1
                                property: "x"
                                to: 0
                                easing: _qt_node0.easing_04
                            }
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node129_transform_0_1
                                property: "y"
                                to: 77.5
                                easing: _qt_node0.easing_04
                            }
                        }
                        PauseAnimation { duration: 4250 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node129_transform_0_1
                                property: "x"
                                to: 0
                                easing: _qt_node0.easing_04
                            }
                            PropertyAnimation {
                                duration: 250
                                target: _qt_node129_transform_0_1
                                property: "y"
                                to: 107.5
                                easing: _qt_node0.easing_04
                            }
                        }
                        PauseAnimation { duration: 2500 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node129_transform_base_group.deactivateOverride(_qt_node129_transform_group_0)
                    }
                }
            }
        }
    }
    LayerItem {
        objectName: "Ring 5"
        id: _qt_node135
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node135_transform_base_group
            TransformGroup {
                id: _qt_node135_transform_group_0
                Scale { id: _qt_node135_transform_0_2}
                Translate { x: 636.5; y: 288.5 }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node135_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node135_transform_animation
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
                                script:_qt_node135_transform_0_2.xScale = 0.8
                            }
                            ScriptAction {
                                script:_qt_node135_transform_0_2.yScale = 0.8
                            }
                        }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 2483
                                target: _qt_node135_transform_0_2
                                property: "xScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 2483
                                target: _qt_node135_transform_0_2
                                property: "yScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                        }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 767
                                target: _qt_node135_transform_0_2
                                property: "xScale"
                                to: 1.25
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 767
                                target: _qt_node135_transform_0_2
                                property: "yScale"
                                to: 1.25
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 4750 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node135_transform_base_group.deactivateOverride(_qt_node135_transform_group_0)
                    }
                }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node135_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node135_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node135.opacity = 0
                    }
                    PauseAnimation { duration: 2483 }
                    PropertyAnimation {
                        duration: 767
                        target: _qt_node135
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 4750 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Ring 5"
            id: _qt_node136
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node137_path_fill_stroke
                objectName: "svg_path:Ellipse Path 1"
                strokeColor: "#ff9f9f9f"
                strokeWidth: 5
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "#00000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 0 -73.5 Q 30.4447 -73.5 51.9723 -51.9723 Q 73.5 -30.4447 73.5 0 Q 73.5 30.4447 51.9723 51.9723 Q 30.4447 73.5 0 73.5 Q -30.4447 73.5 -51.9723 51.9723 Q -73.5 30.4447 -73.5 0 Q -73.5 -30.4447 -51.9723 -51.9723 Q -30.4447 -73.5 0 -73.5 " }
            }
        }
    }
    LayerItem {
        objectName: "Ring 4"
        id: _qt_node138
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node138_transform_base_group
            TransformGroup {
                id: _qt_node138_transform_group_0
                Scale { id: _qt_node138_transform_0_2}
                Translate { x: 636.5; y: 288.5 }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node138_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node138_transform_animation
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
                                script:_qt_node138_transform_0_2.xScale = 0.8
                            }
                            ScriptAction {
                                script:_qt_node138_transform_0_2.yScale = 0.8
                            }
                        }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 4000
                                target: _qt_node138_transform_0_2
                                property: "xScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 4000
                                target: _qt_node138_transform_0_2
                                property: "yScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                        }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 750
                                target: _qt_node138_transform_0_2
                                property: "xScale"
                                to: 1.25
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 750
                                target: _qt_node138_transform_0_2
                                property: "yScale"
                                to: 1.25
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 3250 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node138_transform_base_group.deactivateOverride(_qt_node138_transform_group_0)
                    }
                }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node138_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node138_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node138.opacity = 0
                    }
                    PauseAnimation { duration: 4000 }
                    PropertyAnimation {
                        duration: 750
                        target: _qt_node138
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 3250 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Ring 4"
            id: _qt_node139
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node140_path_fill_stroke
                objectName: "svg_path:Ellipse Path 1"
                strokeColor: "#ff9f9f9f"
                strokeWidth: 5
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "#00000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 0 -73.5 Q 30.4447 -73.5 51.9723 -51.9723 Q 73.5 -30.4447 73.5 0 Q 73.5 30.4447 51.9723 51.9723 Q 30.4447 73.5 0 73.5 Q -30.4447 73.5 -51.9723 51.9723 Q -73.5 30.4447 -73.5 0 Q -73.5 -30.4447 -51.9723 -51.9723 Q -30.4447 -73.5 0 -73.5 " }
            }
        }
    }
    LayerItem {
        objectName: "Ring 1"
        id: _qt_node141
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node141_transform_base_group
            TransformGroup {
                id: _qt_node141_transform_group_0
                Scale { id: _qt_node141_transform_0_2}
                Translate { x: 636.5; y: 288.5 }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node141_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node141_transform_animation
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
                                script:_qt_node141_transform_0_2.xScale = 0.8
                            }
                            ScriptAction {
                                script:_qt_node141_transform_0_2.yScale = 0.8
                            }
                        }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 1000
                                target: _qt_node141_transform_0_2
                                property: "xScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 1000
                                target: _qt_node141_transform_0_2
                                property: "yScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                        }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 750
                                target: _qt_node141_transform_0_2
                                property: "xScale"
                                to: 1.25
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 750
                                target: _qt_node141_transform_0_2
                                property: "yScale"
                                to: 1.25
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 6250 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node141_transform_base_group.deactivateOverride(_qt_node141_transform_group_0)
                    }
                }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node141_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node141_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node141.opacity = 0
                    }
                    PauseAnimation { duration: 1000 }
                    PropertyAnimation {
                        duration: 750
                        target: _qt_node141
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 6250 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Ring 1"
            id: _qt_node142
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node143_path_fill_stroke
                objectName: "svg_path:Ellipse Path 1"
                strokeColor: "#ff9f9f9f"
                strokeWidth: 5
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.MiterJoin
                miterLimit: 4
                fillColor: "#00000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 0 -73.5 Q 30.4447 -73.5 51.9723 -51.9723 Q 73.5 -30.4447 73.5 0 Q 73.5 30.4447 51.9723 51.9723 Q 30.4447 73.5 0 73.5 Q -30.4447 73.5 -51.9723 51.9723 Q -73.5 30.4447 -73.5 0 Q -73.5 -30.4447 -51.9723 -51.9723 Q -30.4447 -73.5 0 -73.5 " }
            }
        }
    }
    readonly property easingCurve easing_02: ({ type: Easing.BezierSpline, bezierCurve: [ 0.001, 0, 0.999, 1, 1, 1 ] })
    readonly property easingCurve easing_00: ({ type: Easing.BezierSpline, bezierCurve: [ 0.42, 0, 0.58, 1, 1, 1 ] })
    readonly property easingCurve easing_01: ({ type: Easing.BezierSpline, bezierCurve: [ 0.42, 0, 0.999, 1, 1, 1 ] })
    readonly property easingCurve easing_04: ({ type: Easing.BezierSpline, bezierCurve: [ 0.88, 0.14, 0.12, 0.86, 1, 1 ] })
    readonly property easingCurve easing_03: ({ type: Easing.BezierSpline, bezierCurve: [ 0.9, 0, 0.1, 1, 1, 1 ] })
}
