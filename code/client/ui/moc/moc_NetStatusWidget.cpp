/****************************************************************************
** Meta object code from reading C++ file 'NetStatusWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../forms/NetStatusWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NetStatusWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_noda__ui__NetStatusWidget_t {
	QByteArrayData data[8];
	char stringdata0[101];
};
#define QT_MOC_LITERAL(idx, ofs, len)                            \
	Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
	                                                        qptrdiff(offsetof(qt_meta_stringdata_noda__ui__NetStatusWidget_t, stringdata0) + ofs - idx * sizeof(QByteArrayData)))
static const qt_meta_stringdata_noda__ui__NetStatusWidget_t qt_meta_stringdata_noda__ui__NetStatusWidget = {
	{
	    QT_MOC_LITERAL(0, 0, 25),  // "noda::ui::NetStatusWidget"
	    QT_MOC_LITERAL(1, 26, 11), // "OnConnected"
	    QT_MOC_LITERAL(2, 38, 0),  // ""
	    QT_MOC_LITERAL(3, 39, 12), // "OnDisconnect"
	    QT_MOC_LITERAL(4, 52, 8),  // "uint32_t"
	    QT_MOC_LITERAL(5, 61, 11), // "OnBroadcast"
	    QT_MOC_LITERAL(6, 73, 13), // "OnStatsUpdate"
	    QT_MOC_LITERAL(7, 87, 13)  // "net::NetStats"

	},
	"noda::ui::NetStatusWidget\0OnConnected\0"
	"\0OnDisconnect\0uint32_t\0OnBroadcast\0"
	"OnStatsUpdate\0net::NetStats"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_noda__ui__NetStatusWidget[] = {

	// content:
	7,     // revision
	0,     // classname
	0, 0,  // classinfo
	4, 14, // methods
	0, 0,  // properties
	0, 0,  // enums/sets
	0, 0,  // constructors
	0,     // flags
	0,     // signalCount

	// slots: name, argc, parameters, tag, flags
	1, 0, 34, 2, 0x0a /* Public */,
	3, 1, 35, 2, 0x0a /* Public */,
	5, 1, 38, 2, 0x0a /* Public */,
	6, 1, 41, 2, 0x0a /* Public */,

	// slots: parameters
	QMetaType::Void,
	QMetaType::Void, 0x80000000 | 4, 2,
	QMetaType::Void, QMetaType::Int, 2,
	QMetaType::Void, 0x80000000 | 7, 2,

	0 // eod
};

void noda::ui::NetStatusWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
	if(_c == QMetaObject::InvokeMetaMethod) {
		NetStatusWidget *_t = static_cast<NetStatusWidget *>(_o);
		Q_UNUSED(_t)
		switch(_id) {
		case 0: _t->OnConnected(); break;
		case 1: _t->OnDisconnect((*reinterpret_cast<uint32_t(*)>(_a[1]))); break;
		case 2: _t->OnBroadcast((*reinterpret_cast<int(*)>(_a[1]))); break;
		case 3: _t->OnStatsUpdate((*reinterpret_cast<const net::NetStats(*)>(_a[1]))); break;
		default:;
		}
	}
}

const QMetaObject noda::ui::NetStatusWidget::staticMetaObject = {
	{ &QWidget::staticMetaObject, qt_meta_stringdata_noda__ui__NetStatusWidget.data,
	  qt_meta_data_noda__ui__NetStatusWidget, qt_static_metacall, Q_NULLPTR, Q_NULLPTR }
};

const QMetaObject *noda::ui::NetStatusWidget::metaObject() const
{
	return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *noda::ui::NetStatusWidget::qt_metacast(const char *_clname)
{
	if(!_clname)
		return Q_NULLPTR;
	if(!strcmp(_clname, qt_meta_stringdata_noda__ui__NetStatusWidget.stringdata0))
		return static_cast<void *>(const_cast<NetStatusWidget *>(this));
	return QWidget::qt_metacast(_clname);
}

int noda::ui::NetStatusWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
	_id = QWidget::qt_metacall(_c, _id, _a);
	if(_id < 0)
		return _id;
	if(_c == QMetaObject::InvokeMetaMethod) {
		if(_id < 4)
			qt_static_metacall(this, _c, _id, _a);
		_id -= 4;
	} else if(_c == QMetaObject::RegisterMethodArgumentMetaType) {
		if(_id < 4)
			*reinterpret_cast<int *>(_a[0]) = -1;
		_id -= 4;
	}
	return _id;
}
QT_END_MOC_NAMESPACE
