// Generated from Lottie file lock_animation.json
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
            Translate { x: 106; y: 121.989}
            Matrix4x4 { matrix: _qt_node2.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node3
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node4_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 102.322 -32.715 L 92.26 -86.382 Q 92.1621 -86.8265 91.799 -87.098 Q 91.4371 -87.3687 90.983 -87.336 Q 74.756 -85.9604 61.275 -95.126 Q 47.7962 -104.29 43.057 -119.922 Q 42.9311 -120.377 42.531 -120.625 Q 42.1292 -120.874 41.668 -120.782 L -14.238 -110.241 L -70.144 -99.7 Q -70.5933 -99.6043 -70.865 -99.228 Q -71.1357 -98.853 -71.086 -98.393 Q -69.8544 -82.1175 -79.094 -68.68 Q -88.3343 -55.2416 -103.937 -50.615 Q -104.39 -50.4859 -104.636 -50.086 Q -104.882 -49.6876 -104.794 -49.222 L -103.223 -40.8365 L -99.7675 -22.3885 L -96.3118 -3.94047 L -94.741 4.445 Q -86.106 51.1251 -51.288 83.278 Q -17.3186 114.896 28.641 120.799 Q 29.0207 120.836 29.361 120.661 Q 70.0991 98.4321 90.323 56.565 Q 111.151 13.9306 102.322 -32.715 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node5
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node5_transform_base_group
            Translate { x: 106; y: 121.397}
            Matrix4x4 { matrix: _qt_node2.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node6
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node7_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -69.002 -97.198 L -51.7615 -100.435 L -13.8325 -107.556 L 41.337 -117.914 Q 43.8181 -110.02 48.4874 -103.751 Q 53.1566 -97.4831 60.014 -92.841 Q 66.8643 -88.2037 74.4145 -86.1932 Q 81.9648 -84.1827 90.215 -84.799 L 95.176 -58.449 L 100.137 -32.099 L 100.138 -32.096 Q 102.308 -20.6816 102.656 -9.51258 Q 103.003 1.65648 101.529 12.5802 Q 100.054 23.504 96.758 34.1824 Q 93.4615 44.8609 88.343 55.294 L 88.339 55.301 Q 83.3778 65.5278 77.1601 74.5583 Q 70.9424 83.5889 63.4681 91.4231 Q 55.9939 99.2574 47.2631 105.895 Q 38.5323 112.533 28.545 117.975 Q 5.99882 115.064 -13.6057 105.886 Q -33.2102 96.7075 -49.873 81.263 L -49.8755 81.2605 L -49.878 81.258 Q -66.9911 65.5221 -77.6698 46.2308 Q -88.3486 26.9396 -92.593 4.093 L -94.1432 -4.147 L -95.7321 -12.593 L -97.5535 -22.275 L -102.514 -48.643 Q -86.6908 -53.4756 -77.297 -67.08 Q -67.8959 -80.6954 -69.002 -97.198 M 93.467 -85.955 L 91.801 -85.587 L 93.478 -85.902 L 98.4835 -59.317 L 103.489 -32.732 Q 107.935 -9.34509 104.914 13.0354 Q 101.894 35.4159 91.408 56.79 Q 81.2271 77.7752 65.8826 93.8395 Q 50.5381 109.904 30.03 121.047 L 30.0176 121.054 L 29.989 121.069 Q 29.2107 121.466 28.337 121.383 L 28.3095 121.38 L 28.282 121.377 Q 16.7138 119.897 5.90014 116.807 Q -4.91349 113.716 -14.9725 109.014 Q -25.0315 104.313 -34.3359 97.9998 Q -43.6402 91.6869 -52.19 83.763 Q -60.9556 75.7022 -68.0732 66.7314 Q -75.1908 57.7606 -80.6604 47.8797 Q -86.13 37.9989 -89.9517 27.2079 Q -93.7733 16.417 -95.947 4.716 L -95.947 4.715 L -105.949 -48.451 L -105.949 -48.454 Q -106.159 -49.5614 -105.567 -50.52 Q -104.977 -51.4739 -103.897 -51.785 Q -96.4258 -53.9924 -90.4778 -58.2999 Q -84.5298 -62.6074 -80.105 -69.015 Q -75.6835 -75.4175 -73.7652 -82.498 Q -71.847 -89.5785 -72.432 -97.337 Q -72.5375 -98.4263 -71.895 -99.314 Q -71.2402 -100.218 -70.15 -100.449 L -70.1283 -100.453 L -70.11 -100.457 L 41.131 -121.342 Q 41.1325 -121.342 41.1355 -121.343 Q 41.1385 -121.343 41.14 -121.344 Q 42.2516 -121.559 43.215 -120.965 Q 44.1692 -120.377 44.479 -119.298 Q 49.0215 -104.399 61.927 -95.661 Q 74.8416 -86.9178 90.387 -88.23 L 90.397 -88.231 L 90.407 -88.232 Q 91.4898 -88.311 92.361 -87.663 Q 93.2331 -87.0143 93.467 -85.955 M -102.597 -49.084 Q -102.597 -49.0845 -102.598 -49.086 Q -102.598 -49.0875 -102.598 -49.088 L -102.597 -49.084 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node8
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node8_transform_base_group
            Translate { x: 106.592; y: 123.173}
            Matrix4x4 { matrix: _qt_node2.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node9
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node10_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff6f4ae7"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 24.258 98.185 Q 57.8447 80.0954 74.524 46.034 Q 91.6817 11.352 84.4 -26.583 L 76.108 -70.254 Q 76.0521 -70.6439 75.739 -70.89 Q 75.4269 -71.1353 75.029 -71.101 Q 61.6586 -69.9822 50.548 -77.433 Q 39.4383 -84.8832 35.528 -97.591 Q 35.416 -97.9566 35.089 -98.155 Q 34.7605 -98.3543 34.382 -98.286 L -16.107 -88.899 L -36.9745 -85.015 L -57.842 -81.131 Q -58.2183 -81.0579 -58.449 -80.754 Q -58.68 -80.4496 -58.646 -80.072 Q -57.6323 -66.8309 -65.25 -55.896 Q -72.8655 -44.9641 -85.731 -41.196 Q -86.1053 -41.09 -86.308 -40.765 Q -86.5123 -40.4375 -86.441 -40.061 L -82.295 -18.226 L -78.149 3.609 Q -71.039 41.6239 -42.293 67.797 Q -14.2551 93.5267 23.668 98.295 Q 23.9811 98.3318 24.258 98.185 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node11
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node11_transform_base_group
            Translate { x: 76.391; y: 127.911}
            Matrix4x4 { matrix: _qt_node2.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node12
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node13_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff5d42d1"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 55.657 93.557 Q 55.9754 93.5939 56.257 93.447 L 37.5555 0.17 L 18.854 -93.107 L 18.2887 -93.1784 L 17.045 -93.3355 L 15.236 -93.564 L -27.176 -85.798 Q -27.5575 -85.7251 -27.793 -85.421 Q -28.028 -85.1175 -27.994 -84.74 Q -26.9631 -71.5001 -34.704 -60.571 Q -42.4446 -49.6424 -55.518 -45.876 Q -55.8955 -45.7708 -56.104 -45.444 Q -56.3127 -45.1169 -56.24 -44.741 L -52.0265 -22.912 L -49.1297 -7.90456 L -47.813 -1.083 Q -40.5829 36.9116 -11.375 63.069 Q 17.1172 88.7903 55.657 93.557 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node14
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node14_transform_base_group
            Translate { x: 113.698; y: 120.212}
            Matrix4x4 { matrix: _qt_node2.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node15
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node16_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 35.012 -19.814 Q 35.0812 -19.9087 35.1609 -20.03 L 35.303 -20.252 L 35.303 -20.244 Q 36.2887 -21.7476 36.592 -23.505 Q 36.8945 -25.2582 36.465 -26.993 Q 36.036 -28.7257 34.944 -30.16 Q 33.852 -31.5943 32.267 -32.508 Q 30.6811 -33.4223 28.854 -33.669 Q 27.0286 -33.9155 25.242 -33.459 Q 23.459 -33.0034 21.997 -31.913 Q 20.5349 -30.8225 19.63 -29.273 L 6.0625 -8.7795 L -7.505 11.714 Q -7.74879 12.075 -8.186 12.164 Q -8.62395 12.2532 -8.997 12.02 L -15.766 7.8565 L -22.535 3.693 Q -24.065 2.74983 -25.85 2.444 Q -27.6363 2.13795 -29.408 2.515 Q -31.1812 2.89237 -32.667 3.894 Q -34.1547 4.89689 -35.13 6.372 L -35.1541 6.407 L -35.207 6.484 L -35.284 6.596 Q -37.3154 9.66891 -36.5 13.229 Q -35.6846 16.7891 -32.498 18.751 L -21.4335 25.56 L -13.8267 30.2412 L -10.369 32.369 Q -8.84049 33.3118 -7.052 33.618 Q -5.2613 33.9245 -3.492 33.548 Q -1.71804 33.1705 -0.231 32.168 Q 1.25755 31.1645 2.234 29.69 L 35.012 -19.814 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node17
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node17_transform_base_group
            Translate { x: 113.698; y: 119.62}
            Matrix4x4 { matrix: _qt_node2.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node18
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node19_path_fill
                objectName: "svg_fill_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 24.986 -31.474 Q 26.2239 -31.8062 27.49 -31.626 Q 28.7575 -31.4456 29.855 -30.781 Q 30.9517 -30.1169 31.71 -29.072 Q 32.4666 -28.0294 32.764 -26.767 Q 33.0603 -25.5092 32.851 -24.229 Q 32.653 -23.02 32.027 -21.968 Q 31.9857 -21.9093 31.939 -21.831 Q 31.8028 -21.6045 31.748 -21.526 L 31.7156 -21.4779 L 31.684 -21.429 L 0.064 28.731 Q -0.612392 29.8038 -1.644 30.534 Q -2.67543 31.264 -3.903 31.538 Q -5.13021 31.8119 -6.369 31.589 Q -7.60711 31.3662 -8.666 30.68 L -12.0018 28.5238 L -19.3405 23.78 L -30.015 16.88 Q -32.2229 15.4518 -32.788 12.862 Q -33.3501 10.2842 -31.958 8.054 L -31.825 7.851 L -31.811 7.8295 L -31.797 7.808 Q -31.1198 6.73305 -30.092 6.006 Q -29.0615 5.27707 -27.835 5.003 Q -26.607 4.7286 -25.371 4.951 Q -24.1357 5.17327 -23.076 5.86 L -16.5475 10.0775 L -10.019 14.295 Q -8.79941 15.0898 -7.381 14.784 Q -5.95738 14.478 -5.162 13.242 L -7.24 11.869 L -5.149 13.222 L 7.9395 -7.5435 L 12.7454 -15.1683 L 16.9378 -21.8198 L 21.028 -28.309 Q 21.076 -28.387 21.098 -28.427 Q 21.7273 -29.5567 22.738 -30.348 Q 23.7538 -31.1433 24.986 -31.474 M -11.338 34.917 Q -9.44845 36.1404 -7.237 36.539 Q -5.02911 36.937 -2.834 36.447 Q -0.641187 35.9575 1.199 34.655 Q 3.04155 33.3508 4.247 31.436 L 9.18325 23.6052 L 20.043 6.3775 L 30.9028 -10.8502 L 35.839 -18.681 L -11.338 34.917 M 35.839 -18.681 L 35.9748 -18.8876 L 36.091 -19.074 Q 36.1161 -19.1094 36.154 -19.17 Q 37.3724 -21.1224 37.747 -23.405 Q 38.1214 -25.6862 37.591 -27.935 Q 37.0603 -30.185 35.71 -32.047 Q 34.358 -33.9113 32.4 -35.097 Q 30.4422 -36.2826 28.18 -36.604 Q 25.9211 -36.925 23.713 -36.332 Q 21.5072 -35.7396 19.702 -34.324 Q 17.9244 -32.9301 16.811 -30.959 L 4.1295 -10.8395 L -8.552 9.28 L -20.403 1.625 Q -22.2951 0.400001 -24.503 0.002 Q -26.7098 -0.395795 -28.904 0.094 Q -31.0961 0.583322 -32.935 1.886 Q -34.7631 3.18095 -35.967 5.083 L -36.102 5.289 L -36.1124 5.30479 Q -36.1244 5.32262 -36.13 5.332 Q -38.6418 9.32464 -37.633 13.947 Q -36.6239 18.5707 -32.685 21.118 L -11.339 34.916 L 35.839 -18.681 M -11.339 34.916 L 35.839 -18.681 L 12.2505 8.118 L -11.338 34.917 L -11.339 34.916 " }
            }
        }
    }
    LayerItem {
        objectName: " Shield"
        id: _qt_node2
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node2_transform_base_group
            TransformGroup {
                id: _qt_node2_transform_group_0
                Translate { x: -106; y: -121.397 }
                Scale { id: _qt_node2_transform_0_3}
                Rotation { id: _qt_node2_transform_0_2 }
                Translate { id: _qt_node2_transform_0_1 }
                Translate { id: _qt_node2_transform_0_0 }
            }
            Matrix4x4 { matrix: _qt_node20.transformMatrix }
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
                                script:_qt_node2_transform_0_0.x = 494
                            }
                            ScriptAction {
                                script:_qt_node2_transform_0_0.y = 0
                            }
                        }
                        PauseAnimation { duration: 2167 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 666
                                target: _qt_node2_transform_0_0
                                property: "x"
                                to: 170
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 666
                                target: _qt_node2_transform_0_0
                                property: "y"
                                to: 0
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 1167 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 667
                                target: _qt_node2_transform_0_0
                                property: "x"
                                to: 494
                                easing: _qt_node0.easing_01
                            }
                            PropertyAnimation {
                                duration: 667
                                target: _qt_node2_transform_0_0
                                property: "y"
                                to: 0
                                easing: _qt_node0.easing_01
                            }
                        }
                        PauseAnimation { duration: 300 }
                    }
                    SequentialAnimation {
                        ParallelAnimation {
                            ScriptAction {
                                script:_qt_node2_transform_0_1.x = 0
                            }
                            ScriptAction {
                                script:_qt_node2_transform_0_1.y = 255.922
                            }
                        }
                        PauseAnimation { duration: 2167 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 666
                                target: _qt_node2_transform_0_1
                                property: "x"
                                to: 0
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 666
                                target: _qt_node2_transform_0_1
                                property: "y"
                                to: 134.922
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 1167 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 667
                                target: _qt_node2_transform_0_1
                                property: "x"
                                to: 0
                                easing: _qt_node0.easing_01
                            }
                            PropertyAnimation {
                                duration: 667
                                target: _qt_node2_transform_0_1
                                property: "y"
                                to: 255.922
                                easing: _qt_node0.easing_01
                            }
                        }
                        PauseAnimation { duration: 300 }
                    }
                    SequentialAnimation {
                        ParallelAnimation {
                            ScriptAction {
                                script:_qt_node2_transform_0_2.angle = -360
                            }
                        }
                        PauseAnimation { duration: 2167 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 666
                                target: _qt_node2_transform_0_2
                                property: "angle"
                                to: 0
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 1167 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 667
                                target: _qt_node2_transform_0_2
                                property: "angle"
                                to: -360
                                easing: _qt_node0.easing_01
                            }
                        }
                        PauseAnimation { duration: 300 }
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
                        PauseAnimation { duration: 2167 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 666
                                target: _qt_node2_transform_0_3
                                property: "xScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 666
                                target: _qt_node2_transform_0_3
                                property: "yScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 1167 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 667
                                target: _qt_node2_transform_0_3
                                property: "xScale"
                                to: 0.1
                                easing: _qt_node0.easing_01
                            }
                            PropertyAnimation {
                                duration: 667
                                target: _qt_node2_transform_0_3
                                property: "yScale"
                                to: 0.1
                                easing: _qt_node0.easing_01
                            }
                        }
                        PauseAnimation { duration: 300 }
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
                    PauseAnimation { duration: 2167 }
                    PropertyAnimation {
                        duration: 666
                        target: _qt_node2
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 1167 }
                    PropertyAnimation {
                        duration: 667
                        target: _qt_node2
                        property: "opacity"
                        to: 0
                        easing: _qt_node0.easing_01
                    }
                    PauseAnimation { duration: 300 }
                }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node21
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node21_transform_base_group
            Translate { x: 118.406; y: 257.346}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node23
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node24_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffc3c5ff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -80.027 -66.387 L 80.027 -66.387 Q 82.2179 -66.387 83.7669 -64.8364 Q 85.316 -63.2856 85.316 -61.092 L 85.316 61.092 Q 85.316 63.2856 83.7669 64.8364 Q 82.2179 66.387 80.027 66.387 L -80.027 66.387 Q -82.2179 66.387 -83.7669 64.8364 Q -85.316 63.2856 -85.316 61.092 L -85.316 -61.092 Q -85.316 -63.2856 -83.7669 -64.8364 Q -82.2179 -66.387 -80.027 -66.387 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node25
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node25_transform_base_group
            Translate { x: 118.405; y: 257.347}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node26
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node27_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffc2c6ff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -85.038 -61.092 Q -85.038 -63.17 -83.5701 -64.6391 Q -82.1025 -66.108 -80.027 -66.108 L 80.027 -66.108 Q 81.0647 -66.108 81.9505 -65.7408 Q 82.8363 -65.3736 83.5701 -64.6391 Q 84.3041 -63.9046 84.671 -63.0178 Q 85.038 -62.131 85.038 -61.092 L 85.038 61.092 Q 85.038 62.131 84.671 63.0178 Q 84.3041 63.9046 83.5701 64.6391 Q 82.8363 65.3736 81.9505 65.7408 Q 81.0647 66.108 80.027 66.108 L -80.027 66.108 Q -82.1025 66.108 -83.5701 64.6391 Q -85.038 63.17 -85.038 61.092 L -85.038 -61.092 M -80.027 -66.666 Q -81.1801 -66.666 -82.1644 -66.2578 Q -83.1487 -65.8496 -83.9641 -65.0332 Q -84.7796 -64.2169 -85.1873 -63.2316 Q -85.595 -62.2463 -85.595 -61.092 L -85.595 61.092 Q -85.595 62.2463 -85.1873 63.2316 Q -84.7796 64.2169 -83.9641 65.0332 Q -83.1487 65.8496 -82.1644 66.2578 Q -81.1801 66.666 -80.027 66.666 L 80.027 66.666 Q 82.3332 66.666 83.9641 65.0332 Q 85.595 63.4006 85.595 61.092 L 85.595 -61.092 Q 85.595 -63.4006 83.9641 -65.0332 Q 82.3332 -66.666 80.027 -66.666 L -80.027 -66.666 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node28
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node28_transform_base_group
            Translate { x: 104.113; y: 5.938}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node29
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node30_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -0.372 5.306 Q -0.189857 5.381 0.003 5.381 Q 0.194857 5.381 0.377 5.306 Q 0.5538 5.2332 0.694 5.093 Q 0.8301 4.9569 0.905 4.775 Q 0.978523 4.59644 0.977 4.4 L 0.977 -4.474 Q 0.947482 -4.85774 0.666 -5.119 Q 0.383726 -5.381 0 -5.381 Q -0.383726 -5.381 -0.666 -5.119 Q -0.947482 -4.85774 -0.977 -4.474 L -0.977 4.4 Q -0.977 4.59529 -0.903 4.775 Q -0.8288 4.9552 -0.69 5.094 Q -0.5522 5.2318 -0.372 5.306 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node31
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node31_transform_base_group
            Translate { x: 104.358; y: 5.381}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node32
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node33_path_fill
                objectName: "svg_fill_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -0.532 -4.153 Q -0.41126 -4.266 -0.245 -4.266 Q -0.0777397 -4.266 0.043 -4.153 Q 0.157355 -4.04637 0.176 -3.891 L 0.176 4.957 L 0.176 4.961 Q 0.176 5.04571 0.145 5.121 Q 0.1128 5.1992 0.054 5.258 Q -0.0062 5.3182 -0.081 5.349 Q -0.158714 5.381 -0.242 5.381 Q -0.326286 5.381 -0.404 5.349 Q -0.4805 5.3175 -0.541 5.257 Q -0.6015 5.1965 -0.633 5.12 Q -0.665 5.04229 -0.665 4.957 L -0.665 -3.891 Q -0.646355 -4.04637 -0.532 -4.153 M 1.289 4.956 L 1.289 -3.938 L 1.288 -3.958 Q 1.24295 -4.55925 0.8 -4.971 Q 0.358939 -5.381 -0.245 -5.381 Q -0.847939 -5.381 -1.289 -4.971 L 1.289 4.956 M 1.289 4.956 L 0.886187 3.40491 L 0 -0.0075 L -0.886187 -3.41991 L -1.289 -4.971 L 1.289 4.956 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node34
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node34_transform_base_group
            Translate { x: 104.172; y: 5.904}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node35
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node36_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 5.3 0.372 Q 5.37352 0.193445 5.372 -0.003 Q 5.372 -0.408 5.087 -0.693 Q 4.802 -0.978 4.398 -0.978 L -4.466 -0.978 Q -4.84985 -0.948353 -5.111 -0.667 Q -5.372 -0.385804 -5.372 0 Q -5.372 0.385804 -5.111 0.667 Q -4.84985 0.948353 -4.466 0.978 L 4.398 0.978 Q 4.58986 0.978 4.772 0.903 Q 4.9505 0.8295 5.089 0.691 Q 5.2244 0.5556 5.3 0.372 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node37
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node37_transform_base_group
            Translate { x: 104.729; y: 5.346}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node38
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node39_path_fill
                objectName: "svg_fill_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 5.372 0.553 Q 5.372 -0.081 4.924 -0.529 Q 4.475 -0.978 3.841 -0.978 L -5.044 -0.978 L 5.372 0.553 M -5.259 0.269 Q -5.15342 0.155771 -4.997 0.137 L 3.841 0.137 Q 4.014 0.137 4.136 0.259 Q 4.259 0.382 4.259 0.555 L 4.259 0.558 Q 4.259 0.641286 4.227 0.719 Q 4.1948 0.7972 4.137 0.855 Q 4.0775 0.9145 4.001 0.946 Q 3.92329 0.978 3.841 0.978 L -4.997 0.978 Q -5.15237 0.959355 -5.259 0.845 Q -5.372 0.72426 -5.372 0.557 Q -5.372 0.38974 -5.259 0.269 M 5.372 0.553 L -5.044 -0.978 L 5.372 0.553 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node40
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node40_transform_base_group
            Translate { x: 162.109; y: 351.521}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node41
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node42_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -3.246 0 Q -3.246 -1.34629 -2.29538 -2.29813 Q -1.34471 -3.25 0 -3.25 Q 1.34471 -3.25 2.29538 -2.29813 Q 3.246 -1.34629 3.246 0 Q 3.246 1.34629 2.29538 2.29813 Q 1.34471 3.25 0 3.25 Q -1.34471 3.25 -2.29538 2.29813 Q -3.246 1.34629 -3.246 0 M 0 -5.479 Q -2.2672 -5.479 -3.87013 -3.87425 Q -5.473 -2.26955 -5.473 0 Q -5.473 2.26955 -3.87013 3.87425 Q -2.2672 5.479 0 5.479 Q 2.2672 5.479 3.87013 3.87425 Q 5.473 2.26955 5.473 0 Q 5.473 -2.26955 3.87013 -3.87425 Q 2.2672 -5.479 0 -5.479 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node43
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node43_transform_base_group
            Translate { x: 5.473; y: 272.709}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node44
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node45_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -3.246 0 Q -3.246 -1.34629 -2.29538 -2.29813 Q -1.34471 -3.25 0 -3.25 Q 1.34471 -3.25 2.29538 -2.29813 Q 3.246 -1.34629 3.246 0 Q 3.246 1.34629 2.29538 2.29813 Q 1.34471 3.25 0 3.25 Q -1.34471 3.25 -2.29538 2.29813 Q -3.246 1.34629 -3.246 0 M 0 -5.479 Q -2.2672 -5.479 -3.87013 -3.87425 Q -5.473 -2.26955 -5.473 0 Q -5.473 2.26955 -3.87013 3.87425 Q -2.2672 5.479 0 5.479 Q 2.2672 5.479 3.87013 3.87425 Q 5.473 2.26955 5.473 0 Q 5.473 -2.26955 3.87013 -3.87425 Q 2.2672 -5.479 0 -5.479 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node46
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node46_transform_base_group
            Translate { x: 211.95; y: 88.599}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node47
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node48_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffc3c5ff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -156.819 -28.244 L 156.819 -28.244 L 156.819 28.244 L -156.819 28.244 L -156.819 -28.244 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node49
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node49_transform_base_group
            Translate { x: 211.951; y: 88.597}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node50
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node51_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffc2c6ff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -156.819 -28.522 Q -156.935 -28.522 -157.016 -28.4408 Q -157.098 -28.3594 -157.098 -28.244 L -157.098 28.244 Q -157.098 28.3594 -157.016 28.4408 Q -156.935 28.522 -156.819 28.522 L 156.819 28.522 Q 156.935 28.522 157.016 28.4408 Q 157.098 28.3594 157.098 28.244 L 157.098 -28.244 Q 157.098 -28.3594 157.016 -28.4408 Q 156.935 -28.522 156.819 -28.522 L -156.819 -28.522 M -156.541 27.965 L -156.541 -27.965 L 156.541 -27.965 L 156.541 27.965 L -156.541 27.965 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node52
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node52_transform_base_group
            Translate { x: 211.95; y: 210.921}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node53
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node54_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -156.819 -92.451 L 156.819 -92.451 L 156.819 92.451 L -156.819 92.451 L -156.819 -92.451 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node55
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node55_transform_base_group
            Translate { x: 211.127; y: 182.894}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node56
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node57_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -156.825 -122.824 Q -157.286 -122.824 -157.612 -122.497 Q -157.938 -122.171 -157.938 -121.709 L -157.938 118.46 Q -157.938 119.329 -157.607 120.13 Q -157.274 120.934 -156.662 121.546 Q -156.047 122.161 -155.247 122.492 Q -154.445 122.824 -153.579 122.824 L 153.579 122.824 Q 154.445 122.824 155.247 122.492 Q 156.047 122.161 156.662 121.546 Q 157.275 120.933 157.607 120.13 Q 157.938 119.329 157.938 118.46 L 157.938 -121.709 Q 157.938 -122.171 157.612 -122.497 Q 157.286 -122.824 156.825 -122.824 L -156.825 -122.824 M -155.711 118.46 L -155.711 -120.595 L 155.711 -120.595 L 155.711 118.46 Q 155.711 118.885 155.549 119.277 Q 155.387 119.669 155.087 119.969 Q 154.785 120.271 154.395 120.432 Q 154 120.595 153.579 120.595 L -153.579 120.595 Q -154 120.595 -154.395 120.432 Q -154.785 120.271 -155.087 119.969 Q -155.387 119.669 -155.549 119.277 Q -155.711 118.885 -155.711 118.46 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node58
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node58_transform_base_group
            Translate { x: 211.126; y: 117.17}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node59
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node60_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -156.825 1.115 L 156.825 1.115 L 156.825 -1.115 L -156.825 -1.115 L -156.825 1.115 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node61
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node61_transform_base_group
            Translate { x: 81.739; y: 91.296}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node62
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node63_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -5.573 0 Q -5.573 -2.31148 -3.94075 -3.94575 Q -2.30852 -5.58 0 -5.58 Q 2.30852 -5.58 3.94075 -3.94575 Q 5.573 -2.31148 5.573 0 Q 5.573 2.31148 3.94075 3.94575 Q 2.30852 5.58 0 5.58 Q -2.30852 5.58 -3.94075 3.94575 Q -5.573 2.31148 -5.573 0 M 0 -7.809 Q -3.231 -7.809 -5.5155 -5.52187 Q -7.8 -3.23475 -7.8 0 Q -7.8 3.23475 -5.5155 5.52187 Q -3.231 7.809 0 7.809 Q 3.231 7.809 5.5155 5.52187 Q 7.8 3.23475 7.8 0 Q 7.8 -3.23475 5.5155 -5.52187 Q 3.231 -7.809 0 -7.809 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node64
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node64_transform_base_group
            Translate { x: 111.711; y: 90.883}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node65
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node66_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -5.573 0 Q -5.573 -2.31148 -3.94075 -3.94575 Q -2.30852 -5.58 0 -5.58 Q 2.30852 -5.58 3.94075 -3.94575 Q 5.573 -2.31148 5.573 0 Q 5.573 2.31148 3.94075 3.94575 Q 2.30852 5.58 0 5.58 Q -2.30852 5.58 -3.94075 3.94575 Q -5.573 2.31148 -5.573 0 M 0 -7.809 Q -3.231 -7.809 -5.5155 -5.52187 Q -7.8 -3.23475 -7.8 0 Q -7.8 3.23475 -5.5155 5.52187 Q -3.231 7.809 0 7.809 Q 3.231 7.809 5.5155 5.52187 Q 7.8 3.23475 7.8 0 Q 7.8 -3.23475 5.5155 -5.52187 Q 3.231 -7.809 0 -7.809 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node67
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node67_transform_base_group
            Translate { x: 141.686; y: 90.883}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node68
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node69_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -5.573 0 Q -5.573 -2.31148 -3.94075 -3.94575 Q -2.30852 -5.58 0 -5.58 Q 2.30852 -5.58 3.94075 -3.94575 Q 5.573 -2.31148 5.573 0 Q 5.573 2.31148 3.94075 3.94575 Q 2.30852 5.58 0 5.58 Q -2.30852 5.58 -3.94075 3.94575 Q -5.573 2.31148 -5.573 0 M 0 -7.809 Q -3.231 -7.809 -5.5155 -5.52187 Q -7.8 -3.23475 -7.8 0 Q -7.8 3.23475 -5.5155 5.52187 Q -3.231 7.809 0 7.809 Q 3.231 7.809 5.5155 5.52187 Q 7.8 3.23475 7.8 0 Q 7.8 -3.23475 5.5155 -5.52187 Q 3.231 -7.809 0 -7.809 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node70
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node70_transform_base_group
            Translate { x: 59.714; y: 43.061}
            Matrix4x4 { matrix: _qt_node71.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node72
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node73_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -37.398 15.005 L -37.398 41.945 L -58.6 41.945 L -58.6 16.722 Q -58.6 -7.57926 -41.436 -24.762 Q -24.2717 -41.945 0 -41.945 Q 24.2717 -41.945 41.436 -24.762 Q 58.6 -7.57926 58.6 16.722 L 58.6 41.945 L 37.398 41.945 L 37.398 15.005 Q 37.398 -0.502506 26.444 -11.469 Q 15.4895 -22.436 0 -22.436 Q -15.4895 -22.436 -26.444 -11.469 Q -37.398 -0.502506 -37.398 15.005 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node74
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node74_transform_base_group
            Translate { x: 59.713; y: 43.06}
            Matrix4x4 { matrix: _qt_node71.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node75
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node76_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 0 -43.06 Q -6.18358 -43.06 -11.9144 -41.9656 Q -17.6452 -40.8713 -22.9232 -38.6825 Q -28.2012 -36.4938 -33.0264 -33.2106 Q -37.8516 -29.9275 -42.224 -25.55 Q -46.5962 -21.1727 -49.8754 -16.342 Q -53.1546 -11.5113 -55.3407 -6.22735 Q -57.5269 -0.943348 -58.6199 4.79399 Q -59.713 10.5313 -59.713 16.722 L -59.713 41.945 Q -59.713 42.407 -59.3871 42.7335 Q -59.0612 43.06 -58.6 43.06 L -37.398 43.06 Q -36.9368 43.06 -36.6109 42.7335 Q -36.285 42.407 -36.285 41.945 L -36.285 15.005 Q -36.285 7.48147 -33.628 1.05997 Q -30.971 -5.36153 -25.657 -10.681 Q -20.3425 -16.001 -13.9282 -18.661 Q -7.51397 -21.321 0 -21.321 Q 7.51397 -21.321 13.9282 -18.661 Q 20.3425 -16.001 25.657 -10.681 Q 30.971 -5.36153 33.628 1.05997 Q 36.285 7.48147 36.285 15.005 L 36.285 41.945 Q 36.285 42.407 36.6109 42.7335 Q 36.9368 43.06 37.398 43.06 L 58.6 43.06 Q 59.0612 43.06 59.3871 42.7335 Q 59.713 42.407 59.713 41.945 L 59.713 16.722 Q 59.713 4.34065 55.3408 -6.22735 Q 50.9685 -16.7953 42.224 -25.55 Q 37.8516 -29.9275 33.0264 -33.2106 Q 28.2012 -36.4938 22.9232 -38.6825 Q 17.6452 -40.8713 11.9144 -41.9656 Q 6.18358 -43.06 0 -43.06 M -40.649 -23.973 Q -32.2299 -32.4015 -22.0676 -36.6158 Q -11.9054 -40.83 0 -40.83 Q 11.9054 -40.83 22.0676 -36.6158 Q 32.2299 -32.4015 40.649 -23.973 Q 44.8582 -19.7591 48.0152 -15.1087 Q 51.1721 -10.4583 53.2767 -5.37138 Q 55.3814 -0.284505 56.4337 5.23884 Q 57.486 10.7622 57.486 16.722 L 57.486 40.83 L 38.512 40.83 L 38.512 15.005 Q 38.512 7.02053 35.692 0.204778 Q 32.872 -6.61097 27.232 -12.258 Q 21.593 -17.904 14.785 -20.727 Q 7.97703 -23.55 0 -23.55 Q -7.97703 -23.55 -14.785 -20.727 Q -21.593 -17.904 -27.232 -12.258 Q -32.872 -6.61097 -35.692 0.204778 Q -38.512 7.02053 -38.512 15.005 L -38.512 40.83 L -57.486 40.83 L -57.486 16.722 Q -57.486 4.80237 -53.2768 -5.37138 Q -49.0675 -15.5451 -40.649 -23.973 " }
            }
        }
    }
    LayerItem {
        objectName: " Lock Mech"
        id: _qt_node71
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node71_transform_base_group
            TransformGroup {
                id: _qt_node71_transform_group_0
                Translate { x: -59.713; y: -43.06 }
                Rotation { id: _qt_node71_transform_0_2 }
                Translate { id: _qt_node71_transform_0_1 }
                Translate { id: _qt_node71_transform_0_0 }
            }
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node71_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node71_transform_animation
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
                                script:_qt_node71_transform_0_0.x = 338.914
                            }
                            ScriptAction {
                                script:_qt_node71_transform_0_0.y = 0
                            }
                        }
                        PauseAnimation { duration: 1667 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node71_transform_0_0
                                property: "x"
                                to: 320.914
                                easing: _qt_node0.easing_02
                            }
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node71_transform_0_0
                                property: "y"
                                to: 0
                                easing: _qt_node0.easing_02
                            }
                        }
                        PauseAnimation { duration: 1833 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node71_transform_0_0
                                property: "x"
                                to: 338.914
                                easing: _qt_node0.easing_03
                            }
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node71_transform_0_0
                                property: "y"
                                to: 0
                                easing: _qt_node0.easing_03
                            }
                        }
                        PauseAnimation { duration: 467 }
                    }
                    SequentialAnimation {
                        ParallelAnimation {
                            ScriptAction {
                                script:_qt_node71_transform_0_1.x = 0
                            }
                            ScriptAction {
                                script:_qt_node71_transform_0_1.y = 196.421
                            }
                        }
                        PauseAnimation { duration: 1667 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node71_transform_0_1
                                property: "x"
                                to: 0
                                easing: _qt_node0.easing_02
                            }
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node71_transform_0_1
                                property: "y"
                                to: 207.421
                                easing: _qt_node0.easing_02
                            }
                        }
                        PauseAnimation { duration: 1833 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node71_transform_0_1
                                property: "x"
                                to: 0
                                easing: _qt_node0.easing_03
                            }
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node71_transform_0_1
                                property: "y"
                                to: 196.421
                                easing: _qt_node0.easing_03
                            }
                        }
                        PauseAnimation { duration: 467 }
                    }
                    SequentialAnimation {
                        ParallelAnimation {
                            ScriptAction {
                                script:_qt_node71_transform_0_2.angle = 45
                            }
                        }
                        PauseAnimation { duration: 1667 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node71_transform_0_2
                                property: "angle"
                                to: 0
                                easing: _qt_node0.easing_02
                            }
                        }
                        PauseAnimation { duration: 1833 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node71_transform_0_2
                                property: "angle"
                                to: 45
                                easing: _qt_node0.easing_03
                            }
                        }
                        PauseAnimation { duration: 467 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node71_transform_base_group.deactivateOverride(_qt_node71_transform_group_0)
                    }
                }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node77
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node77_transform_base_group
            Translate { x: 322.915; y: 293.056}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node78
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node79_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffad79ff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -61.245 -54.099 L 61.245 -54.099 Q 65.1495 -54.099 67.9102 -51.335 Q 70.671 -48.571 70.671 -44.662 L 70.671 44.662 Q 70.671 48.571 67.9102 51.335 Q 65.1495 54.099 61.245 54.099 L -61.245 54.099 Q -65.1495 54.099 -67.9102 51.335 Q -70.671 48.571 -70.671 44.662 L -70.671 -44.662 Q -70.671 -48.571 -67.9102 -51.335 Q -65.1495 -54.099 -61.245 -54.099 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node80
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node80_transform_base_group
            Translate { x: 322.914; y: 293.055}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node81
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node82_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -69.557 -44.662 Q -69.557 -48.1091 -67.1226 -50.5465 Q -64.6882 -52.984 -61.245 -52.984 L 61.245 -52.984 Q 64.6882 -52.984 67.1226 -50.5465 Q 69.557 -48.1091 69.557 -44.662 L 69.557 44.662 Q 69.557 48.1091 67.1226 50.5465 Q 64.6882 52.984 61.245 52.984 L -61.245 52.984 Q -64.6882 52.984 -67.1226 50.5465 Q -69.557 48.1091 -69.557 44.662 L -69.557 -44.662 M -61.245 -55.214 Q -65.6107 -55.214 -68.6974 -52.1235 Q -71.784 -49.033 -71.784 -44.662 L -71.784 44.662 Q -71.784 49.033 -68.6974 52.1235 Q -65.6107 55.214 -61.245 55.214 L 61.245 55.214 Q 65.6107 55.214 68.6974 52.1235 Q 71.784 49.033 71.784 44.662 L 71.784 -44.662 Q 71.784 -49.033 68.6974 -52.1235 Q 65.6107 -55.214 61.245 -55.214 L -61.245 -55.214 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node83
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node83_transform_base_group
            Translate { x: 322.915; y: 293.06}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node84
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node85_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 13.393 -0.936 Q 15.2955 -4.3889 15.294 -8.334 Q 15.294 -12.3983 13.282 -15.925 Q 11.2689 -19.4537 7.775 -21.518 Q 4.27908 -23.5835 0.222 -23.643 Q -3.83306 -23.7025 -7.389 -21.739 Q -10.9422 -19.777 -13.056 -16.308 Q -15.169 -12.8403 -15.288 -8.779 Q -15.407 -4.71811 -13.498 -1.132 Q -11.589 2.45401 -8.157 4.62 L -8.157 15.478 Q -8.157 18.8624 -5.768 21.253 Q -3.37856 23.644 0 23.644 Q 3.37856 23.644 5.768 21.253 Q 8.157 18.8624 8.157 15.478 L 8.157 4.62 Q 11.4895 2.51873 13.393 -0.936 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node86
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node86_transform_base_group
            Translate { x: 322.915; y: 293.061}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node87
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node88_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -6.851 -20.763 Q -3.55741 -22.5829 0.206 -22.528 Q 3.96942 -22.4731 7.209 -20.558 Q 10.4495 -18.6424 12.315 -15.372 Q 14.181 -12.1008 14.181 -8.334 L 15.294 -8.334 L 15.1201 -8.33384 L 14.7375 -8.3335 L 14.181 -8.333 Q 14.1825 -4.67668 12.418 -1.474 Q 10.6536 1.72839 7.563 3.677 Q 7.043 4.00441 7.043 4.62 L 7.043 15.478 Q 7.043 18.3994 4.98 20.464 Q 2.91556 22.53 0 22.53 Q -2.91556 22.53 -4.98 20.464 Q -7.043 18.3994 -7.043 15.478 L -7.043 4.62 Q -7.043 4.00542 -7.563 3.677 Q -10.7457 1.66763 -12.515 -1.656 Q -14.2849 -4.98057 -14.175 -8.746 Q -14.0652 -12.5104 -12.105 -15.727 Q -10.1454 -18.9426 -6.851 -20.763 M 16.408 -8.334 Q 16.4095 -4.10193 14.368 -0.397 Q 12.4865 3.01828 9.27 5.222 L 9.27 15.478 Q 9.27 19.3229 6.555 22.041 Q 3.84012 24.759 0 24.759 Q -3.84012 24.759 -6.555 22.041 Q -9.27 19.3229 -9.27 15.478 L -9.27 5.221 Q -12.5894 2.94557 -14.481 -0.608 Q -16.5277 -4.45281 -16.401 -8.811 Q -16.2744 -13.1674 -14.006 -16.889 Q -11.7383 -20.6095 -7.927 -22.715 Q -4.11469 -24.8211 0.238 -24.757 Q 4.59308 -24.6929 8.341 -22.478 Q 12.0896 -20.2627 14.249 -16.478 Q 16.408 -12.694 16.408 -8.334 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node89
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node89_transform_base_group
            TransformGroup {
                id: _qt_node89_transform_group_0
                Scale { id: _qt_node89_transform_0_2}
                Translate { x: 126.638; y: 206.224 }
            }
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node89_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node89_transform_animation
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
                                script:_qt_node89_transform_0_2.xScale = 0.3
                            }
                            ScriptAction {
                                script:_qt_node89_transform_0_2.yScale = 0.3
                            }
                        }
                        PauseAnimation { duration: 667 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node89_transform_0_2
                                property: "xScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node89_transform_0_2
                                property: "yScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 2333 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node89_transform_0_2
                                property: "xScale"
                                to: 0.3
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node89_transform_0_2
                                property: "yScale"
                                to: 0.3
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 967 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node89_transform_base_group.deactivateOverride(_qt_node89_transform_group_0)
                    }
                }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node89_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node89_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node89.opacity = 0
                    }
                    PauseAnimation { duration: 667 }
                    PropertyAnimation {
                        duration: 500
                        target: _qt_node89
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 2333 }
                    PropertyAnimation {
                        duration: 500
                        target: _qt_node89
                        property: "opacity"
                        to: 0
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 967 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node90
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node91_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff6f4ae7"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 25.805 -20.442 Q 28.4141 -19.7421 29.765 -17.401 Q 30.4147 -16.2798 30.588 -14.988 Q 30.7606 -13.7011 30.427 -12.444 Q 30.0947 -11.1917 29.305 -10.156 Q 28.518 -9.12385 27.393 -8.471 L 20.2275 -4.3295 L 13.062 -0.188 L 20.2275 3.9535 L 22.8586 5.47421 L 25.1538 6.80078 L 27.393 8.095 Q 29.6671 9.48003 30.327 12.062 Q 30.9865 14.6423 29.655 16.951 Q 28.3241 19.2585 25.76 19.978 Q 23.1977 20.697 20.862 19.416 L 13.699 15.2745 L 6.536 11.133 L 6.536 27.693 Q 6.57541 29.0179 6.095 30.253 Q 5.61326 31.4915 4.693 32.439 Q 3.76873 33.3906 2.552 33.906 Q 1.33152 34.423 0.008 34.423 Q -1.31452 34.423 -2.535 33.906 Q -3.75343 33.3899 -4.677 32.439 Q -5.59726 31.4915 -6.079 30.253 Q -6.55941 29.0179 -6.52 27.693 L -6.52 11.133 L -20.851 19.416 Q -21.976 20.0664 -23.263 20.236 Q -24.5523 20.4059 -25.805 20.07 Q -27.0583 19.734 -28.09 18.942 Q -29.1194 18.1518 -29.77 17.025 Q -30.4204 15.8985 -30.59 14.61 Q -30.7596 13.3211 -30.423 12.064 Q -30.0863 10.8067 -29.297 9.777 Q -28.5057 8.7447 -27.382 8.095 L -20.219 3.9535 L -13.056 -0.188 L -27.376 -8.471 Q -28.5029 -9.12253 -29.292 -10.152 Q -30.0829 -11.1837 -30.419 -12.439 Q -30.7555 -13.6954 -30.586 -14.983 Q -30.4163 -16.2726 -29.768 -17.398 Q -29.1209 -18.5213 -28.089 -19.315 Q -27.0578 -20.1081 -25.805 -20.444 Q -24.5498 -20.7806 -23.263 -20.611 Q -21.9737 -20.4411 -20.851 -19.792 L -13.688 -15.6505 L -6.525 -11.509 L -6.525 -28.075 Q -6.447 -30.727 -4.547 -32.574 Q -2.64495 -34.423 0.003 -34.423 Q 2.65095 -34.423 4.553 -32.574 Q 6.453 -30.727 6.531 -28.075 L 6.531 -11.509 L 20.857 -19.792 Q 23.1951 -21.1421 25.805 -20.442 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node92
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node92_transform_base_group
            TransformGroup {
                id: _qt_node92_transform_group_0
                Scale { id: _qt_node92_transform_0_2}
                Translate { x: 210.131; y: 206.224 }
            }
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node92_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node92_transform_animation
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
                                script:_qt_node92_transform_0_2.xScale = 0.3
                            }
                            ScriptAction {
                                script:_qt_node92_transform_0_2.yScale = 0.3
                            }
                        }
                        PauseAnimation { duration: 1333 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node92_transform_0_2
                                property: "xScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node92_transform_0_2
                                property: "yScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 1667 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node92_transform_0_2
                                property: "xScale"
                                to: 0.3
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 500
                                target: _qt_node92_transform_0_2
                                property: "yScale"
                                to: 0.3
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 967 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node92_transform_base_group.deactivateOverride(_qt_node92_transform_group_0)
                    }
                }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node92_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node92_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node92.opacity = 0
                    }
                    PauseAnimation { duration: 1333 }
                    PropertyAnimation {
                        duration: 500
                        target: _qt_node92
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 1667 }
                    PropertyAnimation {
                        duration: 500
                        target: _qt_node92
                        property: "opacity"
                        to: 0
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 967 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node93
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node94_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff6f4ae7"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 25.876 -20.442 Q 28.4851 -19.7421 29.836 -17.401 Q 31.187 -15.0551 30.486 -12.44 Q 29.7846 -9.82347 27.442 -8.471 L 25.2062 -7.17678 L 20.2875 -4.3295 L 13.133 -0.188 L 20.296 3.9535 L 27.459 8.095 Q 29.7331 9.48003 30.393 12.062 Q 31.0525 14.6423 29.721 16.951 Q 28.3901 19.2585 25.826 19.978 Q 23.2637 20.697 20.928 19.416 L 13.765 15.2745 L 8.84044 12.4272 L 6.602 11.133 L 6.602 27.693 Q 6.64141 29.0179 6.161 30.253 Q 5.67926 31.4915 4.759 32.439 Q 3.83543 33.3899 2.617 33.906 Q 1.39652 34.423 0.074 34.423 Q -1.24852 34.423 -2.469 33.906 Q -3.68743 33.3899 -4.611 32.439 Q -5.53126 31.4915 -6.013 30.253 Q -6.49341 29.0179 -6.454 27.693 L -6.454 11.133 L -20.785 19.416 Q -23.1207 20.697 -25.683 19.978 Q -28.2471 19.2585 -29.578 16.951 Q -30.9095 14.6423 -30.25 12.062 Q -29.5901 9.48003 -27.316 8.095 L -20.1505 3.9535 L -12.985 -0.188 L -20.1505 -4.3295 L -27.316 -8.471 Q -28.4706 -9.10354 -29.292 -10.139 Q -30.1123 -11.1731 -30.468 -12.443 Q -30.8234 -13.7119 -30.658 -15.023 Q -30.4925 -16.3345 -29.834 -17.476 Q -29.175 -18.6183 -28.124 -19.416 Q -27.0749 -20.2123 -25.796 -20.54 Q -24.5199 -20.867 -23.215 -20.673 Q -21.9124 -20.4793 -20.785 -19.792 L -6.454 -11.509 L -6.454 -28.075 Q -6.376 -30.727 -4.476 -32.574 Q -2.57395 -34.423 0.074 -34.423 Q 2.72195 -34.423 4.624 -32.574 Q 6.524 -30.727 6.602 -28.075 L 6.602 -11.509 L 13.765 -15.6505 L 20.928 -19.792 Q 23.2661 -21.1421 25.876 -20.442 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node95
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node95_transform_base_group
            TransformGroup {
                id: _qt_node95_transform_group_0
                Scale { id: _qt_node95_transform_0_2}
                Translate { x: 12.962; y: 104.101 }
            }
            Matrix4x4 { matrix: _qt_node96.transformMatrix }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node95_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node95_transform_animation
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
                                script:_qt_node95_transform_0_2.xScale = 0.3
                            }
                            ScriptAction {
                                script:_qt_node95_transform_0_2.yScale = 0.3
                            }
                        }
                        PauseAnimation { duration: 1833 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 167
                                target: _qt_node95_transform_0_2
                                property: "xScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 167
                                target: _qt_node95_transform_0_2
                                property: "yScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                        }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 667
                                target: _qt_node95_transform_0_2
                                property: "xScale"
                                to: 0.3
                                easing: _qt_node0.easing_04
                            }
                            PropertyAnimation {
                                duration: 667
                                target: _qt_node95_transform_0_2
                                property: "yScale"
                                to: 0.3
                                easing: _qt_node0.easing_04
                            }
                        }
                        PauseAnimation { duration: 2300 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node95_transform_base_group.deactivateOverride(_qt_node95_transform_group_0)
                    }
                }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node95_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node95_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node95.opacity = 0
                    }
                    PauseAnimation { duration: 1833 }
                    PropertyAnimation {
                        duration: 167
                        target: _qt_node95
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PropertyAnimation {
                        duration: 667
                        target: _qt_node95
                        property: "opacity"
                        to: 0
                        easing: _qt_node0.easing_04
                    }
                    PauseAnimation { duration: 2300 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node97
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node98_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -8.025 -8.935 Q -8.36386 -9.24837 -8.82463 -9.22975 Q -9.28512 -9.21114 -9.598 -8.872 Q -9.91087 -8.53286 -9.89287 -8.07175 Q -9.87487 -7.61038 -9.536 -7.297 L 8.025 8.935 Q 8.36386 9.24837 8.82463 9.22975 Q 9.28512 9.21114 9.598 8.872 Q 9.91087 8.53286 9.89287 8.07175 Q 9.87487 7.61038 9.536 7.297 L -8.025 -8.935 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node99
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node99_transform_base_group
            TransformGroup {
                id: _qt_node99_transform_group_0
                Scale { id: _qt_node99_transform_0_2}
                Translate { x: 28.997; y: 73.555 }
            }
            Matrix4x4 { matrix: _qt_node96.transformMatrix }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node99_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node99_transform_animation
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
                                script:_qt_node99_transform_0_2.xScale = 0.3
                            }
                            ScriptAction {
                                script:_qt_node99_transform_0_2.yScale = 0.3
                            }
                        }
                        PauseAnimation { duration: 1833 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 167
                                target: _qt_node99_transform_0_2
                                property: "xScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 167
                                target: _qt_node99_transform_0_2
                                property: "yScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                        }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 667
                                target: _qt_node99_transform_0_2
                                property: "xScale"
                                to: 0.3
                                easing: _qt_node0.easing_04
                            }
                            PropertyAnimation {
                                duration: 667
                                target: _qt_node99_transform_0_2
                                property: "yScale"
                                to: 0.3
                                easing: _qt_node0.easing_04
                            }
                        }
                        PauseAnimation { duration: 2300 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node99_transform_base_group.deactivateOverride(_qt_node99_transform_group_0)
                    }
                }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node99_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node99_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node99.opacity = 0
                    }
                    PauseAnimation { duration: 1833 }
                    PropertyAnimation {
                        duration: 167
                        target: _qt_node99
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PropertyAnimation {
                        duration: 667
                        target: _qt_node99
                        property: "opacity"
                        to: 0
                        easing: _qt_node0.easing_04
                    }
                    PauseAnimation { duration: 2300 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node100
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node101_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -11.413 -3.724 Q -11.8631 -3.82602 -12.2534 -3.57938 Q -12.6438 -3.33267 -12.745 -2.882 Q -12.8463 -2.43125 -12.6001 -2.04113 Q -12.354 -1.651 -11.904 -1.549 L -10.9021 -1.32243 L -8.26072 -0.725094 L -0.2455 1.0875 L 11.413 3.724 Q 11.8631 3.82602 12.2534 3.57938 Q 12.6438 3.33267 12.745 2.882 Q 12.8463 2.43125 12.6001 2.04113 Q 12.354 1.651 11.904 1.549 L 10.9021 1.32243 L 8.26072 0.725094 L 0.2455 -1.0875 L -11.413 -3.724 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node102
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node102_transform_base_group
            TransformGroup {
                id: _qt_node102_transform_group_0
                Scale { id: _qt_node102_transform_0_2}
                Translate { x: 27.627; y: 39.076 }
            }
            Matrix4x4 { matrix: _qt_node96.transformMatrix }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node102_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node102_transform_animation
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
                                script:_qt_node102_transform_0_2.xScale = 0.3
                            }
                            ScriptAction {
                                script:_qt_node102_transform_0_2.yScale = 0.3
                            }
                        }
                        PauseAnimation { duration: 1833 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 167
                                target: _qt_node102_transform_0_2
                                property: "xScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 167
                                target: _qt_node102_transform_0_2
                                property: "yScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                        }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 667
                                target: _qt_node102_transform_0_2
                                property: "xScale"
                                to: 0.3
                                easing: _qt_node0.easing_04
                            }
                            PropertyAnimation {
                                duration: 667
                                target: _qt_node102_transform_0_2
                                property: "yScale"
                                to: 0.3
                                easing: _qt_node0.easing_04
                            }
                        }
                        PauseAnimation { duration: 2300 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node102_transform_base_group.deactivateOverride(_qt_node102_transform_group_0)
                    }
                }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node102_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node102_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node102.opacity = 0
                    }
                    PauseAnimation { duration: 1833 }
                    PropertyAnimation {
                        duration: 167
                        target: _qt_node102
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PropertyAnimation {
                        duration: 667
                        target: _qt_node102
                        property: "opacity"
                        to: 0
                        easing: _qt_node0.easing_04
                    }
                    PauseAnimation { duration: 2300 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node103
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node104_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 11.744 -2.489 Q 12.1844 -2.6263 12.3993 -3.03513 Q 12.6142 -3.44417 12.477 -3.885 Q 12.3398 -4.32598 11.9315 -4.54063 Q 11.5233 -4.75526 11.083 -4.618 L -0.3305 -1.0645 L -8.17728 1.37853 L -10.7632 2.18362 L -11.744 2.489 Q -12.1844 2.6263 -12.3993 3.03513 Q -12.6142 3.44417 -12.477 3.885 Q -12.3398 4.32598 -11.9315 4.54063 Q -11.5233 4.75526 -11.083 4.618 L 0.3305 1.0645 L 8.17728 -1.37853 L 10.7632 -2.18362 L 11.744 -2.489 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector (Stroke)"
        id: _qt_node105
        transformOrigin: Item.TopLeft
        opacity: 0
        transform: TransformGroup {
            id: _qt_node105_transform_base_group
            TransformGroup {
                id: _qt_node105_transform_group_0
                Scale { id: _qt_node105_transform_0_2}
                Translate { x: 9.22; y: 9.902 }
            }
            Matrix4x4 { matrix: _qt_node96.transformMatrix }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node105_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node105_transform_animation
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
                                script:_qt_node105_transform_0_2.xScale = 0.3
                            }
                            ScriptAction {
                                script:_qt_node105_transform_0_2.yScale = 0.3
                            }
                        }
                        PauseAnimation { duration: 1833 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 167
                                target: _qt_node105_transform_0_2
                                property: "xScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 167
                                target: _qt_node105_transform_0_2
                                property: "yScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                        }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 667
                                target: _qt_node105_transform_0_2
                                property: "xScale"
                                to: 0.3
                                easing: _qt_node0.easing_04
                            }
                            PropertyAnimation {
                                duration: 667
                                target: _qt_node105_transform_0_2
                                property: "yScale"
                                to: 0.3
                                easing: _qt_node0.easing_04
                            }
                        }
                        PauseAnimation { duration: 2300 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node105_transform_base_group.deactivateOverride(_qt_node105_transform_group_0)
                    }
                }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node105_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node105_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node105.opacity = 0
                    }
                    PauseAnimation { duration: 1833 }
                    PropertyAnimation {
                        duration: 167
                        target: _qt_node105
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PropertyAnimation {
                        duration: 667
                        target: _qt_node105
                        property: "opacity"
                        to: 0
                        easing: _qt_node0.easing_04
                    }
                    PauseAnimation { duration: 2300 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector (Stroke)"
            id: _qt_node106
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node107_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 8.925 -8.031 Q 9.23778 -8.37004 9.21937 -8.83125 Q 9.20096 -9.29253 8.862 -9.606 Q 8.52305 -9.91946 8.06238 -9.90125 Q 7.60179 -9.88305 7.289 -9.544 L -0.818 -0.7565 L -8.925 8.031 Q -9.23778 8.37004 -9.21937 8.83125 Q -9.20096 9.29253 -8.862 9.606 Q -8.52305 9.91946 -8.06238 9.90125 Q -7.60179 9.88305 -7.289 9.544 L 0.818 0.7565 L 8.925 -8.031 " }
            }
        }
    }
    LayerItem {
        objectName: " Click"
        id: _qt_node96
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node96_transform_base_group
            Translate { x: 412.13; y: 183.792}
            Matrix4x4 { matrix: _qt_node22.transformMatrix }
        }
    }
    LayerItem {
        objectName: " Lock"
        id: _qt_node22
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node22_transform_base_group
            Translate { x: 286; y: 55.319}
            Matrix4x4 { matrix: _qt_node20.transformMatrix }
        }
    }
    LayerItem {
        objectName: "Clould Medium"
        id: _qt_node108
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node108_transform_base_group
            TransformGroup {
                id: _qt_node108_transform_group_0
            }
            Translate {
                x: _qt_node108_motion_animation.currentInterpolator.x
                y: _qt_node108_motion_animation.currentInterpolator.y
            }
            Matrix4x4 { matrix: _qt_node20.transformMatrix }
        }
        QtObject {
            id: _qt_node108_pathInterpolator_0
            property real x: 652.937
            property real y: 31.111
            property real angle: -180
        }
        PathInterpolator {
            id: _qt_node108_pathInterpolator_1
            path: Path { PathSvg { path: "M 652.937 31.111 C 648.77 31.111 627.937 31.111 627.937 31.111 " } }
        }
        PathInterpolator {
            id: _qt_node108_pathInterpolator_2
            path: Path { PathSvg { path: "M 627.937 31.111 C 627.937 31.111 648.77 31.111 652.937 31.111 " } }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node108_motion_animation.restart() } }
        SequentialAnimation {
            id: _qt_node108_motion_animation
            property var currentInterpolator: _qt_node108_pathInterpolator_0
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            ScriptAction {
                script: {
                    _qt_node108_motion_animation.currentInterpolator = _qt_node108_pathInterpolator_1
                }
            }
            PropertyAnimation {
                id: _qt_node108_motionAnimation_1
                duration: 2000
                target: _qt_node108_pathInterpolator_1
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_01
            }
            ScriptAction {
                script: {
                    _qt_node108_motion_animation.currentInterpolator = _qt_node108_pathInterpolator_2
                }
            }
            PropertyAnimation {
                id: _qt_node108_motionAnimation_2
                duration: 2967
                target: _qt_node108_pathInterpolator_2
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_01
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Clould Medium"
            id: _qt_node109
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node110_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff6a3de6"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -97.255 31.111 L 97.255 31.111 Q 97.255 19.2072 88.7744 10.7607 Q 80.2804 2.301 68.275 2.301 Q 57.0611 2.301 48.841 9.973 Q 43.5327 -8.02454 28.4697 -19.455 Q 13.1096 -31.111 -6.222 -31.111 Q -22.0907 -31.111 -35.6294 -23.0351 Q -48.8047 -15.176 -56.342 -1.79 L -57.876 -1.79 Q -72.6076 -1.79 -83.7665 7.6285 Q -94.8087 16.9486 -97.255 31.111 " }
            }
        }
    }
    LayerItem {
        objectName: "Cloud Small"
        id: _qt_node111
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node111_transform_base_group
            TransformGroup {
                id: _qt_node111_transform_group_0
                Translate { id: _qt_node111_transform_0_0 }
            }
            Matrix4x4 { matrix: _qt_node20.transformMatrix }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node111_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node111_transform_animation
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
                                script:_qt_node111_transform_0_0.x = 797.244
                            }
                            ScriptAction {
                                script:_qt_node111_transform_0_0.y = 91.287
                            }
                        }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 3000
                                target: _qt_node111_transform_0_0
                                property: "x"
                                to: 837.244
                                easing: _qt_node0.easing_01
                            }
                            PropertyAnimation {
                                duration: 3000
                                target: _qt_node111_transform_0_0
                                property: "y"
                                to: 101.287
                                easing: _qt_node0.easing_01
                            }
                        }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 1967
                                target: _qt_node111_transform_0_0
                                property: "x"
                                to: 797.244
                                easing: _qt_node0.easing_01
                            }
                            PropertyAnimation {
                                duration: 1967
                                target: _qt_node111_transform_0_0
                                property: "y"
                                to: 91.287
                                easing: _qt_node0.easing_01
                            }
                        }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node111_transform_base_group.deactivateOverride(_qt_node111_transform_group_0)
                    }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Cloud Small"
            id: _qt_node112
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node113_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff6a3de6"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -60.348 19.349 L 60.348 19.349 Q 60.348 12.0745 55.1269 6.81887 Q 49.9191 1.57673 42.618 1.449 Q 39.2543 1.449 36.055 2.74875 Q 33.0179 3.98259 30.515 6.222 Q 27.1521 -5.03126 17.7936 -12.1251 Q 8.26359 -19.349 -3.75 -19.349 Q -13.6817 -19.349 -22.0972 -14.3197 Q -30.2589 -9.44221 -34.947 -1.108 L -35.97 -1.108 Q -44.9894 -1.108 -51.9308 4.7735 Q -58.8032 10.5966 -60.348 19.349 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node114
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node114_transform_base_group
            TransformGroup {
                id: _qt_node114_transform_group_0
            }
            Translate {
                x: _qt_node114_motion_animation.currentInterpolator.x
                y: _qt_node114_motion_animation.currentInterpolator.y
            }
            Matrix4x4 { matrix: _qt_node115.transformMatrix }
        }
        QtObject {
            id: _qt_node114_pathInterpolator_0
            property real x: 202.811
            property real y: 41.357
            property real angle: 0
        }
        PathInterpolator {
            id: _qt_node114_pathInterpolator_1
            path: Path { PathSvg { path: "M 202.811 41.357 C 209.811 41.357 244.811 41.357 244.811 41.357 " } }
        }
        PathInterpolator {
            id: _qt_node114_pathInterpolator_2
            path: Path { PathSvg { path: "M 244.811 41.357 C 244.811 41.357 209.811 41.357 202.811 41.357 " } }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node114_motion_animation.restart() } }
        SequentialAnimation {
            id: _qt_node114_motion_animation
            property var currentInterpolator: _qt_node114_pathInterpolator_0
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            ScriptAction {
                script: {
                    _qt_node114_motion_animation.currentInterpolator = _qt_node114_pathInterpolator_1
                }
            }
            PropertyAnimation {
                id: _qt_node114_motionAnimation_1
                duration: 2867
                target: _qt_node114_pathInterpolator_1
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_01
            }
            ScriptAction {
                script: {
                    _qt_node114_motion_animation.currentInterpolator = _qt_node114_pathInterpolator_2
                }
            }
            PropertyAnimation {
                id: _qt_node114_motionAnimation_2
                duration: 2133
                target: _qt_node114_pathInterpolator_2
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_01
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node116
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node117_path_fill_stroke
                objectName: "svg_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff6a3de6"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -129.017 41.357 L 129.017 41.357 Q 129.017 25.5565 117.806 14.3125 Q 106.608 3.081 90.904 3.081 Q 75.7853 3.081 64.955 13.137 Q 57.8113 -10.7569 37.8092 -25.909 Q 17.4166 -41.357 -8.19 -41.357 Q -29.2311 -41.357 -47.2156 -30.592 Q -64.645 -20.1593 -74.686 -2.433 L -75.8008 -2.433 L -76.794 -2.433 Q -96.2213 -2.433 -111.055 10.1567 Q -125.705 22.5901 -129.017 41.357 " }
            }
        }
    }
    LayerItem {
        objectName: "Vector"
        id: _qt_node118
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node118_transform_base_group
            TransformGroup {
                id: _qt_node118_transform_group_0
            }
            Translate {
                x: _qt_node118_motion_animation.currentInterpolator.x
                y: _qt_node118_motion_animation.currentInterpolator.y
            }
            Matrix4x4 { matrix: _qt_node115.transformMatrix }
        }
        QtObject {
            id: _qt_node118_pathInterpolator_0
            property real x: 131.5
            property real y: 61.594
            property real angle: -180
        }
        PathInterpolator {
            id: _qt_node118_pathInterpolator_1
            path: Path { PathSvg { path: "M 131.5 61.594 C 127.333 61.594 106.5 61.594 106.5 61.594 " } }
        }
        PathInterpolator {
            id: _qt_node118_pathInterpolator_2
            path: Path { PathSvg { path: "M 106.5 61.594 C 106.5 61.594 127.333 61.594 131.5 61.594 " } }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node118_motion_animation.restart() } }
        SequentialAnimation {
            id: _qt_node118_motion_animation
            property var currentInterpolator: _qt_node118_pathInterpolator_0
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            ScriptAction {
                script: {
                    _qt_node118_motion_animation.currentInterpolator = _qt_node118_pathInterpolator_1
                }
            }
            PropertyAnimation {
                id: _qt_node118_motionAnimation_1
                duration: 2833
                target: _qt_node118_pathInterpolator_1
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_01
            }
            ScriptAction {
                script: {
                    _qt_node118_motion_animation.currentInterpolator = _qt_node118_pathInterpolator_2
                }
            }
            PropertyAnimation {
                id: _qt_node118_motionAnimation_2
                duration: 2134
                target: _qt_node118_pathInterpolator_2
                property: "progress"
                from: 0; to: 1
                easing: _qt_node0.easing_01
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Vector"
            id: _qt_node119
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node120_path_fill
                objectName: "svg_fill_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ffffffff"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M -131.5 43.5 L -131.424 43.0705 L -131.346 42.6302 L -131.257 42.1255 L -131.014 40.751 Q -129.251 30.8651 -124.523 22.7138 Q -119.795 14.5625 -112.104 8.14587 Q -104.411 1.72794 -95.5179 -1.48103 Q -86.6252 -4.69 -76.533 -4.69 L -75.884 -4.69 Q -70.5846 -13.6805 -63.4046 -20.7755 Q -56.2245 -27.8705 -47.1637 -33.0699 Q -38.0757 -38.2849 -28.3115 -40.8925 Q -18.5473 -43.5 -8.107 -43.5 Q 16.8911 -43.5 37.4757 -28.8452 Q 57.9894 -14.241 66.155 9.217 Q 71.61 5.013 77.7715 2.911 Q 83.933 0.809 90.801 0.809 Q 99.1921 0.809 106.357 3.77025 Q 113.522 6.7315 119.461 12.654 Q 131.338 24.4991 131.338 41.236 L 131.338 43.5 L -131.5 43.5 M 126.798 38.81 Q 126.371 31.8191 123.564 25.8914 Q 120.757 19.9637 115.57 15.0991 Q 105.158 5.336 90.964 5.336 Q 76.964 5.336 66.642 14.877 L 65.2635 16.09 L 63.885 17.303 L 63.3175 15.524 L 62.9273 14.3009 L 62.75 13.745 Q 59.2841 2.1015 52.5957 -7.39888 Q 45.9073 -16.8993 35.9964 -24.2565 Q 26.0848 -31.6143 15.0184 -35.2931 Q 3.9521 -38.972 -8.269 -38.972 Q -28.3337 -38.972 -45.6435 -28.8652 Q -62.9521 -18.7592 -72.803 -1.294 L -73.452 0 L -74.911 0 L -76.857 0 Q -94.4547 -0.121255 -108.232 10.7939 Q -121.966 21.6744 -125.987 38.81 L 126.798 38.81 " }
            }
        }
    }
    LayerItem {
        objectName: " Cloud Big"
        id: _qt_node115
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node115_transform_base_group
            Translate { x: 0; y: 373.318}
            Matrix4x4 { matrix: _qt_node20.transformMatrix }
        }
    }
    LayerItem {
        objectName: "Logo"
        id: _qt_node121
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node121_transform_base_group
            TransformGroup {
                id: _qt_node121_transform_group_0
                Scale { id: _qt_node121_transform_0_2}
                Translate { x: 495.5; y: 260.318 }
            }
            Matrix4x4 { matrix: _qt_node20.transformMatrix }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node121_transform_animation.restart() } }
        ParallelAnimation {
            id:_qt_node121_transform_animation
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
                                script:_qt_node121_transform_0_2.xScale = 1
                            }
                            ScriptAction {
                                script:_qt_node121_transform_0_2.yScale = 1
                            }
                        }
                        PauseAnimation { duration: 500 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 333
                                target: _qt_node121_transform_0_2
                                property: "xScale"
                                to: 0.3
                                easing: _qt_node0.easing_05
                            }
                            PropertyAnimation {
                                duration: 333
                                target: _qt_node121_transform_0_2
                                property: "yScale"
                                to: 0.3
                                easing: _qt_node0.easing_05
                            }
                        }
                        PauseAnimation { duration: 3500 }
                        ParallelAnimation {
                            PropertyAnimation {
                                duration: 334
                                target: _qt_node121_transform_0_2
                                property: "xScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                            PropertyAnimation {
                                duration: 334
                                target: _qt_node121_transform_0_2
                                property: "yScale"
                                to: 1
                                easing: _qt_node0.easing_00
                            }
                        }
                        PauseAnimation { duration: 300 }
                    }
                }
                ScriptAction {
                    script: {
                        _qt_node121_transform_base_group.deactivateOverride(_qt_node121_transform_group_0)
                    }
                }
            }
        }
        Connections { target: _qt_node0.animations; function onRestart() {_qt_node121_opacity_animation.restart() } }
        ParallelAnimation {
            id: _qt_node121_opacity_animation
            loops: _qt_node0.animations.loops
            paused: _qt_node0.animations.paused
            running: true
            onLoopsChanged: { if (running) { restart() } }
            SequentialAnimation {
                SequentialAnimation {
                    loops: 1
                    ScriptAction {
                        script:_qt_node121.opacity = 1
                    }
                    PauseAnimation { duration: 500 }
                    PropertyAnimation {
                        duration: 333
                        target: _qt_node121
                        property: "opacity"
                        to: 0
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 3500 }
                    PropertyAnimation {
                        duration: 334
                        target: _qt_node121
                        property: "opacity"
                        to: 1
                        easing: _qt_node0.easing_00
                    }
                    PauseAnimation { duration: 300 }
                }
            }
        }
        Shape {
            preferredRendererType: Shape.CurveRenderer
            objectName: "Logo"
            id: _qt_node122
            transformOrigin: Item.TopLeft
            ShapePath {
                id: _qt_node123_path_fill
                objectName: "svg_fill_path:Path 1"
                strokeColor: "transparent"
                fillColor: "#ff000000"
                fillRule: ShapePath.WindingFill
                pathHints: ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
                PathSvg { path: "M 31.193 -11.616 Q 34.7233 -9.29262 36.371 -5.424 Q 37.4803 -2.83769 37.5 -0.021 Q 37.4828 5.75141 33.366 9.837 Q 29.2505 13.9213 23.429 13.94 Q 19.6759 13.9476 16.42 12.1 L 16.401 12.1 L 12.8715 15.601 L 9.342 19.102 Q 9.5426 19.4565 9.739 19.8534 Q 9.92407 20.2273 10.103 20.634 Q 11.2099 23.231 11.216 26.053 Q 11.2084 29.9228 9.199 33.233 Q 7.1875 36.5468 3.746 38.363 Q 0.304498 40.1792 -3.591 39.982 Q -7.48352 39.7849 -10.722 37.629 Q -14.1794 35.3036 -15.82 31.494 Q -16.9416 28.8806 -16.934 26.049 Q -16.9385 23.2721 -15.868 20.707 Q -14.7973 18.1416 -12.816 16.18 Q -10.8338 14.2175 -8.247 13.161 Q -5.65637 12.1029 -2.859 12.112 Q 0.76246 12.1013 3.934 13.835 L 11.095 6.732 Q 10.741 6.10728 10.452 5.437 Q 10.0937 4.6206 9.867 3.772 L -9.902 3.772 Q -10.4872 5.94781 -11.777 7.798 Q -13.737 10.681 -16.83 12.315 Q -19.9227 13.9489 -23.429 13.955 Q -27.7303 13.9657 -31.292 11.579 Q -34.7556 9.25666 -36.39 5.441 Q -37.5091 2.82979 -37.5 -0.005 Q -37.503 -2.78029 -36.432 -5.345 Q -35.362 -7.90715 -33.381 -9.87 Q -31.4019 -11.8309 -28.814 -12.89 Q -26.2286 -13.9481 -23.429 -13.942 Q -19.8 -13.9557 -16.62 -12.227 L -9.467 -19.322 Q -9.82669 -19.9541 -10.114 -20.617 Q -11.2331 -23.2282 -11.224 -26.063 Q -11.227 -28.8383 -10.156 -31.403 Q -9.08604 -33.9651 -7.105 -35.928 Q -5.12595 -37.8889 -2.538 -38.948 Q 0.0473673 -40.0061 2.847 -40 Q 7.16588 -40.0138 10.742 -37.619 Q 13.6468 -35.6763 15.295 -32.607 Q 16.943 -29.538 16.946 -26.063 Q 16.946 -21.8544 14.598 -18.347 Q 12.2506 -14.8404 8.338 -13.207 Q 5.70458 -12.0899 2.847 -12.102 Q -0.911473 -12.0852 -4.177 -13.934 L -7.7065 -10.4335 L -11.236 -6.933 Q -10.7734 -6.21269 -10.451 -5.424 Q -10.0971 -4.62072 -9.887 -3.759 L 9.902 -3.759 Q 10.4903 -5.93108 11.754 -7.804 Q 13.712 -10.6961 16.814 -12.327 Q 19.9147 -13.9572 23.429 -13.942 Q 27.6634 -13.9389 31.193 -11.616 M -0.761 -31.388 Q -2.35681 -30.3206 -3.114 -28.564 Q -3.7482 -27.0624 -3.596 -25.443 Q -3.44374 -23.8229 -2.541 -22.465 Q -1.63419 -21.1299 -0.205 -20.375 Q 1.22712 -19.6185 2.847 -19.617 Q 4.84197 -19.6047 6.491 -20.722 Q 8.09146 -21.789 8.844 -23.55 Q 9.36059 -24.7594 9.35 -26.067 Q 9.3546 -28.007 8.273 -29.624 Q 7.19093 -31.2417 5.389 -31.991 Q 3.87462 -32.6211 2.237 -32.461 Q 0.601848 -32.3011 -0.761 -31.388 M -19.577 5.195 Q -18.1351 4.14411 -17.44 2.508 L -17.44 2.512 Q -16.9175 1.30438 -16.922 -0.005 Q -16.9174 -1.94497 -17.999 -3.562 Q -19.0811 -5.17973 -20.883 -5.929 Q -22.3993 -6.55991 -24.035 -6.4 Q -25.6684 -6.24031 -27.033 -5.326 Q -28.6245 -4.25631 -29.386 -2.502 Q -30.0193 -1.00257 -29.867 0.616 Q -29.7148 2.23426 -28.813 3.593 Q -27.8165 5.06789 -26.199 5.827 Q -24.5806 6.58651 -22.8 6.417 Q -21.0214 6.24768 -19.577 5.195 M 0.761 31.397 Q 2.35717 30.3262 3.114 28.569 L 3.106 28.565 Q 3.73867 27.0618 3.582 25.445 Q 3.42497 23.8244 2.518 22.47 Q 1.4328 20.8843 -0.349 20.136 Q -2.3342 19.3114 -4.42 19.824 Q -6.50844 20.3372 -7.874 21.983 Q -9.24074 23.6302 -9.347 25.76 Q -9.45313 27.8872 -8.255 29.662 Q -7.16087 31.245 -5.385 31.996 Q -4.17473 32.4965 -2.867 32.498 Q -0.883588 32.4995 0.761 31.397 M 27.037 5.336 Q 28.6332 4.26525 29.39 2.508 L 29.386 2.512 Q 29.907 1.30794 29.907 -0.005 Q 29.9085 -1.94407 28.827 -3.561 Q 27.7454 -5.17799 25.943 -5.929 Q 24.4267 -6.55991 22.791 -6.4 Q 21.1576 -6.24031 19.793 -5.326 Q 18.1972 -4.25855 17.44 -2.502 Q 16.8051 -1.0022 16.959 0.618 Q 17.1128 2.23681 18.021 3.593 Q 18.9263 4.92807 20.357 5.683 Q 21.7869 6.43752 23.409 6.436 Q 25.3939 6.43754 27.037 5.336 " }
            }
        }
    }
    LayerItem {
        objectName: " Layer 1"
        id: _qt_node20
        transformOrigin: Item.TopLeft
        transform: TransformGroup {
            id: _qt_node20_transform_base_group
            Translate { x: 191; y: 114.682}
        }
    }
    readonly property easingCurve easing_04: ({ type: Easing.BezierSpline, bezierCurve: [ 0.17, 0, 0.32, 1, 1, 1 ] })
    readonly property easingCurve easing_05: ({ type: Easing.BezierSpline, bezierCurve: [ 0.17, 0, 0.32, 1.27, 1, 1 ] })
    readonly property easingCurve easing_02: ({ type: Easing.BezierSpline, bezierCurve: [ 0.17, 0.667, 0.32, 1.202, 1, 1 ] })
    readonly property easingCurve easing_00: ({ type: Easing.BezierSpline, bezierCurve: [ 0.17, 0.89, 0.32, 1.27, 1, 1 ] })
    readonly property easingCurve easing_01: ({ type: Easing.BezierSpline, bezierCurve: [ 0.42, 0, 0.58, 1, 1, 1 ] })
    readonly property easingCurve easing_03: ({ type: Easing.BezierSpline, bezierCurve: [ 0.6, -0.28, 0.73, 0.04, 1, 1 ] })
}
