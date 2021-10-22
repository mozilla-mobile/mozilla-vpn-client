/****************************************************************************
** Meta object code from reading C++ file 'closeeventhandler.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../src/closeeventhandler.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'closeeventhandler.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CloseEventHandler_t {
    QByteArrayData data[11];
    char stringdata0[103];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CloseEventHandler_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CloseEventHandler_t qt_meta_stringdata_CloseEventHandler = {
    {
QT_MOC_LITERAL(0, 0, 17), // "CloseEventHandler"
QT_MOC_LITERAL(1, 18, 6), // "goBack"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 11), // "QQuickItem*"
QT_MOC_LITERAL(4, 38, 4), // "item"
QT_MOC_LITERAL(5, 43, 10), // "removeItem"
QT_MOC_LITERAL(6, 54, 12), // "eventHandled"
QT_MOC_LITERAL(7, 67, 12), // "addStackView"
QT_MOC_LITERAL(8, 80, 9), // "stackView"
QT_MOC_LITERAL(9, 90, 7), // "addView"
QT_MOC_LITERAL(10, 98, 4) // "view"

    },
    "CloseEventHandler\0goBack\0\0QQuickItem*\0"
    "item\0removeItem\0eventHandled\0addStackView\0"
    "stackView\0addView\0view"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CloseEventHandler[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   42,    2, 0x08 /* Private */,

 // methods: name, argc, parameters, tag, flags
       6,    0,   45,    2, 0x02 /* Public */,
       7,    1,   46,    2, 0x02 /* Public */,
       9,    1,   49,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void, QMetaType::QObjectStar,    4,

 // methods: parameters
    QMetaType::Bool,
    QMetaType::Void, QMetaType::QVariant,    8,
    QMetaType::Void, QMetaType::QVariant,   10,

       0        // eod
};

void CloseEventHandler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CloseEventHandler *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->goBack((*reinterpret_cast< QQuickItem*(*)>(_a[1]))); break;
        case 1: _t->removeItem((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        case 2: { bool _r = _t->eventHandled();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 3: _t->addStackView((*reinterpret_cast< const QVariant(*)>(_a[1]))); break;
        case 4: _t->addView((*reinterpret_cast< const QVariant(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CloseEventHandler::*)(QQuickItem * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CloseEventHandler::goBack)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CloseEventHandler::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CloseEventHandler.data,
    qt_meta_data_CloseEventHandler,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CloseEventHandler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CloseEventHandler::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CloseEventHandler.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CloseEventHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void CloseEventHandler::goBack(QQuickItem * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
