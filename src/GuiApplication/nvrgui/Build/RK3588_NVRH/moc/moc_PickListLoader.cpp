/****************************************************************************
** Meta object code from reading C++ file 'PickListLoader.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/PickListLoader.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PickListLoader.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PickListLoader_t {
    QByteArrayData data[13];
    char stringdata0[169];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PickListLoader_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PickListLoader_t qt_meta_stringdata_PickListLoader = {
    {
QT_MOC_LITERAL(0, 0, 14), // "PickListLoader"
QT_MOC_LITERAL(1, 15, 15), // "sigValueChanged"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 3), // "key"
QT_MOC_LITERAL(4, 36, 5), // "value"
QT_MOC_LITERAL(5, 42, 13), // "isCancelClick"
QT_MOC_LITERAL(6, 56, 22), // "slotCloseButtonClicked"
QT_MOC_LITERAL(7, 79, 11), // "indexInPage"
QT_MOC_LITERAL(8, 91, 21), // "slotMenuButtonClicked"
QT_MOC_LITERAL(9, 113, 5), // "index"
QT_MOC_LITERAL(10, 119, 10), // "slotScroll"
QT_MOC_LITERAL(11, 130, 13), // "numberOfSteps"
QT_MOC_LITERAL(12, 144, 24) // "slotUpdateCurrentElement"

    },
    "PickListLoader\0sigValueChanged\0\0key\0"
    "value\0isCancelClick\0slotCloseButtonClicked\0"
    "indexInPage\0slotMenuButtonClicked\0"
    "index\0slotScroll\0numberOfSteps\0"
    "slotUpdateCurrentElement"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PickListLoader[] = {

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
       1,    3,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   46,    2, 0x0a /* Public */,
       8,    1,   49,    2, 0x0a /* Public */,
      10,    1,   52,    2, 0x0a /* Public */,
      12,    1,   55,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::UChar, QMetaType::QString, QMetaType::Bool,    3,    4,    5,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, QMetaType::Int,   11,
    QMetaType::Void, QMetaType::Int,    9,

       0        // eod
};

void PickListLoader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PickListLoader *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigValueChanged((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 1: _t->slotCloseButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slotMenuButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotScroll((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PickListLoader::*)(quint8 , QString , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PickListLoader::sigValueChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PickListLoader::staticMetaObject = { {
    QMetaObject::SuperData::link<BackGround::staticMetaObject>(),
    qt_meta_stringdata_PickListLoader.data,
    qt_meta_data_PickListLoader,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PickListLoader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PickListLoader::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PickListLoader.stringdata0))
        return static_cast<void*>(this);
    return BackGround::qt_metacast(_clname);
}

int PickListLoader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void PickListLoader::sigValueChanged(quint8 _t1, QString _t2, bool _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
