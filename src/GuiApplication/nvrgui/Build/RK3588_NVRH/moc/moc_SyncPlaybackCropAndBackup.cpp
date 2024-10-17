/****************************************************************************
** Meta object code from reading C++ file 'SyncPlaybackCropAndBackup.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/SyncPlayback/SyncPlaybackCropAndBackup.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SyncPlaybackCropAndBackup.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SyncPlaybackCropAndBackup_t {
    QByteArrayData data[10];
    char stringdata0[193];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SyncPlaybackCropAndBackup_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SyncPlaybackCropAndBackup_t qt_meta_stringdata_SyncPlaybackCropAndBackup = {
    {
QT_MOC_LITERAL(0, 0, 25), // "SyncPlaybackCropAndBackup"
QT_MOC_LITERAL(1, 26, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(2, 51, 0), // ""
QT_MOC_LITERAL(3, 52, 5), // "index"
QT_MOC_LITERAL(4, 58, 21), // "slotCnfgButtonClicked"
QT_MOC_LITERAL(5, 80, 25), // "slotInfoPageButtonClicked"
QT_MOC_LITERAL(6, 106, 23), // "slotRemoveButtonClicked"
QT_MOC_LITERAL(7, 130, 27), // "slotNavigationButtonClicked"
QT_MOC_LITERAL(8, 158, 22), // "slotCloseButtonClicked"
QT_MOC_LITERAL(9, 181, 11) // "indexInPage"

    },
    "SyncPlaybackCropAndBackup\0"
    "slotUpdateCurrentElement\0\0index\0"
    "slotCnfgButtonClicked\0slotInfoPageButtonClicked\0"
    "slotRemoveButtonClicked\0"
    "slotNavigationButtonClicked\0"
    "slotCloseButtonClicked\0indexInPage"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SyncPlaybackCropAndBackup[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x0a /* Public */,
       4,    1,   47,    2, 0x0a /* Public */,
       5,    1,   50,    2, 0x0a /* Public */,
       6,    1,   53,    2, 0x0a /* Public */,
       7,    1,   56,    2, 0x0a /* Public */,
       8,    1,   59,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    9,

       0        // eod
};

void SyncPlaybackCropAndBackup::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SyncPlaybackCropAndBackup *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->slotCnfgButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slotInfoPageButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotRemoveButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotNavigationButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->slotCloseButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SyncPlaybackCropAndBackup::staticMetaObject = { {
    QMetaObject::SuperData::link<BackGround::staticMetaObject>(),
    qt_meta_stringdata_SyncPlaybackCropAndBackup.data,
    qt_meta_data_SyncPlaybackCropAndBackup,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SyncPlaybackCropAndBackup::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SyncPlaybackCropAndBackup::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SyncPlaybackCropAndBackup.stringdata0))
        return static_cast<void*>(this);
    return BackGround::qt_metacast(_clname);
}

int SyncPlaybackCropAndBackup::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BackGround::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
