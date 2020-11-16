/****************************************************************************
** Meta object code from reading C++ file 'UiController.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "Pch.h"
#include "../UiController.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'UiController.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_noda__UiController_t {
    QByteArrayData data[6];
    char stringdata0[76];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_noda__UiController_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_noda__UiController_t qt_meta_stringdata_noda__UiController = {
    {
QT_MOC_LITERAL(0, 0, 18), // "noda::UiController"
QT_MOC_LITERAL(1, 19, 13), // "ToggleConnect"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 12), // "OpenSyncMenu"
QT_MOC_LITERAL(4, 47, 15), // "OpenAboutDialog"
QT_MOC_LITERAL(5, 63, 12) // "OpenSettings"

    },
    "noda::UiController\0ToggleConnect\0\0"
    "OpenSyncMenu\0OpenAboutDialog\0OpenSettings"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_noda__UiController[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x0a /* Public */,
       3,    0,   35,    2, 0x08 /* Private */,
       4,    0,   36,    2, 0x08 /* Private */,
       5,    0,   37,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void noda::UiController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        UiController *_t = static_cast<UiController *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->ToggleConnect(); break;
        case 1: _t->OpenSyncMenu(); break;
        case 2: _t->OpenAboutDialog(); break;
        case 3: _t->OpenSettings(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject noda::UiController::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_noda__UiController.data,
      qt_meta_data_noda__UiController,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *noda::UiController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *noda::UiController::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_noda__UiController.stringdata0))
        return static_cast<void*>(const_cast< UiController*>(this));
    return QObject::qt_metacast(_clname);
}

int noda::UiController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
QT_END_MOC_NAMESPACE
