/****************************************************************************
** Meta object code from reading C++ file 'connectionhealth.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../src/connectionhealth.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'connectionhealth.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ConnectionHealth_t {
    QByteArrayData data[15];
    char stringdata0[189];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ConnectionHealth_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ConnectionHealth_t qt_meta_stringdata_ConnectionHealth = {
    {
QT_MOC_LITERAL(0, 0, 16), // "ConnectionHealth"
QT_MOC_LITERAL(1, 17, 16), // "stabilityChanged"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 11), // "pingChanged"
QT_MOC_LITERAL(4, 47, 22), // "connectionStateChanged"
QT_MOC_LITERAL(5, 70, 23), // "applicationStateChanged"
QT_MOC_LITERAL(6, 94, 20), // "Qt::ApplicationState"
QT_MOC_LITERAL(7, 115, 5), // "state"
QT_MOC_LITERAL(8, 121, 9), // "stability"
QT_MOC_LITERAL(9, 131, 19), // "ConnectionStability"
QT_MOC_LITERAL(10, 151, 7), // "latency"
QT_MOC_LITERAL(11, 159, 4), // "loss"
QT_MOC_LITERAL(12, 164, 6), // "Stable"
QT_MOC_LITERAL(13, 171, 8), // "Unstable"
QT_MOC_LITERAL(14, 180, 8) // "NoSignal"

    },
    "ConnectionHealth\0stabilityChanged\0\0"
    "pingChanged\0connectionStateChanged\0"
    "applicationStateChanged\0Qt::ApplicationState\0"
    "state\0stability\0ConnectionStability\0"
    "latency\0loss\0Stable\0Unstable\0NoSignal"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ConnectionHealth[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       3,   40, // properties
       1,   52, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x06 /* Public */,
       3,    0,   35,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   36,    2, 0x0a /* Public */,
       5,    1,   37,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6,    7,

 // properties: name, type, flags
       8, 0x80000000 | 9, 0x00495009,
      10, QMetaType::UInt, 0x00495001,
      11, QMetaType::Double, 0x00495001,

 // properties: notify_signal_id
       0,
       1,
       1,

 // enums: name, alias, flags, count, data
       9,    9, 0x0,    3,   57,

 // enum data: key, value
      12, uint(ConnectionHealth::Stable),
      13, uint(ConnectionHealth::Unstable),
      14, uint(ConnectionHealth::NoSignal),

       0        // eod
};

void ConnectionHealth::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ConnectionHealth *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->stabilityChanged(); break;
        case 1: _t->pingChanged(); break;
        case 2: _t->connectionStateChanged(); break;
        case 3: _t->applicationStateChanged((*reinterpret_cast< Qt::ApplicationState(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ConnectionHealth::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ConnectionHealth::stabilityChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ConnectionHealth::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ConnectionHealth::pingChanged)) {
                *result = 1;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<ConnectionHealth *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< ConnectionStability*>(_v) = _t->stability(); break;
        case 1: *reinterpret_cast< uint*>(_v) = _t->latency(); break;
        case 2: *reinterpret_cast< double*>(_v) = _t->loss(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject ConnectionHealth::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ConnectionHealth.data,
    qt_meta_data_ConnectionHealth,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ConnectionHealth::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ConnectionHealth::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ConnectionHealth.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ConnectionHealth::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 3;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void ConnectionHealth::stabilityChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void ConnectionHealth::pingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
