/****************************************************************************
** Meta object code from reading C++ file 'MxAutoAddCam.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../MxAutoAddCam.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MxAutoAddCam.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MxAutoAddCam_t {
    QByteArrayData data[18];
    char stringdata0[339];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MxAutoAddCam_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MxAutoAddCam_t qt_meta_stringdata_MxAutoAddCam = {
    {
QT_MOC_LITERAL(0, 0, 12), // "MxAutoAddCam"
QT_MOC_LITERAL(1, 13, 13), // "sigCloseAlert"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 24), // "sigAutoAddCamPopUpAction"
QT_MOC_LITERAL(4, 53, 31), // "MX_AUTO_ADD_CAM_SEARCH_ACTION_e"
QT_MOC_LITERAL(5, 85, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(6, 110, 5), // "index"
QT_MOC_LITERAL(7, 116, 18), // "slotCnfgBtnClicked"
QT_MOC_LITERAL(8, 135, 22), // "slotCloseButtonClicked"
QT_MOC_LITERAL(9, 158, 25), // "slotSearchDurationTimeOut"
QT_MOC_LITERAL(10, 184, 18), // "slotAqrListTimeOut"
QT_MOC_LITERAL(11, 203, 15), // "slotTextClicked"
QT_MOC_LITERAL(12, 219, 18), // "slotTextLableHover"
QT_MOC_LITERAL(13, 238, 16), // "slotImageClicked"
QT_MOC_LITERAL(14, 255, 21), // "slotTileWithTextClick"
QT_MOC_LITERAL(15, 277, 15), // "slotButtonClick"
QT_MOC_LITERAL(16, 293, 20), // "slotInfoPageBtnclick"
QT_MOC_LITERAL(17, 314, 24) // "loadAutoAddCamAfterLogin"

    },
    "MxAutoAddCam\0sigCloseAlert\0\0"
    "sigAutoAddCamPopUpAction\0"
    "MX_AUTO_ADD_CAM_SEARCH_ACTION_e\0"
    "slotUpdateCurrentElement\0index\0"
    "slotCnfgBtnClicked\0slotCloseButtonClicked\0"
    "slotSearchDurationTimeOut\0slotAqrListTimeOut\0"
    "slotTextClicked\0slotTextLableHover\0"
    "slotImageClicked\0slotTileWithTextClick\0"
    "slotButtonClick\0slotInfoPageBtnclick\0"
    "loadAutoAddCamAfterLogin"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MxAutoAddCam[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   84,    2, 0x06 /* Public */,
       3,    1,   85,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   88,    2, 0x0a /* Public */,
       7,    1,   91,    2, 0x0a /* Public */,
       8,    1,   94,    2, 0x0a /* Public */,
       9,    0,   97,    2, 0x0a /* Public */,
      10,    0,   98,    2, 0x0a /* Public */,
      11,    1,   99,    2, 0x0a /* Public */,
      12,    2,  102,    2, 0x0a /* Public */,
      13,    1,  107,    2, 0x0a /* Public */,
      14,    1,  110,    2, 0x0a /* Public */,
      15,    1,  113,    2, 0x0a /* Public */,
      16,    1,  116,    2, 0x0a /* Public */,
      17,    0,  119,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    2,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,

       0        // eod
};

void MxAutoAddCam::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MxAutoAddCam *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigCloseAlert(); break;
        case 1: _t->sigAutoAddCamPopUpAction((*reinterpret_cast< MX_AUTO_ADD_CAM_SEARCH_ACTION_e(*)>(_a[1]))); break;
        case 2: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotCnfgBtnClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotCloseButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->slotSearchDurationTimeOut(); break;
        case 6: _t->slotAqrListTimeOut(); break;
        case 7: _t->slotTextClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->slotTextLableHover((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 9: _t->slotImageClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->slotTileWithTextClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->slotButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->loadAutoAddCamAfterLogin(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MxAutoAddCam::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MxAutoAddCam::sigCloseAlert)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MxAutoAddCam::*)(MX_AUTO_ADD_CAM_SEARCH_ACTION_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MxAutoAddCam::sigAutoAddCamPopUpAction)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MxAutoAddCam::staticMetaObject = { {
    QMetaObject::SuperData::link<BackGround::staticMetaObject>(),
    qt_meta_stringdata_MxAutoAddCam.data,
    qt_meta_data_MxAutoAddCam,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MxAutoAddCam::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MxAutoAddCam::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MxAutoAddCam.stringdata0))
        return static_cast<void*>(this);
    return BackGround::qt_metacast(_clname);
}

int MxAutoAddCam::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BackGround::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void MxAutoAddCam::sigCloseAlert()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void MxAutoAddCam::sigAutoAddCamPopUpAction(MX_AUTO_ADD_CAM_SEARCH_ACTION_e _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
