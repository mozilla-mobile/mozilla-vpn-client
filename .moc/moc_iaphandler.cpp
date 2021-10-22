/****************************************************************************
** Meta object code from reading C++ file 'iaphandler.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../src/iaphandler.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'iaphandler.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_IAPHandler_t {
    QByteArrayData data[23];
    char stringdata0[414];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_IAPHandler_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_IAPHandler_t qt_meta_stringdata_IAPHandler = {
    {
QT_MOC_LITERAL(0, 0, 10), // "IAPHandler"
QT_MOC_LITERAL(1, 11, 18), // "productsRegistered"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 27), // "productsRegistrationStopped"
QT_MOC_LITERAL(4, 59, 19), // "subscriptionStarted"
QT_MOC_LITERAL(5, 79, 17), // "productIdentifier"
QT_MOC_LITERAL(6, 97, 18), // "subscriptionFailed"
QT_MOC_LITERAL(7, 116, 20), // "subscriptionCanceled"
QT_MOC_LITERAL(8, 137, 21), // "subscriptionCompleted"
QT_MOC_LITERAL(9, 159, 17), // "alreadySubscribed"
QT_MOC_LITERAL(10, 177, 19), // "billingNotAvailable"
QT_MOC_LITERAL(11, 197, 24), // "subscriptionNotValidated"
QT_MOC_LITERAL(12, 222, 16), // "stopSubscription"
QT_MOC_LITERAL(13, 239, 24), // "unknownProductRegistered"
QT_MOC_LITERAL(14, 264, 10), // "identifier"
QT_MOC_LITERAL(15, 275, 29), // "productsRegistrationCompleted"
QT_MOC_LITERAL(16, 305, 24), // "stopProductsRegistration"
QT_MOC_LITERAL(17, 330, 9), // "subscribe"
QT_MOC_LITERAL(18, 340, 11), // "ProductType"
QT_MOC_LITERAL(19, 352, 14), // "ProductMonthly"
QT_MOC_LITERAL(20, 367, 17), // "ProductHalfYearly"
QT_MOC_LITERAL(21, 385, 13), // "ProductYearly"
QT_MOC_LITERAL(22, 399, 14) // "ProductUnknown"

    },
    "IAPHandler\0productsRegistered\0\0"
    "productsRegistrationStopped\0"
    "subscriptionStarted\0productIdentifier\0"
    "subscriptionFailed\0subscriptionCanceled\0"
    "subscriptionCompleted\0alreadySubscribed\0"
    "billingNotAvailable\0subscriptionNotValidated\0"
    "stopSubscription\0unknownProductRegistered\0"
    "identifier\0productsRegistrationCompleted\0"
    "stopProductsRegistration\0subscribe\0"
    "ProductType\0ProductMonthly\0ProductHalfYearly\0"
    "ProductYearly\0ProductUnknown"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_IAPHandler[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       1,  104, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   84,    2, 0x06 /* Public */,
       3,    0,   85,    2, 0x06 /* Public */,
       4,    1,   86,    2, 0x06 /* Public */,
       6,    0,   89,    2, 0x06 /* Public */,
       7,    0,   90,    2, 0x06 /* Public */,
       8,    0,   91,    2, 0x06 /* Public */,
       9,    0,   92,    2, 0x06 /* Public */,
      10,    0,   93,    2, 0x06 /* Public */,
      11,    0,   94,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      12,    0,   95,    2, 0x0a /* Public */,
      13,    1,   96,    2, 0x0a /* Public */,
      15,    0,   99,    2, 0x0a /* Public */,
      16,    0,  100,    2, 0x0a /* Public */,

 // methods: name, argc, parameters, tag, flags
      17,    1,  101,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   14,
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void, QMetaType::QString,    5,

 // enums: name, alias, flags, count, data
      18,   18, 0x0,    4,  109,

 // enum data: key, value
      19, uint(IAPHandler::ProductMonthly),
      20, uint(IAPHandler::ProductHalfYearly),
      21, uint(IAPHandler::ProductYearly),
      22, uint(IAPHandler::ProductUnknown),

       0        // eod
};

void IAPHandler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<IAPHandler *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->productsRegistered(); break;
        case 1: _t->productsRegistrationStopped(); break;
        case 2: _t->subscriptionStarted((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->subscriptionFailed(); break;
        case 4: _t->subscriptionCanceled(); break;
        case 5: _t->subscriptionCompleted(); break;
        case 6: _t->alreadySubscribed(); break;
        case 7: _t->billingNotAvailable(); break;
        case 8: _t->subscriptionNotValidated(); break;
        case 9: _t->stopSubscription(); break;
        case 10: _t->unknownProductRegistered((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: _t->productsRegistrationCompleted(); break;
        case 12: _t->stopProductsRegistration(); break;
        case 13: _t->subscribe((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (IAPHandler::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IAPHandler::productsRegistered)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (IAPHandler::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IAPHandler::productsRegistrationStopped)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (IAPHandler::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IAPHandler::subscriptionStarted)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (IAPHandler::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IAPHandler::subscriptionFailed)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (IAPHandler::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IAPHandler::subscriptionCanceled)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (IAPHandler::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IAPHandler::subscriptionCompleted)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (IAPHandler::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IAPHandler::alreadySubscribed)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (IAPHandler::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IAPHandler::billingNotAvailable)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (IAPHandler::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IAPHandler::subscriptionNotValidated)) {
                *result = 8;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject IAPHandler::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_meta_stringdata_IAPHandler.data,
    qt_meta_data_IAPHandler,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *IAPHandler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *IAPHandler::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_IAPHandler.stringdata0))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int IAPHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void IAPHandler::productsRegistered()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void IAPHandler::productsRegistrationStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void IAPHandler::subscriptionStarted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void IAPHandler::subscriptionFailed()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void IAPHandler::subscriptionCanceled()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void IAPHandler::subscriptionCompleted()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void IAPHandler::alreadySubscribed()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void IAPHandler::billingNotAvailable()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void IAPHandler::subscriptionNotValidated()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
