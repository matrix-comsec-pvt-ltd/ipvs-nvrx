/****************************************************************************
** Meta object code from reading C++ file 'PrivacyMaskSettings.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/CameraSettings/PrivacyMaskSettings.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PrivacyMaskSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PrivacyMaskSettings_t {
    QByteArrayData data[9];
    char stringdata0[136];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PrivacyMaskSettings_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PrivacyMaskSettings_t qt_meta_stringdata_PrivacyMaskSettings = {
    {
QT_MOC_LITERAL(0, 0, 19), // "PrivacyMaskSettings"
QT_MOC_LITERAL(1, 20, 17), // "sigLoadProcessBar"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 22), // "slotMenuButtonSelected"
QT_MOC_LITERAL(4, 62, 9), // "menuLabel"
QT_MOC_LITERAL(5, 72, 9), // "menuIndex"
QT_MOC_LITERAL(6, 82, 21), // "slotMenuListDestroyed"
QT_MOC_LITERAL(7, 104, 25), // "slotInfoPageButtonClicked"
QT_MOC_LITERAL(8, 130, 5) // "index"

    },
    "PrivacyMaskSettings\0sigLoadProcessBar\0"
    "\0slotMenuButtonSelected\0menuLabel\0"
    "menuIndex\0slotMenuListDestroyed\0"
    "slotInfoPageButtonClicked\0index"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PrivacyMaskSettings[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    2,   35,    2, 0x0a /* Public */,
       6,    0,   40,    2, 0x0a /* Public */,
       7,    1,   41,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::UChar,    4,    5,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    8,

       0        // eod
};

void PrivacyMaskSettings::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PrivacyMaskSettings *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigLoadProcessBar(); break;
        case 1: _t->slotMenuButtonSelected((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2]))); break;
        case 2: _t->slotMenuListDestroyed(); break;
        case 3: _t->slotInfoPageButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PrivacyMaskSettings::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PrivacyMaskSettings::sigLoadProcessBar)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PrivacyMaskSettings::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_PrivacyMaskSettings.data,
    qt_meta_data_PrivacyMaskSettings,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PrivacyMaskSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PrivacyMaskSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PrivacyMaskSettings.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int PrivacyMaskSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void PrivacyMaskSettings::sigLoadProcessBar()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
