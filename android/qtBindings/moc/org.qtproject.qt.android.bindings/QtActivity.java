package org.qtproject.qt.android.bindings;

import android.app.Activity;

public class QtActivity extends Activity {
    // This thing should never be used in reality,
    // it's just a helper to tell gradle when invoked outside of qmake
    // that QtApplication is an Android Application -> thus a context
}
