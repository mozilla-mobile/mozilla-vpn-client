#include "dbus.h"
#include "dbus_adaptor.h"

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    DBus *dbus = new DBus(&app);
    new DbusAdaptor(dbus);

    QDBusConnection connection = QDBusConnection::systemBus();
    qDebug() << "Connecting to DBus...";

    if (!connection.registerService("org.mozilla.vpn.dbus")
        || !connection.registerObject("/", dbus)) {
        qDebug() << "Connection failed:" << connection.lastError();
        app.exit(1);
        return 1;
    }

    qDebug() << "Ready!";
    return app.exec();
}
