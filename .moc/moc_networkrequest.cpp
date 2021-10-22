/****************************************************************************
** Meta object code from reading C++ file 'networkrequest.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../src/networkrequest.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'networkrequest.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_NetworkRequest_t {
    QByteArrayData data[14];
    char stringdata0[176];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_NetworkRequest_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_NetworkRequest_t qt_meta_stringdata_NetworkRequest = {
    {
QT_MOC_LITERAL(0, 0, 14), // "NetworkRequest"
QT_MOC_LITERAL(1, 15, 21), // "requestHeaderReceived"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 15), // "NetworkRequest*"
QT_MOC_LITERAL(4, 54, 7), // "request"
QT_MOC_LITERAL(5, 62, 13), // "requestFailed"
QT_MOC_LITERAL(6, 76, 27), // "QNetworkReply::NetworkError"
QT_MOC_LITERAL(7, 104, 5), // "error"
QT_MOC_LITERAL(8, 110, 4), // "data"
QT_MOC_LITERAL(9, 115, 17), // "requestRedirected"
QT_MOC_LITERAL(10, 133, 3), // "url"
QT_MOC_LITERAL(11, 137, 16), // "requestCompleted"
QT_MOC_LITERAL(12, 154, 13), // "replyFinished"
QT_MOC_LITERAL(13, 168, 7) // "timeout"

    },
    "NetworkRequest\0requestHeaderReceived\0"
    "\0NetworkRequest*\0request\0requestFailed\0"
    "QNetworkReply::NetworkError\0error\0"
    "data\0requestRedirected\0url\0requestCompleted\0"
    "replyFinished\0timeout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_NetworkRequest[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       5,    2,   47,    2, 0x06 /* Public */,
       9,    2,   52,    2, 0x06 /* Public */,
      11,    1,   57,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      12,    0,   60,    2, 0x08 /* Private */,
      13,    0,   61,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6, QMetaType::QByteArray,    7,    8,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QUrl,    4,   10,
    QMetaType::Void, QMetaType::QByteArray,    8,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void NetworkRequest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<NetworkRequest *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->requestHeaderReceived((*reinterpret_cast< NetworkRequest*(*)>(_a[1]))); break;
        case 1: _t->requestFailed((*reinterpret_cast< QNetworkReply::NetworkError(*)>(_a[1])),(*reinterpret_cast< const QByteArray(*)>(_a[2]))); break;
        case 2: _t->requestRedirected((*reinterpret_cast< NetworkRequest*(*)>(_a[1])),(*reinterpret_cast< const QUrl(*)>(_a[2]))); break;
        case 3: _t->requestCompleted((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 4: _t->replyFinished(); break;
        case 5: _t->timeout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< NetworkRequest* >(); break;
            }
            break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QNetworkReply::NetworkError >(); break;
            }
            break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< NetworkRequest* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (NetworkRequest::*)(NetworkRequest * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NetworkRequest::requestHeaderReceived)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (NetworkRequest::*)(QNetworkReply::NetworkError , const QByteArray & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NetworkRequest::requestFailed)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (NetworkRequest::*)(NetworkRequest * , const QUrl & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NetworkRequest::requestRedirected)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (NetworkRequest::*)(const QByteArray & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NetworkRequest::requestCompleted)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject NetworkRequest::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_NetworkRequest.data,
    qt_meta_data_NetworkRequest,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *NetworkRequest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *NetworkRequest::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_NetworkRequest.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int NetworkRequest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void NetworkRequest::requestHeaderReceived(NetworkRequest * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void NetworkRequest::requestFailed(QNetworkReply::NetworkError _t1, const QByteArray & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void NetworkRequest::requestRedirected(NetworkRequest * _t1, const QUrl & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void NetworkRequest::requestCompleted(const QByteArray & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
