/****************************************************************************
** Meta object code from reading C++ file 'IpAddressChange.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/CameraSettings/IpAddressChange.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'IpAddressChange.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_IpAddressChange_t {
    QByteArrayData data[11];
    char stringdata0[203];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_IpAddressChange_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_IpAddressChange_t qt_meta_stringdata_IpAddressChange = {
    {
QT_MOC_LITERAL(0, 0, 15), // "IpAddressChange"
QT_MOC_LITERAL(1, 16, 15), // "sigObjectDelete"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 26), // "sigDataSelectedForIpChange"
QT_MOC_LITERAL(4, 60, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(5, 85, 17), // "slotButtonClicked"
QT_MOC_LITERAL(6, 103, 5), // "index"
QT_MOC_LITERAL(7, 109, 25), // "slotIpTextBoxLoadInfopage"
QT_MOC_LITERAL(8, 135, 20), // "slotInfoPageBtnclick"
QT_MOC_LITERAL(9, 156, 22), // "slotIpTextBoxEntryDone"
QT_MOC_LITERAL(10, 179, 23) // "slotSpinboxValueChanged"

    },
    "IpAddressChange\0sigObjectDelete\0\0"
    "sigDataSelectedForIpChange\0"
    "slotUpdateCurrentElement\0slotButtonClicked\0"
    "index\0slotIpTextBoxLoadInfopage\0"
    "slotInfoPageBtnclick\0slotIpTextBoxEntryDone\0"
    "slotSpinboxValueChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_IpAddressChange[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   54,    2, 0x06 /* Public */,
       3,    3,   55,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,   62,    2, 0x0a /* Public */,
       5,    1,   65,    2, 0x0a /* Public */,
       7,    1,   68,    2, 0x0a /* Public */,
       8,    1,   71,    2, 0x0a /* Public */,
       9,    1,   74,    2, 0x0a /* Public */,
      10,    2,   77,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString,    2,    2,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::UInt,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::UInt,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    2,

       0        // eod
};

void IpAddressChange::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<IpAddressChange *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigObjectDelete(); break;
        case 1: _t->sigDataSelectedForIpChange((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 2: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotIpTextBoxLoadInfopage((*reinterpret_cast< quint32(*)>(_a[1]))); break;
        case 5: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->slotIpTextBoxEntryDone((*reinterpret_cast< quint32(*)>(_a[1]))); break;
        case 7: _t->slotSpinboxValueChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (IpAddressChange::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IpAddressChange::sigObjectDelete)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (IpAddressChange::*)(QString , QString , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IpAddressChange::sigDataSelectedForIpChange)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject IpAddressChange::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_IpAddressChange.data,
    qt_meta_data_IpAddressChange,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *IpAddressChange::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *IpAddressChange::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_IpAddressChange.stringdata0))
        return static_cast<void*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int IpAddressChange::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = KeyBoard::qt_metacall(_c, _id, _a);
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
void IpAddressChange::sigObjectDelete()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void IpAddressChange::sigDataSelectedForIpChange(QString _t1, QString _t2, QString _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
