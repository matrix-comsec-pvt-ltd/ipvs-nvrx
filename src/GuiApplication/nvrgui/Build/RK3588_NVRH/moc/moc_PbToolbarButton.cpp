/****************************************************************************
** Meta object code from reading C++ file 'PbToolbarButton.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/PlaybackControl/PbToolbarButton.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PbToolbarButton.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PbToolbarButton_t {
    QByteArrayData data[8];
    char stringdata0[120];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PbToolbarButton_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PbToolbarButton_t qt_meta_stringdata_PbToolbarButton = {
    {
QT_MOC_LITERAL(0, 0, 15), // "PbToolbarButton"
QT_MOC_LITERAL(1, 16, 14), // "sigButtonClick"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 11), // "indexInPage"
QT_MOC_LITERAL(4, 44, 23), // "sigUpdateCurrentElement"
QT_MOC_LITERAL(5, 68, 5), // "index"
QT_MOC_LITERAL(6, 74, 18), // "sigImageMouseHover"
QT_MOC_LITERAL(7, 93, 26) // "slotclickeffctTimerTimeout"

    },
    "PbToolbarButton\0sigButtonClick\0\0"
    "indexInPage\0sigUpdateCurrentElement\0"
    "index\0sigImageMouseHover\0"
    "slotclickeffctTimerTimeout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PbToolbarButton[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x06 /* Public */,
       4,    1,   37,    2, 0x06 /* Public */,
       6,    2,   40,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    0,   45,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::UChar, QMetaType::Bool,    2,    2,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void PbToolbarButton::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PbToolbarButton *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->sigUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->sigImageMouseHover((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 3: _t->slotclickeffctTimerTimeout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PbToolbarButton::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PbToolbarButton::sigButtonClick)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PbToolbarButton::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PbToolbarButton::sigUpdateCurrentElement)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PbToolbarButton::*)(quint8 , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PbToolbarButton::sigImageMouseHover)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PbToolbarButton::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_PbToolbarButton.data,
    qt_meta_data_PbToolbarButton,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PbToolbarButton::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PbToolbarButton::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PbToolbarButton.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int PbToolbarButton::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void PbToolbarButton::sigButtonClick(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PbToolbarButton::sigUpdateCurrentElement(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PbToolbarButton::sigImageMouseHover(quint8 _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
