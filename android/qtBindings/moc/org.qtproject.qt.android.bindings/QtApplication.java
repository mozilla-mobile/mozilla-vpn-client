package  org.qtproject.qt.android.bindings;


import android.app.Application;

class QtApplication extends Application {
    QtApplication(){
        // This thing should never be used in reality,
        // it's just a helper to tell gradle when invoked outside of qmake
        // that QtApplication is an Android Application -> thus a context
        throw new RuntimeException();
    }
}