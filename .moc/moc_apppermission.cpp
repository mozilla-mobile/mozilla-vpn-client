/****************************************************************************
** Meta object code from reading C++ file 'apppermission.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../src/apppermission.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'apppermission.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AppPermission_t {
    QByteArrayData data[16];
    char stringdata0[178];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AppPermission_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AppPermission_t qt_meta_stringdata_AppPermission = {
    {
QT_MOC_LITERAL(0, 0, 13), // "AppPermission"
QT_MOC_LITERAL(1, 14, 12), // "readyChanged"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 12), // "notification"
QT_MOC_LITERAL(4, 41, 4), // "type"
QT_MOC_LITERAL(5, 46, 7), // "message"
QT_MOC_LITERAL(6, 54, 13), // "actionMessage"
QT_MOC_LITERAL(7, 68, 14), // "receiveAppList"
QT_MOC_LITERAL(8, 83, 21), // "QMap<QString,QString>"
QT_MOC_LITERAL(9, 105, 7), // "applist"
QT_MOC_LITERAL(10, 113, 4), // "flip"
QT_MOC_LITERAL(11, 118, 5), // "appID"
QT_MOC_LITERAL(12, 124, 14), // "requestApplist"
QT_MOC_LITERAL(13, 139, 10), // "protectAll"
QT_MOC_LITERAL(14, 150, 12), // "unprotectAll"
QT_MOC_LITERAL(15, 163, 14) // "openFilePicker"

    },
    "AppPermission\0readyChanged\0\0notification\0"
    "type\0message\0actionMessage\0receiveAppList\0"
    "QMap<QString,QString>\0applist\0flip\0"
    "appID\0requestApplist\0protectAll\0"
    "unprotectAll\0openFilePicker"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AppPermission[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x06 /* Public */,
       3,    3,   60,    2, 0x06 /* Public */,
       3,    2,   67,    2, 0x26 /* Public | MethodCloned */,

 // slots: name, argc, parameters, tag, flags
       7,    1,   72,    2, 0x08 /* Private */,

 // methods: name, argc, parameters, tag, flags
      10,    1,   75,    2, 0x02 /* Public */,
      12,    0,   78,    2, 0x02 /* Public */,
      13,    0,   79,    2, 0x02 /* Public */,
      14,    0,   80,    2, 0x02 /* Public */,
      15,    0,   81,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString,    4,    5,    6,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    4,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 8,    9,

 // methods: parameters
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void AppPermission::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AppPermission *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->readyChanged(); break;
        case 1: _t->notification((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 2: _t->notification((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 3: _t->receiveAppList((*reinterpret_cast< const QMap<QString,QString>(*)>(_a[1]))); break;
        case 4: _t->flip((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->requestApplist(); break;
        case 6: _t->protectAll(); break;
        case 7: _t->unprotectAll(); break;
        case 8: _t->openFilePicker(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AppPermission::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AppPermission::readyChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (AppPermission::*)(const QString & , const QString & , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AppPermission::notification)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject AppPermission::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_meta_stringdata_AppPermission.data,
    qt_meta_data_AppPermission,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *AppPermission::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AppPermission::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AppPermission.stringdata0))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int AppPermission::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void AppPermission::readyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void AppPermission::notification(const QString & _t1, const QString & _t2, const QString & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
