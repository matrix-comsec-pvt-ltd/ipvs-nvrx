/****************************************************************************
** Meta object code from reading C++ file 'PasswordTextbox.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/PasswordTextbox.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PasswordTextbox.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PasswordTextbox_t {
    QByteArrayData data[3];
    char stringdata0[41];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PasswordTextbox_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PasswordTextbox_t qt_meta_stringdata_PasswordTextbox = {
    {
QT_MOC_LITERAL(0, 0, 15), // "PasswordTextbox"
QT_MOC_LITERAL(1, 16, 23), // "slotRefreshTimerTimeout"
QT_MOC_LITERAL(2, 40, 0) // ""

    },
    "PasswordTextbox\0slotRefreshTimerTimeout\0"
    ""
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PasswordTextbox[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void PasswordTextbox::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PasswordTextbox *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotRefreshTimerTimeout(); break;
        default: ;
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject PasswordTextbox::staticMetaObject = { {
    QMetaObject::SuperData::link<TextBox::staticMetaObject>(),
    qt_meta_stringdata_PasswordTextbox.data,
    qt_meta_data_PasswordTextbox,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PasswordTextbox::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PasswordTextbox::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PasswordTextbox.stringdata0))
        return static_cast<void*>(this);
    return TextBox::qt_metacast(_clname);
}

int PasswordTextbox::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = TextBox::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
