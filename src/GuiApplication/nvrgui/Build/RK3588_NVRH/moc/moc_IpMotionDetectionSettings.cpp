/****************************************************************************
** Meta object code from reading C++ file 'IpMotionDetectionSettings.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/CameraSettings/IpMotionDetectionSettings.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'IpMotionDetectionSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_IpMotionDetectionSettings_t {
    QByteArrayData data[14];
    char stringdata0[214];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_IpMotionDetectionSettings_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_IpMotionDetectionSettings_t qt_meta_stringdata_IpMotionDetectionSettings = {
    {
QT_MOC_LITERAL(0, 0, 25), // "IpMotionDetectionSettings"
QT_MOC_LITERAL(1, 26, 17), // "sigLoadProcessBar"
QT_MOC_LITERAL(2, 44, 0), // ""
QT_MOC_LITERAL(3, 45, 22), // "slotMenuButtonSelected"
QT_MOC_LITERAL(4, 68, 9), // "menuLabel"
QT_MOC_LITERAL(5, 78, 9), // "menuIndex"
QT_MOC_LITERAL(6, 88, 21), // "slotMenuListDestroyed"
QT_MOC_LITERAL(7, 110, 25), // "slotInfoPageButtonClicked"
QT_MOC_LITERAL(8, 136, 5), // "index"
QT_MOC_LITERAL(9, 142, 16), // "slotValueChanged"
QT_MOC_LITERAL(10, 159, 3), // "key"
QT_MOC_LITERAL(11, 163, 5), // "value"
QT_MOC_LITERAL(12, 169, 21), // "slotPickListDestroyed"
QT_MOC_LITERAL(13, 191, 22) // "slotCloseButtonClicked"

    },
    "IpMotionDetectionSettings\0sigLoadProcessBar\0"
    "\0slotMenuButtonSelected\0menuLabel\0"
    "menuIndex\0slotMenuListDestroyed\0"
    "slotInfoPageButtonClicked\0index\0"
    "slotValueChanged\0key\0value\0"
    "slotPickListDestroyed\0slotCloseButtonClicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_IpMotionDetectionSettings[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    2,   50,    2, 0x0a /* Public */,
       6,    0,   55,    2, 0x0a /* Public */,
       7,    1,   56,    2, 0x0a /* Public */,
       9,    3,   59,    2, 0x0a /* Public */,
      12,    0,   66,    2, 0x0a /* Public */,
      13,    1,   67,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::UChar,    4,    5,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void, QMetaType::UChar, QMetaType::QString, QMetaType::Bool,   10,   11,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    8,

       0        // eod
};

void IpMotionDetectionSettings::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<IpMotionDetectionSettings *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigLoadProcessBar(); break;
        case 1: _t->slotMenuButtonSelected((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2]))); break;
        case 2: _t->slotMenuListDestroyed(); break;
        case 3: _t->slotInfoPageButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotValueChanged((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 5: _t->slotPickListDestroyed(); break;
        case 6: _t->slotCloseButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (IpMotionDetectionSettings::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IpMotionDetectionSettings::sigLoadProcessBar)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject IpMotionDetectionSettings::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_IpMotionDetectionSettings.data,
    qt_meta_data_IpMotionDetectionSettings,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *IpMotionDetectionSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *IpMotionDetectionSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_IpMotionDetectionSettings.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int IpMotionDetectionSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void IpMotionDetectionSettings::sigLoadProcessBar()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
