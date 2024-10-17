/****************************************************************************
** Meta object code from reading C++ file 'PasswordReset.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Login/PasswordReset.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PasswordReset.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PasswordReset_t {
    QByteArrayData data[8];
    char stringdata0[132];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PasswordReset_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PasswordReset_t qt_meta_stringdata_PasswordReset = {
    {
QT_MOC_LITERAL(0, 0, 13), // "PasswordReset"
QT_MOC_LITERAL(1, 14, 17), // "sigExitPwdRstPage"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(4, 58, 11), // "indexInPage"
QT_MOC_LITERAL(5, 70, 20), // "slotcloseButtonClick"
QT_MOC_LITERAL(6, 91, 15), // "slotButtonClick"
QT_MOC_LITERAL(7, 107, 24) // "slotInfoPageCnfgBtnClick"

    },
    "PasswordReset\0sigExitPwdRstPage\0\0"
    "slotUpdateCurrentElement\0indexInPage\0"
    "slotcloseButtonClick\0slotButtonClick\0"
    "slotInfoPageCnfgBtnClick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PasswordReset[] = {

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
       1,    0,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    1,   40,    2, 0x0a /* Public */,
       5,    1,   43,    2, 0x0a /* Public */,
       6,    1,   46,    2, 0x0a /* Public */,
       7,    1,   49,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,

       0        // eod
};

void PasswordReset::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PasswordReset *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigExitPwdRstPage(); break;
        case 1: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slotcloseButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotInfoPageCnfgBtnClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PasswordReset::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PasswordReset::sigExitPwdRstPage)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PasswordReset::staticMetaObject = { {
    QMetaObject::SuperData::link<BackGround::staticMetaObject>(),
    qt_meta_stringdata_PasswordReset.data,
    qt_meta_data_PasswordReset,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PasswordReset::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PasswordReset::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PasswordReset.stringdata0))
        return static_cast<void*>(this);
    return BackGround::qt_metacast(_clname);
}

int PasswordReset::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BackGround::qt_metacall(_c, _id, _a);
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
void PasswordReset::sigExitPwdRstPage()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
