/****************************************************************************
** Meta object code from reading C++ file 'mozillavpn.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../src/mozillavpn.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mozillavpn.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MozillaVPN_t {
    QByteArrayData data[110];
    char stringdata0[1709];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MozillaVPN_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MozillaVPN_t qt_meta_stringdata_MozillaVPN = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MozillaVPN"
QT_MOC_LITERAL(1, 11, 12), // "stateChanged"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 12), // "alertChanged"
QT_MOC_LITERAL(4, 38, 24), // "updateRecommendedChanged"
QT_MOC_LITERAL(5, 63, 25), // "userAuthenticationChanged"
QT_MOC_LITERAL(6, 89, 14), // "deviceRemoving"
QT_MOC_LITERAL(7, 104, 9), // "publicKey"
QT_MOC_LITERAL(8, 114, 14), // "settingsNeeded"
QT_MOC_LITERAL(9, 129, 11), // "aboutNeeded"
QT_MOC_LITERAL(10, 141, 14), // "viewLogsNeeded"
QT_MOC_LITERAL(11, 156, 15), // "contactUsNeeded"
QT_MOC_LITERAL(12, 172, 15), // "updatingChanged"
QT_MOC_LITERAL(13, 188, 14), // "sendGleanPings"
QT_MOC_LITERAL(14, 203, 18), // "triggerGleanSample"
QT_MOC_LITERAL(15, 222, 15), // "gleanSampleName"
QT_MOC_LITERAL(16, 238, 11), // "aboutToQuit"
QT_MOC_LITERAL(17, 250, 29), // "loadAndroidAuthenticationView"
QT_MOC_LITERAL(18, 280, 9), // "logsReady"
QT_MOC_LITERAL(19, 290, 4), // "logs"
QT_MOC_LITERAL(20, 295, 18), // "currentViewChanged"
QT_MOC_LITERAL(21, 314, 20), // "ticketCreationAnswer"
QT_MOC_LITERAL(22, 335, 10), // "successful"
QT_MOC_LITERAL(23, 346, 15), // "requestSettings"
QT_MOC_LITERAL(24, 362, 12), // "requestAbout"
QT_MOC_LITERAL(25, 375, 15), // "requestViewLogs"
QT_MOC_LITERAL(26, 391, 16), // "requestContactUs"
QT_MOC_LITERAL(27, 408, 13), // "taskCompleted"
QT_MOC_LITERAL(28, 422, 10), // "getStarted"
QT_MOC_LITERAL(29, 433, 12), // "authenticate"
QT_MOC_LITERAL(30, 446, 20), // "cancelAuthentication"
QT_MOC_LITERAL(31, 467, 8), // "openLink"
QT_MOC_LITERAL(32, 476, 8), // "LinkType"
QT_MOC_LITERAL(33, 485, 8), // "linkType"
QT_MOC_LITERAL(34, 494, 25), // "removeDeviceFromPublicKey"
QT_MOC_LITERAL(35, 520, 9), // "hideAlert"
QT_MOC_LITERAL(36, 530, 26), // "hideUpdateRecommendedAlert"
QT_MOC_LITERAL(37, 557, 27), // "postAuthenticationCompleted"
QT_MOC_LITERAL(38, 585, 24), // "telemetryPolicyCompleted"
QT_MOC_LITERAL(39, 610, 8), // "viewLogs"
QT_MOC_LITERAL(40, 619, 12), // "retrieveLogs"
QT_MOC_LITERAL(41, 632, 11), // "cleanupLogs"
QT_MOC_LITERAL(42, 644, 16), // "storeInClipboard"
QT_MOC_LITERAL(43, 661, 4), // "text"
QT_MOC_LITERAL(44, 666, 8), // "activate"
QT_MOC_LITERAL(45, 675, 10), // "deactivate"
QT_MOC_LITERAL(46, 686, 14), // "refreshDevices"
QT_MOC_LITERAL(47, 701, 6), // "update"
QT_MOC_LITERAL(48, 708, 21), // "backendServiceRestore"
QT_MOC_LITERAL(49, 730, 16), // "triggerHeartbeat"
QT_MOC_LITERAL(50, 747, 14), // "submitFeedback"
QT_MOC_LITERAL(51, 762, 12), // "feedbackText"
QT_MOC_LITERAL(52, 775, 6), // "rating"
QT_MOC_LITERAL(53, 782, 8), // "category"
QT_MOC_LITERAL(54, 791, 22), // "openAppStoreReviewLink"
QT_MOC_LITERAL(55, 814, 19), // "createSupportTicket"
QT_MOC_LITERAL(56, 834, 5), // "email"
QT_MOC_LITERAL(57, 840, 7), // "subject"
QT_MOC_LITERAL(58, 848, 9), // "issueText"
QT_MOC_LITERAL(59, 858, 15), // "validateUserDNS"
QT_MOC_LITERAL(60, 874, 3), // "dns"
QT_MOC_LITERAL(61, 878, 5), // "reset"
QT_MOC_LITERAL(62, 884, 17), // "forceInitialState"
QT_MOC_LITERAL(63, 902, 5), // "state"
QT_MOC_LITERAL(64, 908, 5), // "State"
QT_MOC_LITERAL(65, 914, 5), // "alert"
QT_MOC_LITERAL(66, 920, 9), // "AlertType"
QT_MOC_LITERAL(67, 930, 13), // "versionString"
QT_MOC_LITERAL(68, 944, 11), // "buildNumber"
QT_MOC_LITERAL(69, 956, 17), // "updateRecommended"
QT_MOC_LITERAL(70, 974, 17), // "userAuthenticated"
QT_MOC_LITERAL(71, 992, 14), // "startMinimized"
QT_MOC_LITERAL(72, 1007, 8), // "updating"
QT_MOC_LITERAL(73, 1016, 11), // "stagingMode"
QT_MOC_LITERAL(74, 1028, 11), // "currentView"
QT_MOC_LITERAL(75, 1040, 19), // "StateAuthenticating"
QT_MOC_LITERAL(76, 1060, 19), // "StateBackendFailure"
QT_MOC_LITERAL(77, 1080, 24), // "StateBillingNotAvailable"
QT_MOC_LITERAL(78, 1105, 16), // "StateDeviceLimit"
QT_MOC_LITERAL(79, 1122, 15), // "StateInitialize"
QT_MOC_LITERAL(80, 1138, 9), // "StateMain"
QT_MOC_LITERAL(81, 1148, 23), // "StatePostAuthentication"
QT_MOC_LITERAL(82, 1172, 24), // "StateSubscriptionBlocked"
QT_MOC_LITERAL(83, 1197, 23), // "StateSubscriptionNeeded"
QT_MOC_LITERAL(84, 1221, 27), // "StateSubscriptionInProgress"
QT_MOC_LITERAL(85, 1249, 29), // "StateSubscriptionNotValidated"
QT_MOC_LITERAL(86, 1279, 20), // "StateTelemetryPolicy"
QT_MOC_LITERAL(87, 1300, 19), // "StateUpdateRequired"
QT_MOC_LITERAL(88, 1320, 7), // "NoAlert"
QT_MOC_LITERAL(89, 1328, 25), // "AuthenticationFailedAlert"
QT_MOC_LITERAL(90, 1354, 21), // "ConnectionFailedAlert"
QT_MOC_LITERAL(91, 1376, 11), // "LogoutAlert"
QT_MOC_LITERAL(92, 1388, 17), // "NoConnectionAlert"
QT_MOC_LITERAL(93, 1406, 20), // "ControllerErrorAlert"
QT_MOC_LITERAL(94, 1427, 23), // "RemoteServiceErrorAlert"
QT_MOC_LITERAL(95, 1451, 24), // "SubscriptionFailureAlert"
QT_MOC_LITERAL(96, 1476, 21), // "GeoIpRestrictionAlert"
QT_MOC_LITERAL(97, 1498, 23), // "UnrecoverableErrorAlert"
QT_MOC_LITERAL(98, 1522, 11), // "LinkAccount"
QT_MOC_LITERAL(99, 1534, 11), // "LinkContact"
QT_MOC_LITERAL(100, 1546, 12), // "LinkFeedback"
QT_MOC_LITERAL(101, 1559, 15), // "LinkLeaveReview"
QT_MOC_LITERAL(102, 1575, 11), // "LinkLicense"
QT_MOC_LITERAL(103, 1587, 15), // "LinkHelpSupport"
QT_MOC_LITERAL(104, 1603, 18), // "LinkTermsOfService"
QT_MOC_LITERAL(105, 1622, 17), // "LinkPrivacyNotice"
QT_MOC_LITERAL(106, 1640, 10), // "LinkUpdate"
QT_MOC_LITERAL(107, 1651, 13), // "LinkInspector"
QT_MOC_LITERAL(108, 1665, 23), // "LinkSubscriptionBlocked"
QT_MOC_LITERAL(109, 1689, 19) // "LinkSplitTunnelHelp"

    },
    "MozillaVPN\0stateChanged\0\0alertChanged\0"
    "updateRecommendedChanged\0"
    "userAuthenticationChanged\0deviceRemoving\0"
    "publicKey\0settingsNeeded\0aboutNeeded\0"
    "viewLogsNeeded\0contactUsNeeded\0"
    "updatingChanged\0sendGleanPings\0"
    "triggerGleanSample\0gleanSampleName\0"
    "aboutToQuit\0loadAndroidAuthenticationView\0"
    "logsReady\0logs\0currentViewChanged\0"
    "ticketCreationAnswer\0successful\0"
    "requestSettings\0requestAbout\0"
    "requestViewLogs\0requestContactUs\0"
    "taskCompleted\0getStarted\0authenticate\0"
    "cancelAuthentication\0openLink\0LinkType\0"
    "linkType\0removeDeviceFromPublicKey\0"
    "hideAlert\0hideUpdateRecommendedAlert\0"
    "postAuthenticationCompleted\0"
    "telemetryPolicyCompleted\0viewLogs\0"
    "retrieveLogs\0cleanupLogs\0storeInClipboard\0"
    "text\0activate\0deactivate\0refreshDevices\0"
    "update\0backendServiceRestore\0"
    "triggerHeartbeat\0submitFeedback\0"
    "feedbackText\0rating\0category\0"
    "openAppStoreReviewLink\0createSupportTicket\0"
    "email\0subject\0issueText\0validateUserDNS\0"
    "dns\0reset\0forceInitialState\0state\0"
    "State\0alert\0AlertType\0versionString\0"
    "buildNumber\0updateRecommended\0"
    "userAuthenticated\0startMinimized\0"
    "updating\0stagingMode\0currentView\0"
    "StateAuthenticating\0StateBackendFailure\0"
    "StateBillingNotAvailable\0StateDeviceLimit\0"
    "StateInitialize\0StateMain\0"
    "StatePostAuthentication\0"
    "StateSubscriptionBlocked\0"
    "StateSubscriptionNeeded\0"
    "StateSubscriptionInProgress\0"
    "StateSubscriptionNotValidated\0"
    "StateTelemetryPolicy\0StateUpdateRequired\0"
    "NoAlert\0AuthenticationFailedAlert\0"
    "ConnectionFailedAlert\0LogoutAlert\0"
    "NoConnectionAlert\0ControllerErrorAlert\0"
    "RemoteServiceErrorAlert\0"
    "SubscriptionFailureAlert\0GeoIpRestrictionAlert\0"
    "UnrecoverableErrorAlert\0LinkAccount\0"
    "LinkContact\0LinkFeedback\0LinkLeaveReview\0"
    "LinkLicense\0LinkHelpSupport\0"
    "LinkTermsOfService\0LinkPrivacyNotice\0"
    "LinkUpdate\0LinkInspector\0"
    "LinkSubscriptionBlocked\0LinkSplitTunnelHelp"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MozillaVPN[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      46,   14, // methods
      10,  322, // properties
       3,  362, // enums/sets
       0,    0, // constructors
       0,       // flags
      17,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,  244,    2, 0x06 /* Public */,
       3,    0,  245,    2, 0x06 /* Public */,
       4,    0,  246,    2, 0x06 /* Public */,
       5,    0,  247,    2, 0x06 /* Public */,
       6,    1,  248,    2, 0x06 /* Public */,
       8,    0,  251,    2, 0x06 /* Public */,
       9,    0,  252,    2, 0x06 /* Public */,
      10,    0,  253,    2, 0x06 /* Public */,
      11,    0,  254,    2, 0x06 /* Public */,
      12,    0,  255,    2, 0x06 /* Public */,
      13,    0,  256,    2, 0x06 /* Public */,
      14,    1,  257,    2, 0x06 /* Public */,
      16,    0,  260,    2, 0x06 /* Public */,
      17,    0,  261,    2, 0x06 /* Public */,
      18,    1,  262,    2, 0x06 /* Public */,
      20,    0,  265,    2, 0x06 /* Public */,
      21,    1,  266,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      23,    0,  269,    2, 0x0a /* Public */,
      24,    0,  270,    2, 0x0a /* Public */,
      25,    0,  271,    2, 0x0a /* Public */,
      26,    0,  272,    2, 0x0a /* Public */,
      27,    0,  273,    2, 0x08 /* Private */,

 // methods: name, argc, parameters, tag, flags
      28,    0,  274,    2, 0x02 /* Public */,
      29,    0,  275,    2, 0x02 /* Public */,
      30,    0,  276,    2, 0x02 /* Public */,
      31,    1,  277,    2, 0x02 /* Public */,
      34,    1,  280,    2, 0x02 /* Public */,
      35,    0,  283,    2, 0x02 /* Public */,
      36,    0,  284,    2, 0x02 /* Public */,
      37,    0,  285,    2, 0x02 /* Public */,
      38,    0,  286,    2, 0x02 /* Public */,
      39,    0,  287,    2, 0x02 /* Public */,
      40,    0,  288,    2, 0x02 /* Public */,
      41,    0,  289,    2, 0x02 /* Public */,
      42,    1,  290,    2, 0x02 /* Public */,
      44,    0,  293,    2, 0x02 /* Public */,
      45,    0,  294,    2, 0x02 /* Public */,
      46,    0,  295,    2, 0x02 /* Public */,
      47,    0,  296,    2, 0x02 /* Public */,
      48,    0,  297,    2, 0x02 /* Public */,
      49,    0,  298,    2, 0x02 /* Public */,
      50,    3,  299,    2, 0x02 /* Public */,
      54,    0,  306,    2, 0x02 /* Public */,
      55,    4,  307,    2, 0x02 /* Public */,
      59,    1,  316,    2, 0x02 /* Public */,
      61,    1,  319,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   15,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   19,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   22,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 32,   33,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   43,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::SChar, QMetaType::QString,   51,   52,   53,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString,   56,   57,   58,   53,
    QMetaType::Bool, QMetaType::QString,   60,
    QMetaType::Void, QMetaType::Bool,   62,

 // properties: name, type, flags
      63, 0x80000000 | 64, 0x00495009,
      65, 0x80000000 | 66, 0x00495009,
      67, QMetaType::QString, 0x00095401,
      68, QMetaType::QString, 0x00095401,
      69, QMetaType::Bool, 0x00495001,
      70, QMetaType::Bool, 0x00495001,
      71, QMetaType::Bool, 0x00095401,
      72, QMetaType::Bool, 0x00495001,
      73, QMetaType::Bool, 0x00095401,
      74, QMetaType::QString, 0x00495103,

 // properties: notify_signal_id
       0,
       1,
       0,
       0,
       2,
       3,
       0,
       9,
       0,
      15,

 // enums: name, alias, flags, count, data
      64,   64, 0x0,   13,  377,
      66,   66, 0x0,   10,  403,
      32,   32, 0x0,   12,  423,

 // enum data: key, value
      75, uint(MozillaVPN::StateAuthenticating),
      76, uint(MozillaVPN::StateBackendFailure),
      77, uint(MozillaVPN::StateBillingNotAvailable),
      78, uint(MozillaVPN::StateDeviceLimit),
      79, uint(MozillaVPN::StateInitialize),
      80, uint(MozillaVPN::StateMain),
      81, uint(MozillaVPN::StatePostAuthentication),
      82, uint(MozillaVPN::StateSubscriptionBlocked),
      83, uint(MozillaVPN::StateSubscriptionNeeded),
      84, uint(MozillaVPN::StateSubscriptionInProgress),
      85, uint(MozillaVPN::StateSubscriptionNotValidated),
      86, uint(MozillaVPN::StateTelemetryPolicy),
      87, uint(MozillaVPN::StateUpdateRequired),
      88, uint(MozillaVPN::NoAlert),
      89, uint(MozillaVPN::AuthenticationFailedAlert),
      90, uint(MozillaVPN::ConnectionFailedAlert),
      91, uint(MozillaVPN::LogoutAlert),
      92, uint(MozillaVPN::NoConnectionAlert),
      93, uint(MozillaVPN::ControllerErrorAlert),
      94, uint(MozillaVPN::RemoteServiceErrorAlert),
      95, uint(MozillaVPN::SubscriptionFailureAlert),
      96, uint(MozillaVPN::GeoIpRestrictionAlert),
      97, uint(MozillaVPN::UnrecoverableErrorAlert),
      98, uint(MozillaVPN::LinkAccount),
      99, uint(MozillaVPN::LinkContact),
     100, uint(MozillaVPN::LinkFeedback),
     101, uint(MozillaVPN::LinkLeaveReview),
     102, uint(MozillaVPN::LinkLicense),
     103, uint(MozillaVPN::LinkHelpSupport),
     104, uint(MozillaVPN::LinkTermsOfService),
     105, uint(MozillaVPN::LinkPrivacyNotice),
     106, uint(MozillaVPN::LinkUpdate),
     107, uint(MozillaVPN::LinkInspector),
     108, uint(MozillaVPN::LinkSubscriptionBlocked),
     109, uint(MozillaVPN::LinkSplitTunnelHelp),

       0        // eod
};

