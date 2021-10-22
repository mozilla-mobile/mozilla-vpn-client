/****************************************************************************
** Meta object code from reading C++ file 'controllerimpl.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../src/controllerimpl.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'controllerimpl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ControllerImpl_t {
    QByteArrayData data[13];
    char stringdata0[148];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ControllerImpl_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ControllerImpl_t qt_meta_stringdata_ControllerImpl = {
    {
QT_MOC_LITERAL(0, 0, 14), // "ControllerImpl"
QT_MOC_LITERAL(1, 15, 11), // "initialized"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 6), // "status"
QT_MOC_LITERAL(4, 35, 9), // "connected"
QT_MOC_LITERAL(5, 45, 14), // "connectionDate"
QT_MOC_LITERAL(6, 60, 12), // "disconnected"
QT_MOC_LITERAL(7, 73, 13), // "statusUpdated"
QT_MOC_LITERAL(8, 87, 17), // "serverIpv4Gateway"
QT_MOC_LITERAL(9, 105, 17), // "deviceIpv4Address"
QT_MOC_LITERAL(10, 123, 8), // "uint64_t"
QT_MOC_LITERAL(11, 132, 7), // "txBytes"
QT_MOC_LITERAL(12, 140, 7) // "rxBytes"

    },
    "ControllerImpl\0initialized\0\0status\0"
    "connected\0connectionDate\0disconnected\0"
    "statusUpdated\0serverIpv4Gateway\0"
    "deviceIpv4Address\0uint64_t\0txBytes\0"
    "rxBytes"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ControllerImpl[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   34,    2, 0x06 /* Public */,
       4,    0,   41,    2, 0x06 /* Public */,
       6,    0,   42,    2, 0x06 /* Public */,
       7,    4,   43,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool, QMetaType::Bool, QMetaType::QDateTime,    3,    4,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, 0x80000000 | 10, 0x80000000 | 10,    8,    9,   11,   12,

       0        // eod
};

void ControllerImpl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ControllerImpl *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->initialized((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< const QDateTime(*)>(_a[3]))); break;
        case 1: _t->connected(); break;
        case 2: _t->disconnected(); break;
        case 3: _t->statusUpdated((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< uint64_t(*)>(_a[3])),(*reinterpret_cast< uint64_t(*)>(_a[4]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ControllerImpl::*)(bool , bool , const QDateTime & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ControllerImpl::initialized)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ControllerImpl::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ControllerImpl::connected)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ControllerImpl::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ControllerImpl::disconnected)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ControllerImpl::*)(const QString & , const QString & , uint64_t , uint64_t );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ControllerImpl::statusUpdated)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ControllerImpl::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ControllerImpl.data,
    qt_meta_data_ControllerImpl,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ControllerImpl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ControllerImpl::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ControllerImpl.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ControllerImpl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
    return _id;
}

// SIGNAL 0
void ControllerImpl::initialized(bool _t1, bool _t2, const QDateTime & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ControllerImpl::connected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void ControllerImpl::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void ControllerImpl::statusUpdated(const QString & _t1, const QString & _t2, uint64_t _t3, uint64_t _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
