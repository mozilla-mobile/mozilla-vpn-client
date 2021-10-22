/****************************************************************************
** Meta object code from reading C++ file 'authenticationinapp.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../src/authenticationinapp/authenticationinapp.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'authenticationinapp.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AuthenticationInApp_t {
    QByteArrayData data[45];
    char stringdata0[813];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AuthenticationInApp_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AuthenticationInApp_t qt_meta_stringdata_AuthenticationInApp = {
    {
QT_MOC_LITERAL(0, 0, 19), // "AuthenticationInApp"
QT_MOC_LITERAL(1, 20, 12), // "stateChanged"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 13), // "errorOccurred"
QT_MOC_LITERAL(4, 48, 9), // "ErrorType"
QT_MOC_LITERAL(5, 58, 5), // "error"
QT_MOC_LITERAL(6, 64, 12), // "checkAccount"
QT_MOC_LITERAL(7, 77, 12), // "emailAddress"
QT_MOC_LITERAL(8, 90, 11), // "setPassword"
QT_MOC_LITERAL(9, 102, 8), // "password"
QT_MOC_LITERAL(10, 111, 20), // "validateEmailAddress"
QT_MOC_LITERAL(11, 132, 23), // "validatePasswordCommons"
QT_MOC_LITERAL(12, 156, 22), // "validatePasswordLength"
QT_MOC_LITERAL(13, 179, 21), // "validatePasswordEmail"
QT_MOC_LITERAL(14, 201, 5), // "reset"
QT_MOC_LITERAL(15, 207, 6), // "signIn"
QT_MOC_LITERAL(16, 214, 6), // "signUp"
QT_MOC_LITERAL(17, 221, 25), // "setUnblockCodeAndContinue"
QT_MOC_LITERAL(18, 247, 11), // "unblockCode"
QT_MOC_LITERAL(19, 259, 22), // "resendUnblockCodeEmail"
QT_MOC_LITERAL(20, 282, 22), // "verifySessionEmailCode"
QT_MOC_LITERAL(21, 305, 4), // "code"
QT_MOC_LITERAL(22, 310, 34), // "resendVerificationSessionCode..."
QT_MOC_LITERAL(23, 345, 21), // "verifySessionTotpCode"
QT_MOC_LITERAL(24, 367, 5), // "state"
QT_MOC_LITERAL(25, 373, 5), // "State"
QT_MOC_LITERAL(26, 379, 17), // "StateInitializing"
QT_MOC_LITERAL(27, 397, 10), // "StateStart"
QT_MOC_LITERAL(28, 408, 11), // "StateSignIn"
QT_MOC_LITERAL(29, 420, 11), // "StateSignUp"
QT_MOC_LITERAL(30, 432, 22), // "StateUnblockCodeNeeded"
QT_MOC_LITERAL(31, 455, 37), // "StateVerificationSessionByEma..."
QT_MOC_LITERAL(32, 493, 36), // "StateVerificationSessionByTot..."
QT_MOC_LITERAL(33, 530, 22), // "StateFallbackInBrowser"
QT_MOC_LITERAL(34, 553, 25), // "ErrorAccountAlreadyExists"
QT_MOC_LITERAL(35, 579, 19), // "ErrorUnknownAccount"
QT_MOC_LITERAL(36, 599, 22), // "ErrorIncorrectPassword"
QT_MOC_LITERAL(37, 622, 21), // "ErrorInvalidEmailCode"
QT_MOC_LITERAL(38, 644, 26), // "ErrorEmailTypeNotSupported"
QT_MOC_LITERAL(39, 671, 23), // "ErrorEmailAlreadyExists"
QT_MOC_LITERAL(40, 695, 29), // "ErrorEmailCanNotBeUsedToLogin"
QT_MOC_LITERAL(41, 725, 22), // "ErrorFailedToSendEmail"
QT_MOC_LITERAL(42, 748, 20), // "ErrorTooManyRequests"
QT_MOC_LITERAL(43, 769, 22), // "ErrorServerUnavailable"
QT_MOC_LITERAL(44, 792, 20) // "ErrorInvalidTotpCode"

    },
    "AuthenticationInApp\0stateChanged\0\0"
    "errorOccurred\0ErrorType\0error\0"
    "checkAccount\0emailAddress\0setPassword\0"
    "password\0validateEmailAddress\0"
    "validatePasswordCommons\0validatePasswordLength\0"
    "validatePasswordEmail\0reset\0signIn\0"
    "signUp\0setUnblockCodeAndContinue\0"
    "unblockCode\0resendUnblockCodeEmail\0"
    "verifySessionEmailCode\0code\0"
    "resendVerificationSessionCodeEmail\0"
    "verifySessionTotpCode\0state\0State\0"
    "StateInitializing\0StateStart\0StateSignIn\0"
    "StateSignUp\0StateUnblockCodeNeeded\0"
    "StateVerificationSessionByEmailNeeded\0"
    "StateVerificationSessionByTotpNeeded\0"
    "StateFallbackInBrowser\0ErrorAccountAlreadyExists\0"
    "ErrorUnknownAccount\0ErrorIncorrectPassword\0"
    "ErrorInvalidEmailCode\0ErrorEmailTypeNotSupported\0"
    "ErrorEmailAlreadyExists\0"
    "ErrorEmailCanNotBeUsedToLogin\0"
    "ErrorFailedToSendEmail\0ErrorTooManyRequests\0"
    "ErrorServerUnavailable\0ErrorInvalidTotpCode"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AuthenticationInApp[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       1,  130, // properties
       2,  134, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   94,    2, 0x06 /* Public */,
       3,    1,   95,    2, 0x06 /* Public */,

 // methods: name, argc, parameters, tag, flags
       6,    1,   98,    2, 0x02 /* Public */,
       8,    1,  101,    2, 0x02 /* Public */,
      10,    1,  104,    2, 0x02 /* Public */,
      11,    1,  107,    2, 0x02 /* Public */,
      12,    1,  110,    2, 0x02 /* Public */,
      13,    1,  113,    2, 0x02 /* Public */,
      14,    0,  116,    2, 0x02 /* Public */,
      15,    0,  117,    2, 0x02 /* Public */,
      16,    0,  118,    2, 0x02 /* Public */,
      17,    1,  119,    2, 0x02 /* Public */,
      19,    0,  122,    2, 0x02 /* Public */,
      20,    1,  123,    2, 0x02 /* Public */,
      22,    0,  126,    2, 0x02 /* Public */,
      23,    1,  127,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    5,

 // methods: parameters
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Bool, QMetaType::QString,    7,
    QMetaType::Bool, QMetaType::QString,    9,
    QMetaType::Bool, QMetaType::QString,    9,
    QMetaType::Bool, QMetaType::QString,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   18,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   21,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   21,

 // properties: name, type, flags
      24, 0x80000000 | 25, 0x00495009,

 // properties: notify_signal_id
       0,

 // enums: name, alias, flags, count, data
      25,   25, 0x0,    8,  144,
       4,    4, 0x0,   11,  160,

 // enum data: key, value
      26, uint(AuthenticationInApp::StateInitializing),
      27, uint(AuthenticationInApp::StateStart),
      28, uint(AuthenticationInApp::StateSignIn),
      29, uint(AuthenticationInApp::StateSignUp),
      30, uint(AuthenticationInApp::StateUnblockCodeNeeded),
      31, uint(AuthenticationInApp::StateVerificationSessionByEmailNeeded),
      32, uint(AuthenticationInApp::StateVerificationSessionByTotpNeeded),
      33, uint(AuthenticationInApp::StateFallbackInBrowser),
      34, uint(AuthenticationInApp::ErrorAccountAlreadyExists),
      35, uint(AuthenticationInApp::ErrorUnknownAccount),
      36, uint(AuthenticationInApp::ErrorIncorrectPassword),
      37, uint(AuthenticationInApp::ErrorInvalidEmailCode),
      38, uint(AuthenticationInApp::ErrorEmailTypeNotSupported),
      39, uint(AuthenticationInApp::ErrorEmailAlreadyExists),
      40, uint(AuthenticationInApp::ErrorEmailCanNotBeUsedToLogin),
      41, uint(AuthenticationInApp::ErrorFailedToSendEmail),
      42, uint(AuthenticationInApp::ErrorTooManyRequests),
      43, uint(AuthenticationInApp::ErrorServerUnavailable),
      44, uint(AuthenticationInApp::ErrorInvalidTotpCode),

       0        // eod
};

