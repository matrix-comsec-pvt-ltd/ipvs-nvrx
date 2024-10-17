/****************************************************************************
** Meta object code from reading C++ file 'LayoutWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/LayoutWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LayoutWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_LayoutWindow_t {
    QByteArrayData data[20];
    char stringdata0[305];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_LayoutWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_LayoutWindow_t qt_meta_stringdata_LayoutWindow = {
    {
QT_MOC_LITERAL(0, 0, 12), // "LayoutWindow"
QT_MOC_LITERAL(1, 13, 17), // "sigWindowSelected"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 11), // "windowIndex"
QT_MOC_LITERAL(4, 44, 22), // "sigLoadMenuListOptions"
QT_MOC_LITERAL(5, 67, 22), // "sigWindowDoubleClicked"
QT_MOC_LITERAL(6, 90, 18), // "sigEnterKeyPressed"
QT_MOC_LITERAL(7, 109, 21), // "sigWindowImageClicked"
QT_MOC_LITERAL(8, 131, 19), // "WINDOW_IMAGE_TYPE_e"
QT_MOC_LITERAL(9, 151, 9), // "imageType"
QT_MOC_LITERAL(10, 161, 19), // "sigWindowImageHover"
QT_MOC_LITERAL(11, 181, 7), // "isHover"
QT_MOC_LITERAL(12, 189, 13), // "sigSwapWindow"
QT_MOC_LITERAL(13, 203, 21), // "sigAPCenterBtnClicked"
QT_MOC_LITERAL(14, 225, 5), // "index"
QT_MOC_LITERAL(15, 231, 16), // "slotImageClicked"
QT_MOC_LITERAL(16, 248, 11), // "indexInPage"
QT_MOC_LITERAL(17, 260, 14), // "slotMouseHover"
QT_MOC_LITERAL(18, 275, 5), // "state"
QT_MOC_LITERAL(19, 281, 23) // "slotAPToolbarBtnClicked"

    },
    "LayoutWindow\0sigWindowSelected\0\0"
    "windowIndex\0sigLoadMenuListOptions\0"
    "sigWindowDoubleClicked\0sigEnterKeyPressed\0"
    "sigWindowImageClicked\0WINDOW_IMAGE_TYPE_e\0"
    "imageType\0sigWindowImageHover\0isHover\0"
    "sigSwapWindow\0sigAPCenterBtnClicked\0"
    "index\0slotImageClicked\0indexInPage\0"
    "slotMouseHover\0state\0slotAPToolbarBtnClicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_LayoutWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   69,    2, 0x06 /* Public */,
       4,    1,   72,    2, 0x06 /* Public */,
       5,    1,   75,    2, 0x06 /* Public */,
       6,    1,   78,    2, 0x06 /* Public */,
       7,    2,   81,    2, 0x06 /* Public */,
      10,    3,   86,    2, 0x06 /* Public */,
      12,    2,   93,    2, 0x06 /* Public */,
      13,    2,   98,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      15,    1,  103,    2, 0x0a /* Public */,
      17,    2,  106,    2, 0x0a /* Public */,
      19,    1,  111,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::UChar,    3,
    QMetaType::Void, QMetaType::UChar,    3,
    QMetaType::Void, QMetaType::UChar,    3,
    QMetaType::Void, QMetaType::UChar,    3,
    QMetaType::Void, 0x80000000 | 8, QMetaType::UChar,    9,    3,
    QMetaType::Void, 0x80000000 | 8, QMetaType::UChar, QMetaType::Bool,    9,    3,   11,
    QMetaType::Void, QMetaType::UChar, QMetaType::UChar,    2,    2,
    QMetaType::Void, QMetaType::UChar, QMetaType::UShort,   14,    3,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,   16,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,   14,   18,
    QMetaType::Void, QMetaType::Int,   14,

       0        // eod
};

void LayoutWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LayoutWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigWindowSelected((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 1: _t->sigLoadMenuListOptions((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 2: _t->sigWindowDoubleClicked((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 3: _t->sigEnterKeyPressed((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 4: _t->sigWindowImageClicked((*reinterpret_cast< WINDOW_IMAGE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2]))); break;
        case 5: _t->sigWindowImageHover((*reinterpret_cast< WINDOW_IMAGE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 6: _t->sigSwapWindow((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2]))); break;
        case 7: _t->sigAPCenterBtnClicked((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        case 8: _t->slotImageClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->slotMouseHover((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 10: _t->slotAPToolbarBtnClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LayoutWindow::*)(quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LayoutWindow::sigWindowSelected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LayoutWindow::*)(quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LayoutWindow::sigLoadMenuListOptions)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LayoutWindow::*)(quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LayoutWindow::sigWindowDoubleClicked)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (LayoutWindow::*)(quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LayoutWindow::sigEnterKeyPressed)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (LayoutWindow::*)(WINDOW_IMAGE_TYPE_e , quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LayoutWindow::sigWindowImageClicked)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (LayoutWindow::*)(WINDOW_IMAGE_TYPE_e , quint8 , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LayoutWindow::sigWindowImageHover)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (LayoutWindow::*)(quint8 , quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LayoutWindow::sigSwapWindow)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (LayoutWindow::*)(quint8 , quint16 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LayoutWindow::sigAPCenterBtnClicked)) {
                *result = 7;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject LayoutWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_LayoutWindow.data,
    qt_meta_data_LayoutWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *LayoutWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LayoutWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LayoutWindow.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    if (!strcmp(_clname, "QOpenGLFunctions"))
        return static_cast< QOpenGLFunctions*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int LayoutWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = KeyBoard::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void LayoutWindow::sigWindowSelected(quint8 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void LayoutWindow::sigLoadMenuListOptions(quint8 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void LayoutWindow::sigWindowDoubleClicked(quint8 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void LayoutWindow::sigEnterKeyPressed(quint8 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void LayoutWindow::sigWindowImageClicked(WINDOW_IMAGE_TYPE_e _t1, quint8 _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void LayoutWindow::sigWindowImageHover(WINDOW_IMAGE_TYPE_e _t1, quint8 _t2, bool _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void LayoutWindow::sigSwapWindow(quint8 _t1, quint8 _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void LayoutWindow::sigAPCenterBtnClicked(quint8 _t1, quint16 _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
