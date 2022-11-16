/****************************************************************************
** Generated QML type registration code
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <QtQml/qqml.h>
#include <QtQml/qqmlmoduleregistration.h>

#include <filterproxymodel.h>
#include <lottieprivate.h>

#if !defined(QT_STATIC)
#define Q_QMLTYPE_EXPORT Q_DECL_EXPORT
#else
#define Q_QMLTYPE_EXPORT
#endif

Q_QMLTYPE_EXPORT void qml_register_types_Mozilla_VPN_qmlcomponents()
{
    qmlRegisterTypesAndRevisions<FilterProxyModel>("Mozilla.VPN.qmlcomponents", 1.0);
    qmlRegisterTypesAndRevisions<LottiePrivate>("Mozilla.VPN.qmlcomponents", 1.0);
    qmlRegisterModule("Mozilla.VPN.qmlcomponents", 1.0, 0);
}

static const QQmlModuleRegistration registration("Mozilla.VPN.qmlcomponents", qml_register_types_Mozilla_VPN_qmlcomponents);
