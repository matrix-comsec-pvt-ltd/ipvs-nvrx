/****************************************************************************
** Meta object code from reading C++ file 'WizardChangePass.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Wizard/WizardChangePass.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'WizardChangePass.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_WizardChangePass_t {
    QByteArrayData data[11];
    char stringdata0[171];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_WizardChangePass_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_WizardChangePass_t qt_meta_stringdata_WizardChangePass = {
    {
QT_MOC_LITERAL(0, 0, 16), // "WizardChangePass"
QT_MOC_LITERAL(1, 17, 11), // "sigExitPage"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 21), // "TOOLBAR_BUTTON_TYPE_e"
QT_MOC_LITERAL(4, 52, 25), // "slotStatusRepTimerTimeout"
QT_MOC_LITERAL(5, 78, 23), // "slotTextBoxLoadInfopage"
QT_MOC_LITERAL(6, 102, 5), // "index"
QT_MOC_LITERAL(7, 108, 15), // "INFO_MSG_TYPE_e"
QT_MOC_LITERAL(8, 124, 7), // "msgType"
QT_MOC_LITERAL(9, 132, 17), // "slotOkButtonClick"
QT_MOC_LITERAL(10, 150, 20) // "slotcloseButtonClick"

    },
    "WizardChangePass\0sigExitPage\0\0"
    "TOOLBAR_BUTTON_TYPE_e\0slotStatusRepTimerTimeout\0"
    "slotTextBoxLoadInfopage\0index\0"
    "INFO_MSG_TYPE_e\0msgType\0slotOkButtonClick\0"
    "slotcloseButtonClick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_WizardChangePass[] = {

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
       1,    1,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   42,    2, 0x0a /* Public */,
       5,    2,   43,    2, 0x0a /* Public */,
       9,    1,   48,    2, 0x0a /* Public */,
      10,    1,   51,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 7,    6,    8,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,

       0        // eod
};

void WizardChangePass::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<WizardChangePass *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigExitPage((*reinterpret_cast< TOOLBAR_BUTTON_TYPE_e(*)>(_a[1]))); break;
        case 1: _t->slotStatusRepTimerTimeout(); break;
        case 2: _t->slotTextBoxLoadInfopage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< INFO_MSG_TYPE_e(*)>(_a[2]))); break;
        case 3: _t->slotOkButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotcloseButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (WizardChangePass::*)(TOOLBAR_BUTTON_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WizardChangePass::sigExitPage)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject WizardChangePass::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_WizardChangePass.data,
    qt_meta_data_WizardChangePass,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *WizardChangePass::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WizardChangePass::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_WizardChangePass.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int WizardChangePass::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void WizardChangePass::sigExitPage(TOOLBAR_BUTTON_TYPE_e _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
