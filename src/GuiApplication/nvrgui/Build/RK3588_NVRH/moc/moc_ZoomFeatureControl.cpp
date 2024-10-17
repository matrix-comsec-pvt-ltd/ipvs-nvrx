/****************************************************************************
** Meta object code from reading C++ file 'ZoomFeatureControl.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/ZoomFeatureControl.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ZoomFeatureControl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ZoomFeatureControl_t {
    QByteArrayData data[15];
    char stringdata0[210];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ZoomFeatureControl_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ZoomFeatureControl_t qt_meta_stringdata_ZoomFeatureControl = {
    {
QT_MOC_LITERAL(0, 0, 18), // "ZoomFeatureControl"
QT_MOC_LITERAL(1, 19, 15), // "sigChangeLayout"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 13), // "LAYOUT_TYPE_e"
QT_MOC_LITERAL(4, 50, 6), // "layout"
QT_MOC_LITERAL(5, 57, 14), // "DISPLAY_TYPE_e"
QT_MOC_LITERAL(6, 72, 9), // "displayId"
QT_MOC_LITERAL(7, 82, 11), // "windowIndex"
QT_MOC_LITERAL(8, 94, 14), // "ifActualWindow"
QT_MOC_LITERAL(9, 109, 12), // "ifUpdatePage"
QT_MOC_LITERAL(10, 122, 22), // "sigExitFromZoomFeature"
QT_MOC_LITERAL(11, 145, 22), // "slotMenuButtonSelected"
QT_MOC_LITERAL(12, 168, 9), // "menuLabel"
QT_MOC_LITERAL(13, 178, 9), // "menuIndex"
QT_MOC_LITERAL(14, 188, 21) // "slotMenuListDestroyed"

    },
    "ZoomFeatureControl\0sigChangeLayout\0\0"
    "LAYOUT_TYPE_e\0layout\0DISPLAY_TYPE_e\0"
    "displayId\0windowIndex\0ifActualWindow\0"
    "ifUpdatePage\0sigExitFromZoomFeature\0"
    "slotMenuButtonSelected\0menuLabel\0"
    "menuIndex\0slotMenuListDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ZoomFeatureControl[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    5,   34,    2, 0x06 /* Public */,
      10,    0,   45,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    2,   46,    2, 0x0a /* Public */,
      14,    0,   51,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, QMetaType::UShort, QMetaType::Bool, QMetaType::Bool,    4,    6,    7,    8,    9,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::UChar,   12,   13,
    QMetaType::Void,

       0        // eod
};

void ZoomFeatureControl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ZoomFeatureControl *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigChangeLayout((*reinterpret_cast< LAYOUT_TYPE_e(*)>(_a[1])),(*reinterpret_cast< DISPLAY_TYPE_e(*)>(_a[2])),(*reinterpret_cast< quint16(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 1: _t->sigExitFromZoomFeature(); break;
        case 2: _t->slotMenuButtonSelected((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2]))); break;
        case 3: _t->slotMenuListDestroyed(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ZoomFeatureControl::*)(LAYOUT_TYPE_e , DISPLAY_TYPE_e , quint16 , bool , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ZoomFeatureControl::sigChangeLayout)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ZoomFeatureControl::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ZoomFeatureControl::sigExitFromZoomFeature)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ZoomFeatureControl::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ZoomFeatureControl.data,
    qt_meta_data_ZoomFeatureControl,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ZoomFeatureControl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ZoomFeatureControl::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZoomFeatureControl.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ZoomFeatureControl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void ZoomFeatureControl::sigChangeLayout(LAYOUT_TYPE_e _t1, DISPLAY_TYPE_e _t2, quint16 _t3, bool _t4, bool _t5)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ZoomFeatureControl::sigExitFromZoomFeature()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
