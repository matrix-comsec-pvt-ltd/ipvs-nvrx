/****************************************************************************
** Meta object code from reading C++ file 'MxTimelineSlider.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/MxTimelineSlider.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MxTimelineSlider.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MxTimelineSlider_t {
    QByteArrayData data[15];
    char stringdata0[210];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MxTimelineSlider_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MxTimelineSlider_t qt_meta_stringdata_MxTimelineSlider = {
    {
QT_MOC_LITERAL(0, 0, 16), // "MxTimelineSlider"
QT_MOC_LITERAL(1, 17, 20), // "sigTileToolTipUpdate"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 12), // "m_timeString"
QT_MOC_LITERAL(4, 52, 4), // "curX"
QT_MOC_LITERAL(5, 57, 4), // "curY"
QT_MOC_LITERAL(6, 62, 18), // "sigTileToolTipHide"
QT_MOC_LITERAL(7, 81, 7), // "toolTip"
QT_MOC_LITERAL(8, 89, 22), // "sigSliderToolTipUpdate"
QT_MOC_LITERAL(9, 112, 16), // "m_SliderDateTime"
QT_MOC_LITERAL(10, 129, 20), // "sigSliderToolTipHide"
QT_MOC_LITERAL(11, 150, 13), // "sliderToolTip"
QT_MOC_LITERAL(12, 164, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(13, 189, 5), // "index"
QT_MOC_LITERAL(14, 195, 14) // "slotChangePage"

    },
    "MxTimelineSlider\0sigTileToolTipUpdate\0"
    "\0m_timeString\0curX\0curY\0sigTileToolTipHide\0"
    "toolTip\0sigSliderToolTipUpdate\0"
    "m_SliderDateTime\0sigSliderToolTipHide\0"
    "sliderToolTip\0slotUpdateCurrentElement\0"
    "index\0slotChangePage"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MxTimelineSlider[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   49,    2, 0x06 /* Public */,
       6,    1,   56,    2, 0x06 /* Public */,
       8,    3,   59,    2, 0x06 /* Public */,
      10,    1,   66,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      12,    1,   69,    2, 0x0a /* Public */,
      14,    1,   72,    2, 0x0a /* Public */,
      14,    0,   75,    2, 0x2a /* Public | MethodCloned */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::Int, QMetaType::Int,    3,    4,    5,
    QMetaType::Void, QMetaType::Bool,    7,
    QMetaType::Void, QMetaType::QString, QMetaType::Int, QMetaType::Int,    9,    4,    5,
    QMetaType::Void, QMetaType::Bool,   11,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,   13,
    QMetaType::Void, QMetaType::Int,   13,
    QMetaType::Void,

       0        // eod
};

void MxTimelineSlider::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MxTimelineSlider *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigTileToolTipUpdate((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 1: _t->sigTileToolTipHide((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->sigSliderToolTipUpdate((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 3: _t->sigSliderToolTipHide((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->slotChangePage((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->slotChangePage(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MxTimelineSlider::*)(QString , int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MxTimelineSlider::sigTileToolTipUpdate)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MxTimelineSlider::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MxTimelineSlider::sigTileToolTipHide)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (MxTimelineSlider::*)(QString , int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MxTimelineSlider::sigSliderToolTipUpdate)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (MxTimelineSlider::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MxTimelineSlider::sigSliderToolTipHide)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MxTimelineSlider::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_MxTimelineSlider.data,
    qt_meta_data_MxTimelineSlider,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MxTimelineSlider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MxTimelineSlider::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MxTimelineSlider.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return QWidget::qt_metacast(_clname);
}

int MxTimelineSlider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void MxTimelineSlider::sigTileToolTipUpdate(QString _t1, int _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MxTimelineSlider::sigTileToolTipHide(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void MxTimelineSlider::sigSliderToolTipUpdate(QString _t1, int _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void MxTimelineSlider::sigSliderToolTipHide(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