void AuthenticationInApp::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AuthenticationInApp *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->stateChanged(); break;
        case 1: _t->errorOccurred((*reinterpret_cast< ErrorType(*)>(_a[1]))); break;
        case 2: _t->checkAccount((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->setPassword((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: { bool _r = _t->validateEmailAddress((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 5: { bool _r = _t->validatePasswordCommons((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 6: { bool _r = _t->validatePasswordLength((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 7: { bool _r = _t->validatePasswordEmail((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 8: _t->reset(); break;
        case 9: _t->signIn(); break;
        case 10: _t->signUp(); break;
        case 11: _t->setUnblockCodeAndContinue((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 12: _t->resendUnblockCodeEmail(); break;
        case 13: _t->verifySessionEmailCode((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 14: _t->resendVerificationSessionCodeEmail(); break;
        case 15: _t->verifySessionTotpCode((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AuthenticationInApp::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AuthenticationInApp::stateChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (AuthenticationInApp::*)(ErrorType );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AuthenticationInApp::errorOccurred)) {
                *result = 1;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<AuthenticationInApp *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< State*>(_v) = _t->state(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject AuthenticationInApp::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_AuthenticationInApp.data,
    qt_meta_data_AuthenticationInApp,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *AuthenticationInApp::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AuthenticationInApp::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AuthenticationInApp.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AuthenticationInApp::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 16;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 1;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void AuthenticationInApp::stateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void AuthenticationInApp::errorOccurred(ErrorType _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
