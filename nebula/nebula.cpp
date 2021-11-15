#include "nebula.h"

constexpr auto QRC_ROOT = "qrc:///nebula/";

void Nebula::Initialize(QQmlEngine *engine){
    engine->addImportPath(QRC_ROOT);
}
