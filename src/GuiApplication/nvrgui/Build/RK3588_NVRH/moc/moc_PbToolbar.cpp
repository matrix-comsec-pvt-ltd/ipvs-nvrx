/****************************************************************************
** Meta object code from reading C++ file 'PbToolbar.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/PlaybackControl/PbToolbar.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PbToolbar.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PbToolbar_t {
    QByteArrayData data[16];
    char stringdata0[236];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PbToolbar_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PbToolbar_t qt_meta_stringdata_PbToolbar = {
    {
QT_MOC_LITERAL(0, 0, 9), // "PbToolbar"
QT_MOC_LITERAL(1, 10, 21), // "sigSliderValueChanged"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 8), // "currTime"
QT_MOC_LITERAL(4, 42, 8), // "windowId"
QT_MOC_LITERAL(5, 51, 20), // "sigPbToolbarBtnClick"
QT_MOC_LITERAL(6, 72, 5), // "index"
QT_MOC_LITERAL(7, 78, 5), // "state"
QT_MOC_LITERAL(8, 84, 20), // "slotPbBtnButtonClick"
QT_MOC_LITERAL(9, 105, 24), // "slotPbSliderValueChanged"
QT_MOC_LITERAL(10, 130, 7), // "currVal"
QT_MOC_LITERAL(11, 138, 17), // "slotHoverOnSlider"
QT_MOC_LITERAL(12, 156, 22), // "slotHoverInOutOnSlider"
QT_MOC_LITERAL(13, 179, 10), // "hoverState"
QT_MOC_LITERAL(14, 190, 25), // "slotUpadateCurrentElement"
QT_MOC_LITERAL(15, 216, 19) // "slotImageMouseHover"

    },
    "PbToolbar\0sigSliderValueChanged\0\0"
    "currTime\0windowId\0sigPbToolbarBtnClick\0"
    "index\0state\0slotPbBtnButtonClick\0"
    "slotPbSliderValueChanged\0currVal\0"
    "slotHoverOnSlider\0slotHoverInOutOnSlider\0"
    "hoverState\0slotUpadateCurrentElement\0"
    "slotImageMouseHover"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PbToolbar[] = {

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
       1,    2,   54,    2, 0x06 /* Public */,
       5,    3,   59,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    1,   66,    2, 0x0a /* Public */,
       9,    2,   69,    2, 0x0a /* Public */,
      11,    2,   74,    2, 0x0a /* Public */,
      12,    2,   79,    2, 0x0a /* Public */,
      14,    1,   84,    2, 0x0a /* Public */,
      15,    2,   87,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::ULongLong, QMetaType::UShort,    3,    4,
    QMetaType::Void, QMetaType::Int, QMetaType::UShort, QMetaType::Bool,    6,    4,    7,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   10,    2,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   10,    6,
    QMetaType::Void, QMetaType::Bool, QMetaType::Int,   13,    6,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::UChar, QMetaType::Bool,    2,    2,

       0        // eod
};

void PbToolbar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PbToolbar *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigSliderValueChanged((*reinterpret_cast< quint64(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        case 1: _t->sigPbToolbarBtnClick((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 2: _t->slotPbBtnButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotPbSliderValueChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->slotHoverOnSlider((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->slotHoverInOutOnSlider((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->slotUpadateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->slotImageMouseHover((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PbToolbar::*)(quint64 , quint16 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PbToolbar::sigSliderValueChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PbToolbar::*)(int , quint16 , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PbToolbar::sigPbToolbarBtnClick)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PbToolbar::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_PbToolbar.data,
    qt_meta_data_PbToolbar,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PbToolbar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PbToolbar::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PbToolbar.stringdata0))
        return static_cast<void*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int PbToolbar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void PbToolbar::sigSliderValueChanged(quint64 _t1, quint16 _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PbToolbar::sigPbToolbarBtnClick(int _t1, quint16 _t2, bool _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
