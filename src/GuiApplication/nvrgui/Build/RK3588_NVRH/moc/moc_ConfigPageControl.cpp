/****************************************************************************
** Meta object code from reading C++ file 'ConfigPageControl.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/ConfigPageControl.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ConfigPageControl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ConfigPageControl_t {
    QByteArrayData data[13];
    char stringdata0[183];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ConfigPageControl_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ConfigPageControl_t qt_meta_stringdata_ConfigPageControl = {
    {
QT_MOC_LITERAL(0, 0, 17), // "ConfigPageControl"
QT_MOC_LITERAL(1, 18, 20), // "sigOpenCameraFeature"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 5), // "param"
QT_MOC_LITERAL(4, 46, 21), // "CAMERA_FEATURE_TYPE_e"
QT_MOC_LITERAL(5, 68, 11), // "featureType"
QT_MOC_LITERAL(6, 80, 11), // "cameraIndex"
QT_MOC_LITERAL(7, 92, 11), // "configParam"
QT_MOC_LITERAL(8, 104, 7), // "devName"
QT_MOC_LITERAL(9, 112, 18), // "slotCnfgBtnClicked"
QT_MOC_LITERAL(10, 131, 5), // "index"
QT_MOC_LITERAL(11, 137, 20), // "slotInfoPageBtnclick"
QT_MOC_LITERAL(12, 158, 24) // "slotUpdateCurrentElement"

    },
    "ConfigPageControl\0sigOpenCameraFeature\0"
    "\0param\0CAMERA_FEATURE_TYPE_e\0featureType\0"
    "cameraIndex\0configParam\0devName\0"
    "slotCnfgBtnClicked\0index\0slotInfoPageBtnclick\0"
    "slotUpdateCurrentElement"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ConfigPageControl[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    5,   44,    2, 0x06 /* Public */,
       1,    4,   55,    2, 0x26 /* Public | MethodCloned */,
       1,    3,   64,    2, 0x26 /* Public | MethodCloned */,

 // slots: name, argc, parameters, tag, flags
       9,    1,   71,    2, 0x0a /* Public */,
      11,    1,   74,    2, 0x0a /* Public */,
      12,    1,   77,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::VoidStar, 0x80000000 | 4, QMetaType::UChar, QMetaType::VoidStar, QMetaType::QString,    3,    5,    6,    7,    8,
    QMetaType::Void, QMetaType::VoidStar, 0x80000000 | 4, QMetaType::UChar, QMetaType::VoidStar,    3,    5,    6,    7,
    QMetaType::Void, QMetaType::VoidStar, 0x80000000 | 4, QMetaType::UChar,    3,    5,    6,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,   10,
    QMetaType::Void, QMetaType::Int,   10,
    QMetaType::Void, QMetaType::Int,   10,

       0        // eod
};

void ConfigPageControl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ConfigPageControl *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigOpenCameraFeature((*reinterpret_cast< void*(*)>(_a[1])),(*reinterpret_cast< CAMERA_FEATURE_TYPE_e(*)>(_a[2])),(*reinterpret_cast< quint8(*)>(_a[3])),(*reinterpret_cast< void*(*)>(_a[4])),(*reinterpret_cast< QString(*)>(_a[5]))); break;
        case 1: _t->sigOpenCameraFeature((*reinterpret_cast< void*(*)>(_a[1])),(*reinterpret_cast< CAMERA_FEATURE_TYPE_e(*)>(_a[2])),(*reinterpret_cast< quint8(*)>(_a[3])),(*reinterpret_cast< void*(*)>(_a[4]))); break;
        case 2: _t->sigOpenCameraFeature((*reinterpret_cast< void*(*)>(_a[1])),(*reinterpret_cast< CAMERA_FEATURE_TYPE_e(*)>(_a[2])),(*reinterpret_cast< quint8(*)>(_a[3]))); break;
        case 3: _t->slotCnfgBtnClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ConfigPageControl::*)(void * , CAMERA_FEATURE_TYPE_e , quint8 , void * , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ConfigPageControl::sigOpenCameraFeature)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ConfigPageControl::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_ConfigPageControl.data,
    qt_meta_data_ConfigPageControl,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ConfigPageControl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ConfigPageControl::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ConfigPageControl.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int ConfigPageControl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = KeyBoard::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void ConfigPageControl::sigOpenCameraFeature(void * _t1, CAMERA_FEATURE_TYPE_e _t2, quint8 _t3, void * _t4, QString _t5)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