void MozillaVPN::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MozillaVPN *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->stateChanged(); break;
        case 1: _t->alertChanged(); break;
        case 2: _t->updateRecommendedChanged(); break;
        case 3: _t->userAuthenticationChanged(); break;
        case 4: _t->deviceRemoving((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->settingsNeeded(); break;
        case 6: _t->aboutNeeded(); break;
        case 7: _t->viewLogsNeeded(); break;
        case 8: _t->contactUsNeeded(); break;
        case 9: _t->updatingChanged(); break;
        case 10: _t->sendGleanPings(); break;
        case 11: _t->triggerGleanSample((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 12: _t->aboutToQuit(); break;
        case 13: _t->loadAndroidAuthenticationView(); break;
        case 14: _t->logsReady((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 15: _t->currentViewChanged(); break;
        case 16: _t->ticketCreationAnswer((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 17: _t->requestSettings(); break;
        case 18: _t->requestAbout(); break;
        case 19: _t->requestViewLogs(); break;
        case 20: _t->requestContactUs(); break;
        case 21: _t->taskCompleted(); break;
        case 22: _t->getStarted(); break;
        case 23: _t->authenticate(); break;
        case 24: _t->cancelAuthentication(); break;
        case 25: _t->openLink((*reinterpret_cast< LinkType(*)>(_a[1]))); break;
        case 26: _t->removeDeviceFromPublicKey((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 27: _t->hideAlert(); break;
        case 28: _t->hideUpdateRecommendedAlert(); break;
        case 29: _t->postAuthenticationCompleted(); break;
        case 30: _t->telemetryPolicyCompleted(); break;
        case 31: _t->viewLogs(); break;
        case 32: _t->retrieveLogs(); break;
        case 33: _t->cleanupLogs(); break;
        case 34: _t->storeInClipboard((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 35: _t->activate(); break;
        case 36: _t->deactivate(); break;
        case 37: _t->refreshDevices(); break;
        case 38: _t->update(); break;
        case 39: _t->backendServiceRestore(); break;
        case 40: _t->triggerHeartbeat(); break;
        case 41: _t->submitFeedback((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const qint8(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 42: _t->openAppStoreReviewLink(); break;
        case 43: _t->createSupportTicket((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4]))); break;
        case 44: { bool _r = _t->validateUserDNS((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 45: _t->reset((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MozillaVPN::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MozillaVPN::stateChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MozillaVPN::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MozillaVPN::alertChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (MozillaVPN::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MozillaVPN::updateRecommendedChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (MozillaVPN::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MozillaVPN::userAuthenticationChanged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (MozillaVPN::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MozillaVPN::deviceRemoving)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (MozillaVPN::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MozillaVPN::settingsNeeded)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (MozillaVPN::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MozillaVPN::aboutNeeded)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (MozillaVPN::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MozillaVPN::viewLogsNeeded)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (MozillaVPN::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MozillaVPN::contactUsNeeded)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (MozillaVPN::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MozillaVPN::updatingChanged)) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (MozillaVPN::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MozillaVPN::sendGleanPings)) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (MozillaVPN::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MozillaVPN::triggerGleanSample)) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (MozillaVPN::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MozillaVPN::aboutToQuit)) {
                *result = 12;
                return;
            }
        }
        {
            using _t = void (MozillaVPN::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MozillaVPN::loadAndroidAuthenticationView)) {
                *result = 13;
                return;
            }
        }
        {
            using _t = void (MozillaVPN::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MozillaVPN::logsReady)) {
                *result = 14;
                return;
            }
        }
        {
            using _t = void (MozillaVPN::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MozillaVPN::currentViewChanged)) {
                *result = 15;
                return;
            }
        }
        {
            using _t = void (MozillaVPN::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MozillaVPN::ticketCreationAnswer)) {
                *result = 16;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<MozillaVPN *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< State*>(_v) = _t->state(); break;
        case 1: *reinterpret_cast< AlertType*>(_v) = _t->alert(); break;
        case 2: *reinterpret_cast< QString*>(_v) = _t->versionString(); break;
        case 3: *reinterpret_cast< QString*>(_v) = _t->buildNumber(); break;
        case 4: *reinterpret_cast< bool*>(_v) = _t->updateRecommended(); break;
        case 5: *reinterpret_cast< bool*>(_v) = _t->userAuthenticated(); break;
        case 6: *reinterpret_cast< bool*>(_v) = _t->startMinimized(); break;
        case 7: *reinterpret_cast< bool*>(_v) = _t->updating(); break;
        case 8: *reinterpret_cast< bool*>(_v) = _t->stagingMode(); break;
        case 9: *reinterpret_cast< QString*>(_v) = _t->currentView(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<MozillaVPN *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 9: _t->setCurrentView(*reinterpret_cast< QString*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject MozillaVPN::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_MozillaVPN.data,
    qt_meta_data_MozillaVPN,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MozillaVPN::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MozillaVPN::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MozillaVPN.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int MozillaVPN::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 46)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 46;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 46)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 46;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 10;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 10;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 10;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 10;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 10;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void MozillaVPN::stateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void MozillaVPN::alertChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void MozillaVPN::updateRecommendedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void MozillaVPN::userAuthenticationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void MozillaVPN::deviceRemoving(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void MozillaVPN::settingsNeeded()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void MozillaVPN::aboutNeeded()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void MozillaVPN::viewLogsNeeded()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void MozillaVPN::contactUsNeeded()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void MozillaVPN::updatingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void MozillaVPN::sendGleanPings()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void MozillaVPN::triggerGleanSample(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void MozillaVPN::aboutToQuit()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void MozillaVPN::loadAndroidAuthenticationView()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void MozillaVPN::logsReady(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}

// SIGNAL 15
void MozillaVPN::currentViewChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 15, nullptr);
}

// SIGNAL 16
void MozillaVPN::ticketCreationAnswer(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 16, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
