/****************************************************************************
** Meta object code from reading C++ file 'BackupSingleRecord.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/PlaybackControl/BackupSingleRecord.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'BackupSingleRecord.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_BackupSingleRecord_t {
    QByteArrayData data[8];
    char stringdata0[137];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_BackupSingleRecord_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_BackupSingleRecord_t qt_meta_stringdata_BackupSingleRecord = {
    {
QT_MOC_LITERAL(0, 0, 18), // "BackupSingleRecord"
QT_MOC_LITERAL(1, 19, 28), // "sigBackUpSingleRecBtnClicked"
QT_MOC_LITERAL(2, 48, 0), // ""
QT_MOC_LITERAL(3, 49, 5), // "index"
QT_MOC_LITERAL(4, 55, 31), // "slotClockSpinboxTotalCurrentSec"
QT_MOC_LITERAL(5, 87, 7), // "currSec"
QT_MOC_LITERAL(6, 95, 15), // "slotButtonClick"
QT_MOC_LITERAL(7, 111, 25) // "slotUpadateCurrentElement"

    },
    "BackupSingleRecord\0sigBackUpSingleRecBtnClicked\0"
    "\0index\0slotClockSpinboxTotalCurrentSec\0"
    "currSec\0slotButtonClick\0"
    "slotUpadateCurrentElement"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_BackupSingleRecord[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    2,   37,    2, 0x0a /* Public */,
       6,    1,   42,    2, 0x0a /* Public */,
       7,    1,   45,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,

 // slots: parameters
    QMetaType::Void, QMetaType::UShort, QMetaType::Int,    5,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,

       0        // eod
};

void BackupSingleRecord::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<BackupSingleRecord *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigBackUpSingleRecBtnClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->slotClockSpinboxTotalCurrentSec((*reinterpret_cast< quint16(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->slotButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotUpadateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (BackupSingleRecord::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&BackupSingleRecord::sigBackUpSingleRecBtnClicked)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject BackupSingleRecord::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_BackupSingleRecord.data,
    qt_meta_data_BackupSingleRecord,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *BackupSingleRecord::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BackupSingleRecord::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_BackupSingleRecord.stringdata0))
        return static_cast<void*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int BackupSingleRecord::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = KeyBoard::qt_metacall(_c, _id, _a);
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
void BackupSingleRecord::sigBackUpSingleRecBtnClicked(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
