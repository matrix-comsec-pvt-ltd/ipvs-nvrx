/****************************************************************************
** Meta object code from reading C++ file 'PickList.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/PickList.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PickList.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PickList_t {
    QByteArrayData data[16];
    char stringdata0[214];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PickList_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PickList_t qt_meta_stringdata_PickList = {
    {
QT_MOC_LITERAL(0, 0, 8), // "PickList"
QT_MOC_LITERAL(1, 9, 23), // "sigUpdateCurrentElement"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 5), // "index"
QT_MOC_LITERAL(4, 40, 15), // "sigValueChanged"
QT_MOC_LITERAL(5, 56, 3), // "key"
QT_MOC_LITERAL(6, 60, 5), // "value"
QT_MOC_LITERAL(7, 66, 11), // "indexInPage"
QT_MOC_LITERAL(8, 78, 14), // "sigButtonClick"
QT_MOC_LITERAL(9, 93, 15), // "sigPicklistLoad"
QT_MOC_LITERAL(10, 109, 18), // "sigShowHideToolTip"
QT_MOC_LITERAL(11, 128, 13), // "toShowTooltip"
QT_MOC_LITERAL(12, 142, 16), // "slotValueChanged"
QT_MOC_LITERAL(13, 159, 13), // "isCancleClick"
QT_MOC_LITERAL(14, 173, 21), // "slotPickListDestroyed"
QT_MOC_LITERAL(15, 195, 18) // "slotDeletePickList"

    },
    "PickList\0sigUpdateCurrentElement\0\0"
    "index\0sigValueChanged\0key\0value\0"
    "indexInPage\0sigButtonClick\0sigPicklistLoad\0"
    "sigShowHideToolTip\0toShowTooltip\0"
    "slotValueChanged\0isCancleClick\0"
    "slotPickListDestroyed\0slotDeletePickList"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PickList[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,
       4,    3,   57,    2, 0x06 /* Public */,
       8,    1,   64,    2, 0x06 /* Public */,
       9,    1,   67,    2, 0x06 /* Public */,
      10,    2,   70,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      12,    3,   75,    2, 0x0a /* Public */,
      14,    0,   82,    2, 0x0a /* Public */,
      15,    0,   83,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::UChar, QMetaType::QString, QMetaType::Int,    5,    6,    7,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::UChar,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    3,   11,

 // slots: parameters
    QMetaType::Void, QMetaType::UChar, QMetaType::QString, QMetaType::Bool,    5,    6,   13,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void PickList::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PickList *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->sigValueChanged((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: _t->sigButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->sigPicklistLoad((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 4: _t->sigShowHideToolTip((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 5: _t->slotValueChanged((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 6: _t->slotPickListDestroyed(); break;
        case 7: _t->slotDeletePickList(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PickList::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PickList::sigUpdateCurrentElement)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PickList::*)(quint8 , QString , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PickList::sigValueChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PickList::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PickList::sigButtonClick)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (PickList::*)(quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PickList::sigPicklistLoad)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (PickList::*)(int , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PickList::sigShowHideToolTip)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PickList::staticMetaObject = { {
    QMetaObject::SuperData::link<BgTile::staticMetaObject>(),
    qt_meta_stringdata_PickList.data,
    qt_meta_data_PickList,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PickList::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PickList::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PickList.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return BgTile::qt_metacast(_clname);
}

int PickList::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BgTile::qt_metacall(_c, _id, _a);
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
void PickList::sigUpdateCurrentElement(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PickList::sigValueChanged(quint8 _t1, QString _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PickList::sigButtonClick(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void PickList::sigPicklistLoad(quint8 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void PickList::sigShowHideToolTip(int _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
