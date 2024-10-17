/****************************************************************************
** Meta object code from reading C++ file 'TextWithList.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/TextWithList.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TextWithList.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TextWithList_t {
    QByteArrayData data[18];
    char stringdata0[252];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TextWithList_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TextWithList_t qt_meta_stringdata_TextWithList = {
    {
QT_MOC_LITERAL(0, 0, 12), // "TextWithList"
QT_MOC_LITERAL(1, 13, 23), // "sigUpdateCurrentElement"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 5), // "index"
QT_MOC_LITERAL(4, 44, 15), // "sigValueChanged"
QT_MOC_LITERAL(5, 60, 3), // "str"
QT_MOC_LITERAL(6, 64, 17), // "sigValueListEmpty"
QT_MOC_LITERAL(7, 82, 21), // "slotTextBoxKeyPressed"
QT_MOC_LITERAL(8, 104, 10), // "KEY_TYPE_e"
QT_MOC_LITERAL(9, 115, 7), // "keyType"
QT_MOC_LITERAL(10, 123, 23), // "slotLineEditFocusChange"
QT_MOC_LITERAL(11, 147, 9), // "isFocusIn"
QT_MOC_LITERAL(12, 157, 10), // "forceFocus"
QT_MOC_LITERAL(13, 168, 21), // "slotTextValueAppended"
QT_MOC_LITERAL(14, 190, 21), // "slotDropListDestroyed"
QT_MOC_LITERAL(15, 212, 16), // "slotValueChanged"
QT_MOC_LITERAL(16, 229, 3), // "key"
QT_MOC_LITERAL(17, 233, 18) // "slotUnloadDropList"

    },
    "TextWithList\0sigUpdateCurrentElement\0"
    "\0index\0sigValueChanged\0str\0sigValueListEmpty\0"
    "slotTextBoxKeyPressed\0KEY_TYPE_e\0"
    "keyType\0slotLineEditFocusChange\0"
    "isFocusIn\0forceFocus\0slotTextValueAppended\0"
    "slotDropListDestroyed\0slotValueChanged\0"
    "key\0slotUnloadDropList"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TextWithList[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x06 /* Public */,
       4,    2,   62,    2, 0x06 /* Public */,
       6,    1,   67,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    2,   70,    2, 0x0a /* Public */,
      10,    3,   75,    2, 0x0a /* Public */,
      13,    2,   82,    2, 0x0a /* Public */,
      14,    0,   87,    2, 0x0a /* Public */,
      15,    2,   88,    2, 0x0a /* Public */,
      17,    0,   93,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    5,    3,
    QMetaType::Void, QMetaType::UChar,    2,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 8, QMetaType::QString,    9,    5,
    QMetaType::Void, QMetaType::UChar, QMetaType::Bool, QMetaType::Bool,    3,   11,   12,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    5,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UChar, QMetaType::QString,   16,    5,
    QMetaType::Void,

       0        // eod
};

void TextWithList::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TextWithList *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->sigValueChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 2: _t->sigValueListEmpty((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 3: _t->slotTextBoxKeyPressed((*reinterpret_cast< KEY_TYPE_e(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 4: _t->slotLineEditFocusChange((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 5: _t->slotTextValueAppended((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->slotDropListDestroyed(); break;
        case 7: _t->slotValueChanged((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 8: _t->slotUnloadDropList(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TextWithList::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TextWithList::sigUpdateCurrentElement)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TextWithList::*)(QString , quint32 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TextWithList::sigValueChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (TextWithList::*)(quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TextWithList::sigValueListEmpty)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject TextWithList::staticMetaObject = { {
    QMetaObject::SuperData::link<BgTile::staticMetaObject>(),
    qt_meta_stringdata_TextWithList.data,
    qt_meta_data_TextWithList,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TextWithList::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TextWithList::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TextWithList.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return BgTile::qt_metacast(_clname);
}

int TextWithList::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BgTile::qt_metacall(_c, _id, _a);
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
void TextWithList::sigUpdateCurrentElement(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void TextWithList::sigValueChanged(QString _t1, quint32 _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void TextWithList::sigValueListEmpty(quint8 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
