/****************************************************************************
** Meta object code from reading C++ file 'AdvanceCameraSearch.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/CameraSettings/AdvanceCameraSearch.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AdvanceCameraSearch.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AdvanceCameraSearch_t {
    QByteArrayData data[13];
    char stringdata0[227];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AdvanceCameraSearch_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AdvanceCameraSearch_t qt_meta_stringdata_AdvanceCameraSearch = {
    {
QT_MOC_LITERAL(0, 0, 19), // "AdvanceCameraSearch"
QT_MOC_LITERAL(1, 20, 19), // "sigCreateCMDRequest"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 13), // "SET_COMMAND_e"
QT_MOC_LITERAL(4, 55, 23), // "sigAdvanceSearchRequest"
QT_MOC_LITERAL(5, 79, 15), // "sigObjectDelete"
QT_MOC_LITERAL(6, 95, 15), // "slotButtonClick"
QT_MOC_LITERAL(7, 111, 16), // "slotValueChanged"
QT_MOC_LITERAL(8, 128, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(9, 153, 20), // "slotInfoPageBtnclick"
QT_MOC_LITERAL(10, 174, 15), // "slotIpStartDone"
QT_MOC_LITERAL(11, 190, 13), // "slotIpEndDone"
QT_MOC_LITERAL(12, 204, 22) // "slotIpTextLoadInfoPage"

    },
    "AdvanceCameraSearch\0sigCreateCMDRequest\0"
    "\0SET_COMMAND_e\0sigAdvanceSearchRequest\0"
    "sigObjectDelete\0slotButtonClick\0"
    "slotValueChanged\0slotUpdateCurrentElement\0"
    "slotInfoPageBtnclick\0slotIpStartDone\0"
    "slotIpEndDone\0slotIpTextLoadInfoPage"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AdvanceCameraSearch[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   64,    2, 0x06 /* Public */,
       4,    3,   69,    2, 0x06 /* Public */,
       5,    1,   76,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   79,    2, 0x0a /* Public */,
       7,    3,   82,    2, 0x0a /* Public */,
       8,    1,   89,    2, 0x0a /* Public */,
       9,    1,   92,    2, 0x0a /* Public */,
      10,    1,   95,    2, 0x0a /* Public */,
      11,    1,   98,    2, 0x0a /* Public */,
      12,    1,  101,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::UChar,    2,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString,    2,    2,    2,
    QMetaType::Void, QMetaType::Bool,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::UChar, QMetaType::QString, QMetaType::Int,    2,    2,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::UInt,    2,
    QMetaType::Void, QMetaType::UInt,    2,
    QMetaType::Void, QMetaType::UInt,    2,

       0        // eod
};

void AdvanceCameraSearch::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AdvanceCameraSearch *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigCreateCMDRequest((*reinterpret_cast< SET_COMMAND_e(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2]))); break;
        case 1: _t->sigAdvanceSearchRequest((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 2: _t->sigObjectDelete((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->slotButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotValueChanged((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 5: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->slotIpStartDone((*reinterpret_cast< quint32(*)>(_a[1]))); break;
        case 8: _t->slotIpEndDone((*reinterpret_cast< quint32(*)>(_a[1]))); break;
        case 9: _t->slotIpTextLoadInfoPage((*reinterpret_cast< quint32(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AdvanceCameraSearch::*)(SET_COMMAND_e , quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AdvanceCameraSearch::sigCreateCMDRequest)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (AdvanceCameraSearch::*)(QString , QString , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AdvanceCameraSearch::sigAdvanceSearchRequest)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (AdvanceCameraSearch::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AdvanceCameraSearch::sigObjectDelete)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject AdvanceCameraSearch::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_AdvanceCameraSearch.data,
    qt_meta_data_AdvanceCameraSearch,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *AdvanceCameraSearch::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AdvanceCameraSearch::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AdvanceCameraSearch.stringdata0))
        return static_cast<void*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int AdvanceCameraSearch::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = KeyBoard::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void AdvanceCameraSearch::sigCreateCMDRequest(SET_COMMAND_e _t1, quint8 _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void AdvanceCameraSearch::sigAdvanceSearchRequest(QString _t1, QString _t2, QString _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void AdvanceCameraSearch::sigObjectDelete(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
