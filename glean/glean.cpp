#include "glean.h"
#include <QQmlEngine>

constexpr auto QRC_ROOT = "qrc:///glean/";

void Glean::Initialize(QQmlEngine *engine){
    engine->addImportPath(QRC_ROOT);
}
