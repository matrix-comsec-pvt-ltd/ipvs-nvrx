/****************************************************************************
** Meta object code from reading C++ file 'HealthStatus.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../HealthStatus.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'HealthStatus.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_HealthStatus_t {
    QByteArrayData data[21];
    char stringdata0[336];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_HealthStatus_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_HealthStatus_t qt_meta_stringdata_HealthStatus = {
    {
QT_MOC_LITERAL(0, 0, 12), // "HealthStatus"
QT_MOC_LITERAL(1, 13, 11), // "sigNextPage"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 7), // "devName"
QT_MOC_LITERAL(4, 34, 10), // "MENU_TAB_e"
QT_MOC_LITERAL(5, 45, 9), // "tabToShow"
QT_MOC_LITERAL(6, 55, 24), // "slotStatusTileMouseHover"
QT_MOC_LITERAL(7, 80, 9), // "tileIndex"
QT_MOC_LITERAL(8, 90, 10), // "hoverState"
QT_MOC_LITERAL(9, 101, 22), // "slotSpinBoxValueChange"
QT_MOC_LITERAL(10, 124, 7), // "newName"
QT_MOC_LITERAL(11, 132, 11), // "indexInPage"
QT_MOC_LITERAL(12, 144, 22), // "slotRefreshButtonClick"
QT_MOC_LITERAL(13, 167, 5), // "index"
QT_MOC_LITERAL(14, 173, 17), // "slotNextPageClick"
QT_MOC_LITERAL(15, 191, 24), // "slotInfoPageCnfgBtnClick"
QT_MOC_LITERAL(16, 216, 25), // "slotUpadateCurrentElement"
QT_MOC_LITERAL(17, 242, 25), // "slotPrevNextCameraClicked"
QT_MOC_LITERAL(18, 268, 15), // "slotTabSelected"
QT_MOC_LITERAL(19, 284, 25), // "slotPageNumberButtonClick"
QT_MOC_LITERAL(20, 310, 25) // "slotDropDownListDestroyed"

    },
    "HealthStatus\0sigNextPage\0\0devName\0"
    "MENU_TAB_e\0tabToShow\0slotStatusTileMouseHover\0"
    "tileIndex\0hoverState\0slotSpinBoxValueChange\0"
    "newName\0indexInPage\0slotRefreshButtonClick\0"
    "index\0slotNextPageClick\0"
    "slotInfoPageCnfgBtnClick\0"
    "slotUpadateCurrentElement\0"
    "slotPrevNextCameraClicked\0slotTabSelected\0"
    "slotPageNumberButtonClick\0"
    "slotDropDownListDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_HealthStatus[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   69,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    2,   74,    2, 0x0a /* Public */,
       9,    2,   79,    2, 0x0a /* Public */,
      12,    1,   84,    2, 0x0a /* Public */,
      14,    1,   87,    2, 0x0a /* Public */,
      15,    1,   90,    2, 0x0a /* Public */,
      16,    1,   93,    2, 0x0a /* Public */,
      17,    1,   96,    2, 0x0a /* Public */,
      18,    1,   99,    2, 0x0a /* Public */,
      19,    1,  102,    2, 0x0a /* Public */,
      20,    0,  105,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, 0x80000000 | 4,    3,    5,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    7,    8,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,   10,   11,
    QMetaType::Void, QMetaType::Int,   13,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,   13,
    QMetaType::Void, QMetaType::Int,   13,
    QMetaType::Void, QMetaType::Int,   13,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,

       0        // eod
};

void HealthStatus::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<HealthStatus *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigNextPage((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< MENU_TAB_e(*)>(_a[2]))); break;
        case 1: _t->slotStatusTileMouseHover((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 2: _t->slotSpinBoxValueChange((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 3: _t->slotRefreshButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotNextPageClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->slotInfoPageCnfgBtnClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->slotUpadateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->slotPrevNextCameraClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->slotTabSelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->slotPageNumberButtonClick((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 10: _t->slotDropDownListDestroyed(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (HealthStatus::*)(QString , MENU_TAB_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&HealthStatus::sigNextPage)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject HealthStatus::staticMetaObject = { {
    QMetaObject::SuperData::link<BackGround::staticMetaObject>(),
    qt_meta_stringdata_HealthStatus.data,
    qt_meta_data_HealthStatus,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *HealthStatus::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HealthStatus::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_HealthStatus.stringdata0))
        return static_cast<void*>(this);
    return BackGround::qt_metacast(_clname);
}

int HealthStatus::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BackGround::qt_metacall(_c, _id, _a);
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
void HealthStatus::sigNextPage(QString _t1, MENU_TAB_e _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
