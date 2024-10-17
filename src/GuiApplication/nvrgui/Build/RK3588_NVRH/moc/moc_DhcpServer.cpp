/****************************************************************************
** Meta object code from reading C++ file 'DhcpServer.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Wizard/DhcpServer.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DhcpServer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DhcpClientStatus_t {
    QByteArrayData data[8];
    char stringdata0[126];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DhcpClientStatus_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DhcpClientStatus_t qt_meta_stringdata_DhcpClientStatus = {
    {
QT_MOC_LITERAL(0, 0, 16), // "DhcpClientStatus"
QT_MOC_LITERAL(1, 17, 15), // "sigObjectDelete"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 23), // "sigInfoPageCnfgBtnClick"
QT_MOC_LITERAL(4, 58, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(5, 83, 5), // "index"
QT_MOC_LITERAL(6, 89, 15), // "slotButtonClick"
QT_MOC_LITERAL(7, 105, 20) // "slotInfoPageBtnclick"

    },
    "DhcpClientStatus\0sigObjectDelete\0\0"
    "sigInfoPageCnfgBtnClick\0"
    "slotUpdateCurrentElement\0index\0"
    "slotButtonClick\0slotInfoPageBtnclick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DhcpClientStatus[] = {

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
    QMetaType::Void, QMetaType::Int,    5,

       0        // eod
};

void DhcpClientStatus::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DhcpClientStatus *>(_o);
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
            using _t = void (DhcpClientStatus::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DhcpClientStatus::sigObjectDelete)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DhcpClientStatus::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DhcpClientStatus::sigInfoPageCnfgBtnClick)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DhcpClientStatus::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_DhcpClientStatus.data,
    qt_meta_data_DhcpClientStatus,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DhcpClientStatus::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DhcpClientStatus::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DhcpClientStatus.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int DhcpClientStatus::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void DhcpClientStatus::sigObjectDelete()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void DhcpClientStatus::sigInfoPageCnfgBtnClick()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
struct qt_meta_stringdata_DhcpServer_t {
    QByteArrayData data[14];
    char stringdata0[207];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DhcpServer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DhcpServer_t qt_meta_stringdata_DhcpServer = {
    {
QT_MOC_LITERAL(0, 0, 10), // "DhcpServer"
QT_MOC_LITERAL(1, 11, 23), // "slotEnableButtonClicked"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(4, 56, 5), // "state"
QT_MOC_LITERAL(5, 62, 5), // "index"
QT_MOC_LITERAL(6, 68, 28), // "slotInterfaceDropdownChanged"
QT_MOC_LITERAL(7, 97, 3), // "str"
QT_MOC_LITERAL(8, 101, 23), // "slotTextboxLoadInfopage"
QT_MOC_LITERAL(9, 125, 15), // "INFO_MSG_TYPE_e"
QT_MOC_LITERAL(10, 141, 7), // "msgType"
QT_MOC_LITERAL(11, 149, 15), // "slotButtonClick"
QT_MOC_LITERAL(12, 165, 20), // "slotClientListDelete"
QT_MOC_LITERAL(13, 186, 20) // "slotInfoPageBtnclick"

    },
    "DhcpServer\0slotEnableButtonClicked\0\0"
    "OPTION_STATE_TYPE_e\0state\0index\0"
    "slotInterfaceDropdownChanged\0str\0"
    "slotTextboxLoadInfopage\0INFO_MSG_TYPE_e\0"
    "msgType\0slotButtonClick\0slotClientListDelete\0"
    "slotInfoPageBtnclick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DhcpServer[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   44,    2, 0x0a /* Public */,
       6,    2,   49,    2, 0x0a /* Public */,
       8,    2,   54,    2, 0x0a /* Public */,
      11,    1,   59,    2, 0x0a /* Public */,
      12,    0,   62,    2, 0x0a /* Public */,
      13,    1,   63,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    7,    5,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 9,    5,   10,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,

       0        // eod
};

void DhcpServer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DhcpServer *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotEnableButtonClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->slotInterfaceDropdownChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 2: _t->slotTextboxLoadInfopage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< INFO_MSG_TYPE_e(*)>(_a[2]))); break;
        case 3: _t->slotButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotClientListDelete(); break;
        case 5: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DhcpServer::staticMetaObject = { {
    QMetaObject::SuperData::link<WizardCommon::staticMetaObject>(),
    qt_meta_stringdata_DhcpServer.data,
    qt_meta_data_DhcpServer,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DhcpServer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DhcpServer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DhcpServer.stringdata0))
        return static_cast<void*>(this);
    return WizardCommon::qt_metacast(_clname);
}

int DhcpServer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = WizardCommon::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
