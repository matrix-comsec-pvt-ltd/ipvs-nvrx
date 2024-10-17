/****************************************************************************
** Meta object code from reading C++ file 'RecordRetention.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/StorageAndBackupSettings/RecordRetention.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RecordRetention.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_RecordRetention_t {
    QByteArrayData data[9];
    char stringdata0[154];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_RecordRetention_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_RecordRetention_t qt_meta_stringdata_RecordRetention = {
    {
QT_MOC_LITERAL(0, 0, 15), // "RecordRetention"
QT_MOC_LITERAL(1, 16, 15), // "sigDeleteObject"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 15), // "slotButtonClick"
QT_MOC_LITERAL(4, 49, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(5, 74, 20), // "slotInfoPageBtnclick"
QT_MOC_LITERAL(6, 95, 16), // "slotLoadInfoPage"
QT_MOC_LITERAL(7, 112, 15), // "INFO_MSG_TYPE_e"
QT_MOC_LITERAL(8, 128, 25) // "slotPageNumberButtonClick"

    },
    "RecordRetention\0sigDeleteObject\0\0"
    "slotButtonClick\0slotUpdateCurrentElement\0"
    "slotInfoPageBtnclick\0slotLoadInfoPage\0"
    "INFO_MSG_TYPE_e\0slotPageNumberButtonClick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_RecordRetention[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    1,   47,    2, 0x0a /* Public */,
       4,    1,   50,    2, 0x0a /* Public */,
       5,    1,   53,    2, 0x0a /* Public */,
       6,    2,   56,    2, 0x0a /* Public */,
       8,    1,   61,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::UChar,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 7,    2,    2,
    QMetaType::Void, QMetaType::QString,    2,

       0        // eod
};

void RecordRetention::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<RecordRetention *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigDeleteObject((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 1: _t->slotButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotLoadInfoPage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< INFO_MSG_TYPE_e(*)>(_a[2]))); break;
        case 5: _t->slotPageNumberButtonClick((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (RecordRetention::*)(quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&RecordRetention::sigDeleteObject)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject RecordRetention::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_RecordRetention.data,
    qt_meta_data_RecordRetention,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *RecordRetention::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RecordRetention::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_RecordRetention.stringdata0))
        return static_cast<void*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int RecordRetention::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void RecordRetention::sigDeleteObject(quint8 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
