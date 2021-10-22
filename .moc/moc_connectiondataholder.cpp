/****************************************************************************
** Meta object code from reading C++ file 'connectiondataholder.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../src/connectiondataholder.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'connectiondataholder.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ConnectionDataHolder_t {
    QByteArrayData data[17];
    char stringdata0[193];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ConnectionDataHolder_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ConnectionDataHolder_t qt_meta_stringdata_ConnectionDataHolder = {
    {
QT_MOC_LITERAL(0, 0, 20), // "ConnectionDataHolder"
QT_MOC_LITERAL(1, 21, 16), // "ipAddressChecked"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 18), // "ipv4AddressChanged"
QT_MOC_LITERAL(4, 58, 18), // "ipv6AddressChanged"
QT_MOC_LITERAL(5, 77, 12), // "bytesChanged"
QT_MOC_LITERAL(6, 90, 10), // "deactivate"
QT_MOC_LITERAL(7, 101, 12), // "stateChanged"
QT_MOC_LITERAL(8, 114, 8), // "activate"
QT_MOC_LITERAL(9, 123, 8), // "txSeries"
QT_MOC_LITERAL(10, 132, 8), // "rxSeries"
QT_MOC_LITERAL(11, 141, 5), // "axisX"
QT_MOC_LITERAL(12, 147, 5), // "axisY"
QT_MOC_LITERAL(13, 153, 11), // "ipv4Address"
QT_MOC_LITERAL(14, 165, 11), // "ipv6Address"
QT_MOC_LITERAL(15, 177, 7), // "txBytes"
QT_MOC_LITERAL(16, 185, 7) // "rxBytes"

    },
    "ConnectionDataHolder\0ipAddressChecked\0"
    "\0ipv4AddressChanged\0ipv6AddressChanged\0"
    "bytesChanged\0deactivate\0stateChanged\0"
    "activate\0txSeries\0rxSeries\0axisX\0axisY\0"
    "ipv4Address\0ipv6Address\0txBytes\0rxBytes"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ConnectionDataHolder[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       4,   64, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x06 /* Public */,
       3,    0,   50,    2, 0x06 /* Public */,
       4,    0,   51,    2, 0x06 /* Public */,
       5,    0,   52,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,   53,    2, 0x0a /* Public */,
       7,    0,   54,    2, 0x0a /* Public */,

 // methods: name, argc, parameters, tag, flags
       8,    4,   55,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void, QMetaType::QVariant, QMetaType::QVariant, QMetaType::QVariant, QMetaType::QVariant,    9,   10,   11,   12,

 // properties: name, type, flags
      13, QMetaType::QString, 0x00495001,
      14, QMetaType::QString, 0x00495001,
      15, QMetaType::ULongLong, 0x00495001,
      16, QMetaType::ULongLong, 0x00495001,

 // properties: notify_signal_id
       1,
       2,
       3,
       3,

       0        // eod
};

void ConnectionDataHolder::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ConnectionDataHolder *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->ipAddressChecked(); break;
        case 1: _t->ipv4AddressChanged(); break;
        case 2: _t->ipv6AddressChanged(); break;
        case 3: _t->bytesChanged(); break;
        case 4: _t->deactivate(); break;
        case 5: _t->stateChanged(); break;
        case 6: _t->activate((*reinterpret_cast< const QVariant(*)>(_a[1])),(*reinterpret_cast< const QVariant(*)>(_a[2])),(*reinterpret_cast< const QVariant(*)>(_a[3])),(*reinterpret_cast< const QVariant(*)>(_a[4]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ConnectionDataHolder::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ConnectionDataHolder::ipAddressChecked)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ConnectionDataHolder::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ConnectionDataHolder::ipv4AddressChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ConnectionDataHolder::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ConnectionDataHolder::ipv6AddressChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ConnectionDataHolder::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ConnectionDataHolder::bytesChanged)) {
                *result = 3;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<ConnectionDataHolder *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->ipv4Address(); break;
        case 1: *reinterpret_cast< QString*>(_v) = _t->ipv6Address(); break;
        case 2: *reinterpret_cast< quint64*>(_v) = _t->txBytes(); break;
        case 3: *reinterpret_cast< quint64*>(_v) = _t->rxBytes(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject ConnectionDataHolder::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ConnectionDataHolder.data,
    qt_meta_data_ConnectionDataHolder,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ConnectionDataHolder::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ConnectionDataHolder::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ConnectionDataHolder.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ConnectionDataHolder::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 4;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void ConnectionDataHolder::ipAddressChecked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void ConnectionDataHolder::ipv4AddressChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void ConnectionDataHolder::ipv6AddressChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void ConnectionDataHolder::bytesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
