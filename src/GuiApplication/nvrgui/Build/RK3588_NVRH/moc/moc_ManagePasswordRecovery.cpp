/****************************************************************************
** Meta object code from reading C++ file 'ManagePasswordRecovery.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ManagePages/ManagePasswordRecovery.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ManagePasswordRecovery.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ManagePasswordRecovery_t {
    QByteArrayData data[11];
    char stringdata0[169];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ManagePasswordRecovery_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ManagePasswordRecovery_t qt_meta_stringdata_ManagePasswordRecovery = {
    {
QT_MOC_LITERAL(0, 0, 22), // "ManagePasswordRecovery"
QT_MOC_LITERAL(1, 23, 20), // "sigCancelbuttonClick"
QT_MOC_LITERAL(2, 44, 0), // ""
QT_MOC_LITERAL(3, 45, 19), // "slotTextBoxInfoPage"
QT_MOC_LITERAL(4, 65, 5), // "index"
QT_MOC_LITERAL(5, 71, 15), // "INFO_MSG_TYPE_e"
QT_MOC_LITERAL(6, 87, 7), // "msgType"
QT_MOC_LITERAL(7, 95, 16), // "slotTestBtnClick"
QT_MOC_LITERAL(8, 112, 22), // "slotSpinBoxValueChange"
QT_MOC_LITERAL(9, 135, 11), // "indexInPage"
QT_MOC_LITERAL(10, 147, 21) // "slotConfigButtonClick"

    },
    "ManagePasswordRecovery\0sigCancelbuttonClick\0"
    "\0slotTextBoxInfoPage\0index\0INFO_MSG_TYPE_e\0"
    "msgType\0slotTestBtnClick\0"
    "slotSpinBoxValueChange\0indexInPage\0"
    "slotConfigButtonClick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ManagePasswordRecovery[] = {

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
       3,    2,   40,    2, 0x0a /* Public */,
       7,    1,   45,    2, 0x0a /* Public */,
       8,    2,   48,    2, 0x0a /* Public */,
      10,    1,   53,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, 0x80000000 | 5,    4,    6,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    9,
    QMetaType::Void, QMetaType::Int,    4,

       0        // eod
};

void ManagePasswordRecovery::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ManagePasswordRecovery *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigCancelbuttonClick(); break;
        case 1: _t->slotTextBoxInfoPage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< INFO_MSG_TYPE_e(*)>(_a[2]))); break;
        case 2: _t->slotTestBtnClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotSpinBoxValueChange((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 4: _t->slotConfigButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ManagePasswordRecovery::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ManagePasswordRecovery::sigCancelbuttonClick)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ManagePasswordRecovery::staticMetaObject = { {
    QMetaObject::SuperData::link<ManageMenuOptions::staticMetaObject>(),
    qt_meta_stringdata_ManagePasswordRecovery.data,
    qt_meta_data_ManagePasswordRecovery,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ManagePasswordRecovery::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ManagePasswordRecovery::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ManagePasswordRecovery.stringdata0))
        return static_cast<void*>(this);
    return ManageMenuOptions::qt_metacast(_clname);
}

int ManagePasswordRecovery::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ManageMenuOptions::qt_metacall(_c, _id, _a);
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
void ManagePasswordRecovery::sigCancelbuttonClick()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
