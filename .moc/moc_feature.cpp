/****************************************************************************
** Meta object code from reading C++ file 'feature.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../src/models/feature.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'feature.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Feature_t {
    QByteArrayData data[17];
    char stringdata0[176];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Feature_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Feature_t qt_meta_stringdata_Feature = {
    {
QT_MOC_LITERAL(0, 0, 7), // "Feature"
QT_MOC_LITERAL(1, 8, 16), // "supportedChanged"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 10), // "newChanged"
QT_MOC_LITERAL(4, 37, 16), // "isDevModeEnabled"
QT_MOC_LITERAL(5, 54, 2), // "id"
QT_MOC_LITERAL(6, 57, 4), // "name"
QT_MOC_LITERAL(7, 62, 7), // "isMajor"
QT_MOC_LITERAL(8, 70, 11), // "displayName"
QT_MOC_LITERAL(9, 82, 16), // "shortDescription"
QT_MOC_LITERAL(10, 99, 11), // "description"
QT_MOC_LITERAL(11, 111, 9), // "imagePath"
QT_MOC_LITERAL(12, 121, 8), // "iconPath"
QT_MOC_LITERAL(13, 130, 10), // "isReleased"
QT_MOC_LITERAL(14, 141, 5), // "isNew"
QT_MOC_LITERAL(15, 147, 16), // "devModeWriteable"
QT_MOC_LITERAL(16, 164, 11) // "isSupported"

    },
    "Feature\0supportedChanged\0\0newChanged\0"
    "isDevModeEnabled\0id\0name\0isMajor\0"
    "displayName\0shortDescription\0description\0"
    "imagePath\0iconPath\0isReleased\0isNew\0"
    "devModeWriteable\0isSupported"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Feature[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
      12,   32, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x06 /* Public */,
       3,    0,   30,    2, 0x06 /* Public */,

 // methods: name, argc, parameters, tag, flags
       4,    0,   31,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Bool,

 // properties: name, type, flags
       5, QMetaType::QString, 0x00095401,
       6, QMetaType::QString, 0x00095401,
       7, QMetaType::Bool, 0x00095401,
       8, QMetaType::QString, 0x00095401,
       9, QMetaType::QString, 0x00095401,
      10, QMetaType::QString, 0x00095401,
      11, QMetaType::QString, 0x00095401,
      12, QMetaType::QString, 0x00095401,
      13, QMetaType::Bool, 0x00095401,
      14, QMetaType::Bool, 0x00095401,
      15, QMetaType::Bool, 0x00095401,
      16, QMetaType::Bool, 0x00495001,

 // properties: notify_signal_id
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,

       0        // eod
};

void Feature::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Feature *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->supportedChanged(); break;
        case 1: _t->newChanged(); break;
        case 2: { bool _r = _t->isDevModeEnabled();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Feature::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Feature::supportedChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Feature::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Feature::newChanged)) {
                *result = 1;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<Feature *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->m_id; break;
        case 1: *reinterpret_cast< QString*>(_v) = _t->m_name; break;
        case 2: *reinterpret_cast< bool*>(_v) = _t->isMajor(); break;
        case 3: *reinterpret_cast< QString*>(_v) = _t->displayName(); break;
        case 4: *reinterpret_cast< QString*>(_v) = _t->shortDescription(); break;
        case 5: *reinterpret_cast< QString*>(_v) = _t->description(); break;
        case 6: *reinterpret_cast< QString*>(_v) = _t->imagePath(); break;
        case 7: *reinterpret_cast< QString*>(_v) = _t->iconPath(); break;
        case 8: *reinterpret_cast< bool*>(_v) = _t->m_released; break;
        case 9: *reinterpret_cast< bool*>(_v) = _t->isNew(); break;
        case 10: *reinterpret_cast< bool*>(_v) = _t->m_devModeWriteable; break;
        case 11: *reinterpret_cast< bool*>(_v) = _t->isSupported(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject Feature::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_Feature.data,
    qt_meta_data_Feature,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Feature::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Feature::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Feature.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Feature::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 12;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 12;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 12;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 12;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 12;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void Feature::supportedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Feature::newChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
