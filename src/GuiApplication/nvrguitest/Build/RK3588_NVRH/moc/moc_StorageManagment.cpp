/****************************************************************************
** Meta object code from reading C++ file 'StorageManagment.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../StorageManagment.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'StorageManagment.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_StorageManagment_t {
    QByteArrayData data[16];
    char stringdata0[182];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_StorageManagment_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_StorageManagment_t qt_meta_stringdata_StorageManagment = {
    {
QT_MOC_LITERAL(0, 0, 16), // "StorageManagment"
QT_MOC_LITERAL(1, 17, 4), // "act1"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 18), // "UDEV_DEVICE_INFO_t"
QT_MOC_LITERAL(4, 42, 7), // "devInfo"
QT_MOC_LITERAL(5, 50, 21), // "internalMsgServerLoop"
QT_MOC_LITERAL(6, 72, 9), // "mountDisk"
QT_MOC_LITERAL(7, 82, 7), // "CHARPTR"
QT_MOC_LITERAL(8, 90, 10), // "deviceNode"
QT_MOC_LITERAL(9, 101, 9), // "mountPath"
QT_MOC_LITERAL(10, 111, 23), // "RAW_MEDIA_FORMAT_TYPE_e"
QT_MOC_LITERAL(11, 135, 6), // "fsType"
QT_MOC_LITERAL(12, 142, 11), // "mountDevice"
QT_MOC_LITERAL(13, 154, 4), // "type"
QT_MOC_LITERAL(14, 159, 8), // "baseNode"
QT_MOC_LITERAL(15, 168, 13) // "unmountDevice"

    },
    "StorageManagment\0act1\0\0UDEV_DEVICE_INFO_t\0"
    "devInfo\0internalMsgServerLoop\0mountDisk\0"
    "CHARPTR\0deviceNode\0mountPath\0"
    "RAW_MEDIA_FORMAT_TYPE_e\0fsType\0"
    "mountDevice\0type\0baseNode\0unmountDevice"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_StorageManagment[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,

 // methods: name, argc, parameters, tag, flags
       5,    0,   42,    2, 0x02 /* Public */,
       6,    3,   43,    2, 0x02 /* Public */,
      12,    2,   50,    2, 0x02 /* Public */,
      15,    1,   55,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Bool, 0x80000000 | 7, 0x80000000 | 7, 0x80000000 | 10,    8,    9,   11,
    QMetaType::Bool, QMetaType::QString, QMetaType::QString,   13,   14,
    QMetaType::Bool, QMetaType::QString,   13,

       0        // eod
};

void StorageManagment::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<StorageManagment *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->act1((*reinterpret_cast< UDEV_DEVICE_INFO_t(*)>(_a[1]))); break;
        case 1: _t->internalMsgServerLoop(); break;
        case 2: { bool _r = _t->mountDisk((*reinterpret_cast< CHARPTR(*)>(_a[1])),(*reinterpret_cast< CHARPTR(*)>(_a[2])),(*reinterpret_cast< RAW_MEDIA_FORMAT_TYPE_e(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 3: { bool _r = _t->mountDevice((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 4: { bool _r = _t->unmountDevice((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (StorageManagment::*)(UDEV_DEVICE_INFO_t );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StorageManagment::act1)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject StorageManagment::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_StorageManagment.data,
    qt_meta_data_StorageManagment,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *StorageManagment::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *StorageManagment::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_StorageManagment.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int StorageManagment::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void StorageManagment::act1(UDEV_DEVICE_INFO_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
