#ifndef GLEAN_H
#define GLEAN_H

#include <QQmlEngine>

class Glean
{
private:
    Glean(){};
public:
static void Initialize(QQmlEngine * engine);
};

#define INIT_GLEAN   Q_INIT_RESOURCE(glean);

#endif // GLEAN_H
