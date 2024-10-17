/****************************************************************************
** Meta object code from reading C++ file 'AddCamera.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/CameraSettings/AddCamera.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AddCamera.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AddCamera_t {
    QByteArrayData data[24];
    char stringdata0[338];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AddCamera_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AddCamera_t qt_meta_stringdata_AddCamera = {
    {
QT_MOC_LITERAL(0, 0, 9), // "AddCamera"
QT_MOC_LITERAL(1, 10, 15), // "sigDeleteObject"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 11), // "cameraIndex"
QT_MOC_LITERAL(4, 39, 14), // "saveCameraFlag"
QT_MOC_LITERAL(5, 54, 12), // "ipAddressStr"
QT_MOC_LITERAL(6, 67, 12), // "onvifPortStr"
QT_MOC_LITERAL(7, 80, 11), // "httpPortStr"
QT_MOC_LITERAL(8, 92, 12), // "onvifSupport"
QT_MOC_LITERAL(9, 105, 12), // "brandlistStr"
QT_MOC_LITERAL(10, 118, 12), // "modellistStr"
QT_MOC_LITERAL(11, 131, 7), // "camName"
QT_MOC_LITERAL(12, 139, 8), // "userName"
QT_MOC_LITERAL(13, 148, 8), // "password"
QT_MOC_LITERAL(14, 157, 13), // "selectedIndex"
QT_MOC_LITERAL(15, 171, 9), // "currIndex"
QT_MOC_LITERAL(16, 181, 15), // "slotButtonClick"
QT_MOC_LITERAL(17, 197, 16), // "slotValueChanged"
QT_MOC_LITERAL(18, 214, 21), // "slotOptionButtonClick"
QT_MOC_LITERAL(19, 236, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(20, 256, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(21, 281, 20), // "slotInfoPageBtnclick"
QT_MOC_LITERAL(22, 302, 16), // "slotObjectDelete"
QT_MOC_LITERAL(23, 319, 18) // "slotValueListEmpty"

    },
    "AddCamera\0sigDeleteObject\0\0cameraIndex\0"
    "saveCameraFlag\0ipAddressStr\0onvifPortStr\0"
    "httpPortStr\0onvifSupport\0brandlistStr\0"
    "modellistStr\0camName\0userName\0password\0"
    "selectedIndex\0currIndex\0slotButtonClick\0"
    "slotValueChanged\0slotOptionButtonClick\0"
    "OPTION_STATE_TYPE_e\0slotUpdateCurrentElement\0"
    "slotInfoPageBtnclick\0slotObjectDelete\0"
    "slotValueListEmpty"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AddCamera[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,   13,   54,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      16,    1,   81,    2, 0x0a /* Public */,
      17,    2,   84,    2, 0x0a /* Public */,
      18,    2,   89,    2, 0x0a /* Public */,
      20,    1,   94,    2, 0x0a /* Public */,
      21,    1,   97,    2, 0x0a /* Public */,
      22,    1,  100,    2, 0x0a /* Public */,
      23,    1,  103,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::UChar, QMetaType::Bool, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::Bool, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::UChar, QMetaType::UChar,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,   15,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    2,
    QMetaType::Void, 0x80000000 | 19, QMetaType::Int,    2,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::UChar,    2,
    QMetaType::Void, QMetaType::UChar,    2,

       0        // eod
};

void AddCamera::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AddCamera *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigDeleteObject((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4])),(*reinterpret_cast< QString(*)>(_a[5])),(*reinterpret_cast< bool(*)>(_a[6])),(*reinterpret_cast< QString(*)>(_a[7])),(*reinterpret_cast< QString(*)>(_a[8])),(*reinterpret_cast< QString(*)>(_a[9])),(*reinterpret_cast< QString(*)>(_a[10])),(*reinterpret_cast< QString(*)>(_a[11])),(*reinterpret_cast< quint8(*)>(_a[12])),(*reinterpret_cast< quint8(*)>(_a[13]))); break;
        case 1: _t->slotButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slotValueChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 3: _t->slotOptionButtonClick((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->slotObjectDelete((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 7: _t->slotValueListEmpty((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AddCamera::*)(quint8 , bool , QString , QString , QString , bool , QString , QString , QString , QString , QString , quint8 , quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AddCamera::sigDeleteObject)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject AddCamera::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_AddCamera.data,
    qt_meta_data_AddCamera,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *AddCamera::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AddCamera::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AddCamera.stringdata0))
        return static_cast<void*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int AddCamera::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = KeyBoard::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void AddCamera::sigDeleteObject(quint8 _t1, bool _t2, QString _t3, QString _t4, QString _t5, bool _t6, QString _t7, QString _t8, QString _t9, QString _t10, QString _t11, quint8 _t12, quint8 _t13)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t6))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t7))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t8))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t9))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t10))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t11))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t12))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t13))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
