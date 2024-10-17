/****************************************************************************
** Meta object code from reading C++ file 'ManagePages.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ManagePages.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ManagePages.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ManagePages_t {
    QByteArrayData data[14];
    char stringdata0[228];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ManagePages_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ManagePages_t qt_meta_stringdata_ManagePages = {
    {
QT_MOC_LITERAL(0, 0, 11), // "ManagePages"
QT_MOC_LITERAL(1, 12, 14), // "sigButtonClick"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 5), // "index"
QT_MOC_LITERAL(4, 34, 16), // "sigDevNameChange"
QT_MOC_LITERAL(5, 51, 4), // "name"
QT_MOC_LITERAL(6, 56, 20), // "slotSubHeadingChange"
QT_MOC_LITERAL(7, 77, 17), // "slotManageOptions"
QT_MOC_LITERAL(8, 95, 22), // "slotSpinBoxValueChange"
QT_MOC_LITERAL(9, 118, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(10, 143, 20), // "slotInfoPageBtnclick"
QT_MOC_LITERAL(11, 164, 23), // "slotFocusToOtherElement"
QT_MOC_LITERAL(12, 188, 17), // "isPrevoiusElement"
QT_MOC_LITERAL(13, 206, 21) // "slotCancelbuttonClick"

    },
    "ManagePages\0sigButtonClick\0\0index\0"
    "sigDevNameChange\0name\0slotSubHeadingChange\0"
    "slotManageOptions\0slotSpinBoxValueChange\0"
    "slotUpdateCurrentElement\0slotInfoPageBtnclick\0"
    "slotFocusToOtherElement\0isPrevoiusElement\0"
    "slotCancelbuttonClick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ManagePages[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x06 /* Public */,
       4,    1,   62,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   65,    2, 0x0a /* Public */,
       7,    1,   68,    2, 0x0a /* Public */,
       8,    2,   71,    2, 0x0a /* Public */,
       9,    1,   76,    2, 0x0a /* Public */,
      10,    1,   79,    2, 0x0a /* Public */,
      11,    1,   82,    2, 0x0a /* Public */,
      13,    0,   85,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::QString,    5,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    5,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Bool,   12,
    QMetaType::Void,

       0        // eod
};

void ManagePages::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ManagePages *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->sigDevNameChange((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->slotSubHeadingChange((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->slotManageOptions((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotSpinBoxValueChange((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 5: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->slotFocusToOtherElement((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->slotCancelbuttonClick(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ManagePages::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ManagePages::sigButtonClick)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ManagePages::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ManagePages::sigDevNameChange)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ManagePages::staticMetaObject = { {
    QMetaObject::SuperData::link<BackGround::staticMetaObject>(),
    qt_meta_stringdata_ManagePages.data,
    qt_meta_data_ManagePages,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ManagePages::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ManagePages::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ManagePages.stringdata0))
        return static_cast<void*>(this);
    return BackGround::qt_metacast(_clname);
}

int ManagePages::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BackGround::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void ManagePages::sigButtonClick(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ManagePages::sigDevNameChange(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
