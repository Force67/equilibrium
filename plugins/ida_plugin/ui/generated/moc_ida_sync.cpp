/****************************************************************************
** Meta object code from reading C++ file 'ida_sync.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "pch.h"
#include "../../sync/ida_sync.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ida_sync.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_IdaSync_t {
    QByteArrayData data[6];
    char stringdata0[47];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_IdaSync_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_IdaSync_t qt_meta_stringdata_IdaSync = {
    {
QT_MOC_LITERAL(0, 0, 7), // "IdaSync"
QT_MOC_LITERAL(1, 8, 11), // "StateChange"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 5), // "State"
QT_MOC_LITERAL(4, 27, 6), // "Notify"
QT_MOC_LITERAL(5, 34, 12) // "Notification"

    },
    "IdaSync\0StateChange\0\0State\0Notify\0"
    "Notification"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_IdaSync[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,
       4,    1,   27,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    2,
    QMetaType::Void, 0x80000000 | 5,    2,

 // enums: name, flags, count, data

 // enum data: key, value

       0        // eod
};

void IdaSync::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        IdaSync *_t = static_cast<IdaSync *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->StateChange((*reinterpret_cast< State(*)>(_a[1]))); break;
        case 1: _t->Notify((*reinterpret_cast< Notification(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (IdaSync::*_t)(State );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&IdaSync::StateChange)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (IdaSync::*_t)(Notification );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&IdaSync::Notify)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject IdaSync::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_IdaSync.data,
      qt_meta_data_IdaSync,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *IdaSync::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *IdaSync::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_IdaSync.stringdata0))
        return static_cast<void*>(const_cast< IdaSync*>(this));
    if (!strcmp(_clname, "network::ClientDelegate"))
        return static_cast< network::ClientDelegate*>(const_cast< IdaSync*>(this));
    return QObject::qt_metacast(_clname);
}

int IdaSync::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void IdaSync::StateChange(State _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void IdaSync::Notify(Notification _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
