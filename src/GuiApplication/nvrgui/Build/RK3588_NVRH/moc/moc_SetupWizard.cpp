/****************************************************************************
** Meta object code from reading C++ file 'SetupWizard.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Wizard/SetupWizard.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SetupWizard.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SetupWizard_t {
    QByteArrayData data[12];
    char stringdata0[155];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SetupWizard_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SetupWizard_t qt_meta_stringdata_SetupWizard = {
    {
QT_MOC_LITERAL(0, 0, 11), // "SetupWizard"
QT_MOC_LITERAL(1, 12, 15), // "sigQuitSetupWiz"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 22), // "sigLanguageCfgModified"
QT_MOC_LITERAL(4, 52, 3), // "str"
QT_MOC_LITERAL(5, 56, 21), // "slotButtonOrLinkClick"
QT_MOC_LITERAL(6, 78, 18), // "slotTextLableHover"
QT_MOC_LITERAL(7, 97, 5), // "index"
QT_MOC_LITERAL(8, 103, 9), // "isHoverIn"
QT_MOC_LITERAL(9, 113, 15), // "slotTextClicked"
QT_MOC_LITERAL(10, 129, 4), // "pgId"
QT_MOC_LITERAL(11, 134, 20) // "slotInfoPageBtnclick"

    },
    "SetupWizard\0sigQuitSetupWiz\0\0"
    "sigLanguageCfgModified\0str\0"
    "slotButtonOrLinkClick\0slotTextLableHover\0"
    "index\0isHoverIn\0slotTextClicked\0pgId\0"
    "slotInfoPageBtnclick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SetupWizard[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x06 /* Public */,
       3,    1,   45,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   48,    2, 0x0a /* Public */,
       6,    2,   51,    2, 0x0a /* Public */,
       9,    1,   56,    2, 0x0a /* Public */,
      11,    1,   59,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    7,    8,
    QMetaType::Void, QMetaType::Int,   10,
    QMetaType::Void, QMetaType::Int,    7,

       0        // eod
};

void SetupWizard::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SetupWizard *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigQuitSetupWiz(); break;
        case 1: _t->sigLanguageCfgModified((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->slotButtonOrLinkClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotTextLableHover((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: _t->slotTextClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SetupWizard::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SetupWizard::sigQuitSetupWiz)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SetupWizard::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SetupWizard::sigLanguageCfgModified)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SetupWizard::staticMetaObject = { {
    QMetaObject::SuperData::link<BackGround::staticMetaObject>(),
    qt_meta_stringdata_SetupWizard.data,
    qt_meta_data_SetupWizard,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SetupWizard::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SetupWizard::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SetupWizard.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return BackGround::qt_metacast(_clname);
}

int SetupWizard::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BackGround::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void SetupWizard::sigQuitSetupWiz()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SetupWizard::sigLanguageCfgModified(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
