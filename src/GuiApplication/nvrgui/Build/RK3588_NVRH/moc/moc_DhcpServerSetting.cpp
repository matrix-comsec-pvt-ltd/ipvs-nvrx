/****************************************************************************
** Meta object code from reading C++ file 'DhcpServerSetting.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/NetworkSettings/DhcpServerSetting.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DhcpServerSetting.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DhcpClientList_t {
    QByteArrayData data[8];
    char stringdata0[124];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DhcpClientList_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DhcpClientList_t qt_meta_stringdata_DhcpClientList = {
    {
QT_MOC_LITERAL(0, 0, 14), // "DhcpClientList"
QT_MOC_LITERAL(1, 15, 15), // "sigObjectDelete"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 23), // "sigInfoPageCnfgBtnClick"
QT_MOC_LITERAL(4, 56, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(5, 81, 5), // "index"
QT_MOC_LITERAL(6, 87, 15), // "slotButtonClick"
QT_MOC_LITERAL(7, 103, 20) // "slotInfoPageBtnclick"

    },
    "DhcpClientList\0sigObjectDelete\0\0"
    "sigInfoPageCnfgBtnClick\0"
    "slotUpdateCurrentElement\0index\0"
    "slotButtonClick\0slotInfoPageBtnclick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DhcpClientList[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x06 /* Public */,
       3,    0,   40,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,   41,    2, 0x0a /* Public */,
       6,    1,   44,    2, 0x0a /* Public */,
       7,    1,   47,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    2,

       0        // eod
};

void DhcpClientList::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DhcpClientList *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigObjectDelete(); break;
        case 1: _t->sigInfoPageCnfgBtnClick(); break;
        case 2: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DhcpClientList::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DhcpClientList::sigObjectDelete)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DhcpClientList::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DhcpClientList::sigInfoPageCnfgBtnClick)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DhcpClientList::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_DhcpClientList.data,
    qt_meta_data_DhcpClientList,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DhcpClientList::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DhcpClientList::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DhcpClientList.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int DhcpClientList::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = KeyBoard::qt_metacall(_c, _id, _a);
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
void DhcpClientList::sigObjectDelete()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void DhcpClientList::sigInfoPageCnfgBtnClick()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
struct qt_meta_stringdata_DhcpServerSetting_t {
    QByteArrayData data[13];
    char stringdata0[193];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DhcpServerSetting_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DhcpServerSetting_t qt_meta_stringdata_DhcpServerSetting = {
    {
QT_MOC_LITERAL(0, 0, 17), // "DhcpServerSetting"
QT_MOC_LITERAL(1, 18, 23), // "slotEnableButtonClicked"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(4, 63, 5), // "state"
QT_MOC_LITERAL(5, 69, 5), // "index"
QT_MOC_LITERAL(6, 75, 28), // "slotInterfaceDropdownChanged"
QT_MOC_LITERAL(7, 104, 3), // "str"
QT_MOC_LITERAL(8, 108, 23), // "slotTextboxLoadInfopage"
QT_MOC_LITERAL(9, 132, 15), // "INFO_MSG_TYPE_e"
QT_MOC_LITERAL(10, 148, 7), // "msgType"
QT_MOC_LITERAL(11, 156, 15), // "slotButtonClick"
QT_MOC_LITERAL(12, 172, 20) // "slotClientListDelete"

    },
    "DhcpServerSetting\0slotEnableButtonClicked\0"
    "\0OPTION_STATE_TYPE_e\0state\0index\0"
    "slotInterfaceDropdownChanged\0str\0"
    "slotTextboxLoadInfopage\0INFO_MSG_TYPE_e\0"
    "msgType\0slotButtonClick\0slotClientListDelete"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DhcpServerSetting[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   39,    2, 0x0a /* Public */,
       6,    2,   44,    2, 0x0a /* Public */,
       8,    2,   49,    2, 0x0a /* Public */,
      11,    1,   54,    2, 0x0a /* Public */,
      12,    0,   57,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    7,    2,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 9,    5,   10,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,

       0        // eod
};

void DhcpServerSetting::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DhcpServerSetting *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotEnableButtonClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->slotInterfaceDropdownChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 2: _t->slotTextboxLoadInfopage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< INFO_MSG_TYPE_e(*)>(_a[2]))); break;
        case 3: _t->slotButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotClientListDelete(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DhcpServerSetting::staticMetaObject = { {
    QMetaObject::SuperData::link<ConfigPageControl::staticMetaObject>(),
    qt_meta_stringdata_DhcpServerSetting.data,
    qt_meta_data_DhcpServerSetting,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DhcpServerSetting::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DhcpServerSetting::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DhcpServerSetting.stringdata0))
        return static_cast<void*>(this);
    return ConfigPageControl::qt_metacast(_clname);
}

int DhcpServerSetting::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ConfigPageControl::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
