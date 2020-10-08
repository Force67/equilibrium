/****************************************************************************
** Meta object code from reading C++ file 'SyncController.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../sync/SyncController.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SyncController.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_noda__SyncController_t {
  QByteArrayData data[8];
  char stringdata0[88];
};
#define QT_MOC_LITERAL(idx, ofs, len)                          \
  Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
                                                          qptrdiff(offsetof(qt_meta_stringdata_noda__SyncController_t, stringdata0) + ofs - idx * sizeof(QByteArrayData)))
static const qt_meta_stringdata_noda__SyncController_t qt_meta_stringdata_noda__SyncController = {
  {
      QT_MOC_LITERAL(0, 0, 20),  // "noda::SyncController"
      QT_MOC_LITERAL(1, 21, 9),  // "Connected"
      QT_MOC_LITERAL(2, 31, 0),  // ""
      QT_MOC_LITERAL(3, 32, 12), // "Disconnected"
      QT_MOC_LITERAL(4, 45, 8),  // "uint32_t"
      QT_MOC_LITERAL(5, 54, 11), // "Broadcasted"
      QT_MOC_LITERAL(6, 66, 12), // "StatsUpdated"
      QT_MOC_LITERAL(7, 79, 8)   // "NetStats"

  },
  "noda::SyncController\0Connected\0\0"
  "Disconnected\0uint32_t\0Broadcasted\0"
  "StatsUpdated\0NetStats"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_noda__SyncController[] = {

  // content:
  7,     // revision
  0,     // classname
  0, 0,  // classinfo
  4, 14, // methods
  0, 0,  // properties
  0, 0,  // enums/sets
  0, 0,  // constructors
  0,     // flags
  4,     // signalCount

  // signals: name, argc, parameters, tag, flags
  1, 0, 34, 2, 0x06 /* Public */,
  3, 1, 35, 2, 0x06 /* Public */,
  5, 1, 38, 2, 0x06 /* Public */,
  6, 1, 41, 2, 0x06 /* Public */,

  // signals: parameters
  QMetaType::Void,
  QMetaType::Void, 0x80000000 | 4, 2,
  QMetaType::Void, QMetaType::Int, 2,
  QMetaType::Void, 0x80000000 | 7, 2,

  0 // eod
};

void noda::SyncController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
  if(_c == QMetaObject::InvokeMetaMethod) {
	SyncController *_t = static_cast<SyncController *>(_o);
	Q_UNUSED(_t)
	switch(_id) {
	case 0: _t->Connected(); break;
	case 1: _t->Disconnected((*reinterpret_cast<uint32_t(*)>(_a[1]))); break;
	case 2: _t->Broadcasted((*reinterpret_cast<int(*)>(_a[1]))); break;
	case 3: _t->StatsUpdated((*reinterpret_cast<const NetStats(*)>(_a[1]))); break;
	default:;
	}
  }
  else if(_c == QMetaObject::IndexOfMethod) {
	int *result = reinterpret_cast<int *>(_a[0]);
	void **func = reinterpret_cast<void **>(_a[1]);
	{
	  typedef void (SyncController::*_t)();
	  if(*reinterpret_cast<_t *>(func) == static_cast<_t>(&SyncController::Connected)) {
		*result = 0;
		return;
	  }
	}
	{
	  typedef void (SyncController::*_t)(uint32_t);
	  if(*reinterpret_cast<_t *>(func) == static_cast<_t>(&SyncController::Disconnected)) {
		*result = 1;
		return;
	  }
	}
	{
	  typedef void (SyncController::*_t)(int);
	  if(*reinterpret_cast<_t *>(func) == static_cast<_t>(&SyncController::Broadcasted)) {
		*result = 2;
		return;
	  }
	}
	{
	  typedef void (SyncController::*_t)(const NetStats &);
	  if(*reinterpret_cast<_t *>(func) == static_cast<_t>(&SyncController::StatsUpdated)) {
		*result = 3;
		return;
	  }
	}
  }
}

const QMetaObject noda::SyncController::staticMetaObject = {
  { &QObject::staticMetaObject, qt_meta_stringdata_noda__SyncController.data,
    qt_meta_data_noda__SyncController, qt_static_metacall, Q_NULLPTR, Q_NULLPTR }
};

const QMetaObject *noda::SyncController::metaObject() const
{
  return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *noda::SyncController::qt_metacast(const char *_clname)
{
  if(!_clname)
	return Q_NULLPTR;
  if(!strcmp(_clname, qt_meta_stringdata_noda__SyncController.stringdata0))
	return static_cast<void *>(const_cast<SyncController *>(this));
  if(!strcmp(_clname, "NetDelegate"))
	return static_cast<NetDelegate *>(const_cast<SyncController *>(this));
  return QObject::qt_metacast(_clname);
}

int noda::SyncController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
  _id = QObject::qt_metacall(_c, _id, _a);
  if(_id < 0)
	return _id;
  if(_c == QMetaObject::InvokeMetaMethod) {
	if(_id < 4)
	  qt_static_metacall(this, _c, _id, _a);
	_id -= 4;
  }
  else if(_c == QMetaObject::RegisterMethodArgumentMetaType) {
	if(_id < 4)
	  *reinterpret_cast<int *>(_a[0]) = -1;
	_id -= 4;
  }
  return _id;
}

// SIGNAL 0
void noda::SyncController::Connected()
{
  QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void noda::SyncController::Disconnected(uint32_t _t1)
{
  void *_a[] = { Q_NULLPTR, const_cast<void *>(reinterpret_cast<const void *>(&_t1)) };
  QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void noda::SyncController::Broadcasted(int _t1)
{
  void *_a[] = { Q_NULLPTR, const_cast<void *>(reinterpret_cast<const void *>(&_t1)) };
  QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void noda::SyncController::StatsUpdated(const NetStats &_t1)
{
  void *_a[] = { Q_NULLPTR, const_cast<void *>(reinterpret_cast<const void *>(&_t1)) };
  QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
