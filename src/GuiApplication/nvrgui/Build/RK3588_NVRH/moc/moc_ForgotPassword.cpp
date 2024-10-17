/****************************************************************************
** Meta object code from reading C++ file 'ForgotPassword.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Login/ForgotPassword.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ForgotPassword.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ForgotPassword_t {
    QByteArrayData data[13];
    char stringdata0[228];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ForgotPassword_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ForgotPassword_t qt_meta_stringdata_ForgotPassword = {
    {
QT_MOC_LITERAL(0, 0, 14), // "ForgotPassword"
QT_MOC_LITERAL(1, 15, 11), // "sigExitPage"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 18), // "slotExitPwdRstPage"
QT_MOC_LITERAL(4, 47, 22), // "slotSpinBoxValueChange"
QT_MOC_LITERAL(5, 70, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(6, 95, 11), // "indexInPage"
QT_MOC_LITERAL(7, 107, 23), // "slotUpdateVerifyOtpTime"
QT_MOC_LITERAL(8, 131, 18), // "slotTextLableHover"
QT_MOC_LITERAL(9, 150, 9), // "isHoverIn"
QT_MOC_LITERAL(10, 160, 21), // "slotConfigButtonClick"
QT_MOC_LITERAL(11, 182, 20), // "slotcloseButtonClick"
QT_MOC_LITERAL(12, 203, 24) // "slotInfoPageCnfgBtnClick"

    },
    "ForgotPassword\0sigExitPage\0\0"
    "slotExitPwdRstPage\0slotSpinBoxValueChange\0"
    "slotUpdateCurrentElement\0indexInPage\0"
    "slotUpdateVerifyOtpTime\0slotTextLableHover\0"
    "isHoverIn\0slotConfigButtonClick\0"
    "slotcloseButtonClick\0slotInfoPageCnfgBtnClick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ForgotPassword[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   70,    2, 0x0a /* Public */,
       4,    2,   71,    2, 0x0a /* Public */,
       5,    1,   76,    2, 0x0a /* Public */,
       7,    0,   79,    2, 0x0a /* Public */,
       8,    2,   80,    2, 0x0a /* Public */,
       8,    1,   85,    2, 0x2a /* Public | MethodCloned */,
       8,    0,   88,    2, 0x2a /* Public | MethodCloned */,
      10,    1,   89,    2, 0x0a /* Public */,
      11,    1,   92,    2, 0x0a /* Public */,
      12,    1,   95,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    2,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    6,    9,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,

       0        // eod
};

void ForgotPassword::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ForgotPassword *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigExitPage(); break;
        case 1: _t->slotExitPwdRstPage(); break;
        case 2: _t->slotSpinBoxValueChange((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 3: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotUpdateVerifyOtpTime(); break;
        case 5: _t->slotTextLableHover((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 6: _t->slotTextLableHover((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->slotTextLableHover(); break;
        case 8: _t->slotConfigButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->slotcloseButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->slotInfoPageCnfgBtnClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ForgotPassword::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ForgotPassword::sigExitPage)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ForgotPassword::staticMetaObject = { {
    QMetaObject::SuperData::link<BackGround::staticMetaObject>(),
    qt_meta_stringdata_ForgotPassword.data,
    qt_meta_data_ForgotPassword,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ForgotPassword::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ForgotPassword::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ForgotPassword.stringdata0))
        return static_cast<void*>(this);
    return BackGround::qt_metacast(_clname);
}

int ForgotPassword::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BackGround::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void ForgotPassword::sigExitPage()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
