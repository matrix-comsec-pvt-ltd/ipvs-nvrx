/****************************************************************************
** Meta object code from reading C++ file 'Image.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/Image.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Image.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Image_t {
    QByteArrayData data[14];
    char stringdata0[223];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Image_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Image_t qt_meta_stringdata_Image = {
    {
QT_MOC_LITERAL(0, 0, 5), // "Image"
QT_MOC_LITERAL(1, 6, 15), // "sigImagePressed"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 11), // "indexInPage"
QT_MOC_LITERAL(4, 35, 15), // "sigImageClicked"
QT_MOC_LITERAL(5, 51, 21), // "sigImageDoubleClicked"
QT_MOC_LITERAL(6, 73, 23), // "sigUpdateCurrentElement"
QT_MOC_LITERAL(7, 97, 5), // "index"
QT_MOC_LITERAL(8, 103, 18), // "sigImageMouseHover"
QT_MOC_LITERAL(9, 122, 14), // "isImageCantain"
QT_MOC_LITERAL(10, 137, 12), // "sigMouseMove"
QT_MOC_LITERAL(11, 150, 12), // "currentValue"
QT_MOC_LITERAL(12, 163, 32), // "slotmouseButtonPressTimerTimeout"
QT_MOC_LITERAL(13, 196, 26) // "slotclickeffctTimerTimeout"

    },
    "Image\0sigImagePressed\0\0indexInPage\0"
    "sigImageClicked\0sigImageDoubleClicked\0"
    "sigUpdateCurrentElement\0index\0"
    "sigImageMouseHover\0isImageCantain\0"
    "sigMouseMove\0currentValue\0"
    "slotmouseButtonPressTimerTimeout\0"
    "slotclickeffctTimerTimeout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Image[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,
       4,    1,   57,    2, 0x06 /* Public */,
       5,    1,   60,    2, 0x06 /* Public */,
       6,    1,   63,    2, 0x06 /* Public */,
       8,    2,   66,    2, 0x06 /* Public */,
      10,    2,   71,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      12,    0,   76,    2, 0x0a /* Public */,
      13,    0,   77,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    3,    9,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   11,    7,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Image::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Image *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigImagePressed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->sigImageClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->sigImageDoubleClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->sigUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->sigImageMouseHover((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 5: _t->sigMouseMove((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->slotmouseButtonPressTimerTimeout(); break;
        case 7: _t->slotclickeffctTimerTimeout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Image::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Image::sigImagePressed)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Image::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Image::sigImageClicked)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Image::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Image::sigImageDoubleClicked)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (Image::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Image::sigUpdateCurrentElement)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (Image::*)(int , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Image::sigImageMouseHover)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (Image::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Image::sigMouseMove)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Image::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Image.data,
    qt_meta_data_Image,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Image::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Image::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Image.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return QWidget::qt_metacast(_clname);
}

int Image::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void Image::sigImagePressed(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Image::sigImageClicked(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Image::sigImageDoubleClicked(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Image::sigUpdateCurrentElement(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Image::sigImageMouseHover(int _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void Image::sigMouseMove(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
