/****************************************************************************
** Meta object code from reading C++ file 'Settings.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Settings.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Settings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Settings_t {
    QByteArrayData data[25];
    char stringdata0[385];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Settings_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Settings_t qt_meta_stringdata_Settings = {
    {
QT_MOC_LITERAL(0, 0, 8), // "Settings"
QT_MOC_LITERAL(1, 9, 20), // "sigOpenCameraFeature"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 5), // "param"
QT_MOC_LITERAL(4, 37, 21), // "CAMERA_FEATURE_TYPE_e"
QT_MOC_LITERAL(5, 59, 11), // "featureType"
QT_MOC_LITERAL(6, 71, 11), // "cameraIndex"
QT_MOC_LITERAL(7, 83, 11), // "configParam"
QT_MOC_LITERAL(8, 95, 7), // "devName"
QT_MOC_LITERAL(9, 103, 27), // "sigChangeToolbarButtonState"
QT_MOC_LITERAL(10, 131, 21), // "TOOLBAR_BUTTON_TYPE_e"
QT_MOC_LITERAL(11, 153, 11), // "buttonIndex"
QT_MOC_LITERAL(12, 165, 12), // "STATE_TYPE_e"
QT_MOC_LITERAL(13, 178, 5), // "state"
QT_MOC_LITERAL(14, 184, 19), // "slotSettingsOptions"
QT_MOC_LITERAL(15, 204, 5), // "index"
QT_MOC_LITERAL(16, 210, 22), // "slotSubSettingsOptions"
QT_MOC_LITERAL(17, 233, 14), // "subOptionIndex"
QT_MOC_LITERAL(18, 248, 22), // "slotSpinBoxValueChange"
QT_MOC_LITERAL(19, 271, 4), // "name"
QT_MOC_LITERAL(20, 276, 11), // "indexInPage"
QT_MOC_LITERAL(21, 288, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(22, 313, 21), // "slotOpenCameraFeature"
QT_MOC_LITERAL(23, 335, 28), // "slotChangeToolbarButtonState"
QT_MOC_LITERAL(24, 364, 20) // "slotInfoPageBtnclick"

    },
    "Settings\0sigOpenCameraFeature\0\0param\0"
    "CAMERA_FEATURE_TYPE_e\0featureType\0"
    "cameraIndex\0configParam\0devName\0"
    "sigChangeToolbarButtonState\0"
    "TOOLBAR_BUTTON_TYPE_e\0buttonIndex\0"
    "STATE_TYPE_e\0state\0slotSettingsOptions\0"
    "index\0slotSubSettingsOptions\0"
    "subOptionIndex\0slotSpinBoxValueChange\0"
    "name\0indexInPage\0slotUpdateCurrentElement\0"
    "slotOpenCameraFeature\0"
    "slotChangeToolbarButtonState\0"
    "slotInfoPageBtnclick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Settings[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    5,   59,    2, 0x06 /* Public */,
       9,    2,   70,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      14,    1,   75,    2, 0x0a /* Public */,
      16,    1,   78,    2, 0x0a /* Public */,
      18,    2,   81,    2, 0x0a /* Public */,
      21,    1,   86,    2, 0x0a /* Public */,
      22,    5,   89,    2, 0x0a /* Public */,
      23,    2,  100,    2, 0x0a /* Public */,
      24,    1,  105,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::VoidStar, 0x80000000 | 4, QMetaType::UChar, QMetaType::VoidStar, QMetaType::QString,    3,    5,    6,    7,    8,
    QMetaType::Void, 0x80000000 | 10, 0x80000000 | 12,   11,   13,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void, QMetaType::Int,   17,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,   19,   20,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void, QMetaType::VoidStar, 0x80000000 | 4, QMetaType::UChar, QMetaType::VoidStar, QMetaType::QString,    3,    5,    6,    7,    8,
    QMetaType::Void, 0x80000000 | 10, 0x80000000 | 12,   11,   13,
    QMetaType::Void, QMetaType::Int,    2,

       0        // eod
};

void Settings::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Settings *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigOpenCameraFeature((*reinterpret_cast< void*(*)>(_a[1])),(*reinterpret_cast< CAMERA_FEATURE_TYPE_e(*)>(_a[2])),(*reinterpret_cast< quint8(*)>(_a[3])),(*reinterpret_cast< void*(*)>(_a[4])),(*reinterpret_cast< QString(*)>(_a[5]))); break;
        case 1: _t->sigChangeToolbarButtonState((*reinterpret_cast< TOOLBAR_BUTTON_TYPE_e(*)>(_a[1])),(*reinterpret_cast< STATE_TYPE_e(*)>(_a[2]))); break;
        case 2: _t->slotSettingsOptions((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotSubSettingsOptions((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotSpinBoxValueChange((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 5: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->slotOpenCameraFeature((*reinterpret_cast< void*(*)>(_a[1])),(*reinterpret_cast< CAMERA_FEATURE_TYPE_e(*)>(_a[2])),(*reinterpret_cast< quint8(*)>(_a[3])),(*reinterpret_cast< void*(*)>(_a[4])),(*reinterpret_cast< QString(*)>(_a[5]))); break;
        case 7: _t->slotChangeToolbarButtonState((*reinterpret_cast< TOOLBAR_BUTTON_TYPE_e(*)>(_a[1])),(*reinterpret_cast< STATE_TYPE_e(*)>(_a[2]))); break;
        case 8: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Settings::*)(void * , CAMERA_FEATURE_TYPE_e , quint8 , void * , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Settings::sigOpenCameraFeature)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Settings::*)(TOOLBAR_BUTTON_TYPE_e , STATE_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Settings::sigChangeToolbarButtonState)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Settings::staticMetaObject = { {
    QMetaObject::SuperData::link<BackGround::staticMetaObject>(),
    qt_meta_stringdata_Settings.data,
    qt_meta_data_Settings,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Settings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Settings::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Settings.stringdata0))
        return static_cast<void*>(this);
    return BackGround::qt_metacast(_clname);
}

int Settings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BackGround::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void Settings::sigOpenCameraFeature(void * _t1, CAMERA_FEATURE_TYPE_e _t2, quint8 _t3, void * _t4, QString _t5)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Settings::sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e _t1, STATE_TYPE_e _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
