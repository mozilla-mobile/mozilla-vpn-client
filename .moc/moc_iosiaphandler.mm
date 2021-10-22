/****************************************************************************
** Meta object code from reading C++ file 'iosiaphandler.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../src/platforms/ios/iosiaphandler.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'iosiaphandler.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_IOSIAPHandler_t {
    QByteArrayData data[6];
    char stringdata0[74];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_IOSIAPHandler_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_IOSIAPHandler_t qt_meta_stringdata_IOSIAPHandler = {
    {
QT_MOC_LITERAL(0, 0, 13), // "IOSIAPHandler"
QT_MOC_LITERAL(1, 14, 17), // "productRegistered"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 7), // "product"
QT_MOC_LITERAL(4, 41, 28), // "processCompletedTransactions"
QT_MOC_LITERAL(5, 70, 3) // "ids"

    },
    "IOSIAPHandler\0productRegistered\0\0"
    "product\0processCompletedTransactions\0"
    "ids"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_IOSIAPHandler[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x0a /* Public */,
       4,    1,   27,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::VoidStar,    3,
    QMetaType::Void, QMetaType::QStringList,    5,

       0        // eod
};

void IOSIAPHandler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<IOSIAPHandler *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->productRegistered((*reinterpret_cast< void*(*)>(_a[1]))); break;
        case 1: _t->processCompletedTransactions((*reinterpret_cast< const QStringList(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject IOSIAPHandler::staticMetaObject = { {
    QMetaObject::SuperData::link<IAPHandler::staticMetaObject>(),
    qt_meta_stringdata_IOSIAPHandler.data,
    qt_meta_data_IOSIAPHandler,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *IOSIAPHandler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *IOSIAPHandler::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_IOSIAPHandler.stringdata0))
        return static_cast<void*>(this);
    return IAPHandler::qt_metacast(_clname);
}

int IOSIAPHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = IAPHandler::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
