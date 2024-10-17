/****************************************************************************
** Meta object code from reading C++ file 'TextBox.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/TextBox.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TextBox.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TextBox_t {
    QByteArrayData data[18];
    char stringdata0[259];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TextBox_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TextBox_t qt_meta_stringdata_TextBox = {
    {
QT_MOC_LITERAL(0, 0, 7), // "TextBox"
QT_MOC_LITERAL(1, 8, 23), // "sigUpdateCurrentElement"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 5), // "index"
QT_MOC_LITERAL(4, 39, 15), // "sigLoadInfopage"
QT_MOC_LITERAL(5, 55, 15), // "INFO_MSG_TYPE_e"
QT_MOC_LITERAL(6, 71, 7), // "msgType"
QT_MOC_LITERAL(7, 79, 20), // "sigTextValueAppended"
QT_MOC_LITERAL(8, 100, 3), // "str"
QT_MOC_LITERAL(9, 104, 17), // "sigDoneKeyClicked"
QT_MOC_LITERAL(10, 122, 22), // "slotkeyRepTimerTimeout"
QT_MOC_LITERAL(11, 145, 23), // "slotLineEditFocusChange"
QT_MOC_LITERAL(12, 169, 9), // "isFocusIn"
QT_MOC_LITERAL(13, 179, 10), // "forceFocus"
QT_MOC_LITERAL(14, 190, 21), // "slotTextBoxKeyPressed"
QT_MOC_LITERAL(15, 212, 10), // "KEY_TYPE_e"
QT_MOC_LITERAL(16, 223, 7), // "keyType"
QT_MOC_LITERAL(17, 231, 27) // "slotInvisibleCtrlMouseClick"

    },
    "TextBox\0sigUpdateCurrentElement\0\0index\0"
    "sigLoadInfopage\0INFO_MSG_TYPE_e\0msgType\0"
    "sigTextValueAppended\0str\0sigDoneKeyClicked\0"
    "slotkeyRepTimerTimeout\0slotLineEditFocusChange\0"
    "isFocusIn\0forceFocus\0slotTextBoxKeyPressed\0"
    "KEY_TYPE_e\0keyType\0slotInvisibleCtrlMouseClick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TextBox[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,
       4,    2,   57,    2, 0x06 /* Public */,
       7,    2,   62,    2, 0x06 /* Public */,
       9,    1,   67,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    0,   70,    2, 0x0a /* Public */,
      11,    3,   71,    2, 0x0a /* Public */,
      14,    2,   78,    2, 0x0a /* Public */,
      17,    0,   83,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 5,    3,    6,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    8,    3,
    QMetaType::Void, QMetaType::Int,    3,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::UChar, QMetaType::Bool, QMetaType::Bool,    3,   12,   13,
    QMetaType::Void, 0x80000000 | 15, QMetaType::QString,   16,    8,
    QMetaType::Void,

       0        // eod
};

void TextBox::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TextBox *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->sigLoadInfopage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< INFO_MSG_TYPE_e(*)>(_a[2]))); break;
        case 2: _t->sigTextValueAppended((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->sigDoneKeyClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotkeyRepTimerTimeout(); break;
        case 5: _t->slotLineEditFocusChange((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 6: _t->slotTextBoxKeyPressed((*reinterpret_cast< KEY_TYPE_e(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 7: _t->slotInvisibleCtrlMouseClick(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TextBox::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TextBox::sigUpdateCurrentElement)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TextBox::*)(int , INFO_MSG_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TextBox::sigLoadInfopage)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (TextBox::*)(QString , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TextBox::sigTextValueAppended)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (TextBox::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TextBox::sigDoneKeyClicked)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject TextBox::staticMetaObject = { {
    QMetaObject::SuperData::link<BgTile::staticMetaObject>(),
    qt_meta_stringdata_TextBox.data,
    qt_meta_data_TextBox,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TextBox::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TextBox::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TextBox.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return BgTile::qt_metacast(_clname);
}

int TextBox::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void TextBox::sigUpdateCurrentElement(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void TextBox::sigLoadInfopage(int _t1, INFO_MSG_TYPE_e _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void TextBox::sigTextValueAppended(QString _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void TextBox::sigDoneKeyClicked(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
