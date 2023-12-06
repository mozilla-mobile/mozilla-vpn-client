/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.Shared 1.0

StackView {
  id: stackView
  // Let's force the visibility. See
  // https://doc.qt.io/qt-5/qml-qtquick-controls2-stackview.html#visible-attached-prop
  visible: true
  // StackView's items will provide Accessibility, so the StackView itself doesn't need Accessibility.
  Accessible.ignored: true

  property var screens: []
  property var currentLoadPolicy: null

  function showCurrentComponent() {
      if (stackView.currentLoadPolicy === MZNavigator.LoadTemporarily) {
        // Temporarily items will be always at the last position because
        // nothing else can be added after them.
        stackView.pop(StackView.Immediate);
      }

      stackView.currentLoadPolicy = MZNavigator.loadPolicy;

      if (MZNavigator.loadingFlags === MZNavigator.ForceReloadAll) {
        for (let i = 0; i < stackView.screens.length; ++i) {
          stackView.get(i+1).sourceComponent = null;
        }
      }

      // Let's hide the initial empty screen.
      stackView.get(0).visible = false;

      const pos = stackView.screens.indexOf(MZNavigator.screen)
      if (pos === -1) {
        for (let i = 0; i < stackView.screens.length; ++i) {
          stackView.get(i+1).visible = false;
        }

        if (stackView.currentLoadPolicy === MZNavigator.LoadPersistently) {
          stackView.screens.push(MZNavigator.screen);
        }

        const loaderComponent = Qt.createComponent("navigatorLoaderInternal.qml")
        stackView.push(loaderComponent, StackView.Immediate);

        return;
      }

      if (stackView.get(pos+1).sourceComponent === null ||
          (MZNavigator.loadingFlags === MZNavigator.ForceReload)) {
        stackView.get(pos+1).sourceComponent = null;
        stackView.get(pos+1).sourceComponent = MZNavigator.component;
      }

      for (let i = 0; i < stackView.screens.length; ++i) {
        stackView.get(i+1).visible = i === pos;

        if (i === pos) {
          // I really don't know why.
          // Follow up bug for proper fixing: https://mozilla-hub.atlassian.net/browse/VPN-2813
          // For some reason, once we navigate away the loader of the component
          // on android will get the opacity and X value set to garbage. 
          // This will cause the page broken, once we re-navigate here. 
          // So in case we're on android and this is the component we want to show
          // Let's reset those 2 values.
          stackView.get(i+1).opacity = 1;
          stackView.get(i+1).x = 0;

          // Set focus to current component
          stackView.get(i+1).forceActiveFocus();
        }
      }
  }

  Connections {
    target: MZNavigator
    function onCurrentComponentChanged() {
       showCurrentComponent();
    }
  }

  Component {
    id: initialScreen
    Item {}
  }

  Component.onCompleted: () => {
      // We need an empty screen because it can happen that we need to remove
      // even the initial screen to replace it with something else via
      // LoadTemporarily policy.
      stackView.push(initialScreen);

      showCurrentComponent()
  }

  onCurrentItemChanged: {
    currentItem.forceActiveFocus();
  }
}
