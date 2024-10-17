/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../MainWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[21];
    char stringdata0[285];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 17), // "slotTestCompelete"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 15), // "HW_TEST_BLOCK_e"
QT_MOC_LITERAL(4, 46, 9), // "testBlock"
QT_MOC_LITERAL(5, 56, 15), // "slotButtonClick"
QT_MOC_LITERAL(6, 72, 5), // "index"
QT_MOC_LITERAL(7, 78, 10), // "slotUsbBkp"
QT_MOC_LITERAL(8, 89, 10), // "slotFtpBkp"
QT_MOC_LITERAL(9, 100, 14), // "slotHideReport"
QT_MOC_LITERAL(10, 115, 11), // "slotTimeOut"
QT_MOC_LITERAL(11, 127, 19), // "slotPopUpAlertClose"
QT_MOC_LITERAL(12, 147, 12), // "isCloseAlert"
QT_MOC_LITERAL(13, 160, 16), // "slotLoadNextPage"
QT_MOC_LITERAL(14, 177, 17), // "slotManualTimeout"
QT_MOC_LITERAL(15, 195, 18), // "slotCtrlBtnClicked"
QT_MOC_LITERAL(16, 214, 11), // "hwTestBlock"
QT_MOC_LITERAL(17, 226, 7), // "hwIndex"
QT_MOC_LITERAL(18, 234, 12), // "ctrlBtnIndex"
QT_MOC_LITERAL(19, 247, 19), // "slotShutDownClicked"
QT_MOC_LITERAL(20, 267, 17) // "slotUpdateSysTime"

    },
    "MainWindow\0slotTestCompelete\0\0"
    "HW_TEST_BLOCK_e\0testBlock\0slotButtonClick\0"
    "index\0slotUsbBkp\0slotFtpBkp\0slotHideReport\0"
    "slotTimeOut\0slotPopUpAlertClose\0"
    "isCloseAlert\0slotLoadNextPage\0"
    "slotManualTimeout\0slotCtrlBtnClicked\0"
    "hwTestBlock\0hwIndex\0ctrlBtnIndex\0"
    "slotShutDownClicked\0slotUpdateSysTime"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   74,    2, 0x0a /* Public */,
       5,    1,   77,    2, 0x0a /* Public */,
       7,    1,   80,    2, 0x0a /* Public */,
       8,    1,   83,    2, 0x0a /* Public */,
       9,    0,   86,    2, 0x0a /* Public */,
      10,    0,   87,    2, 0x0a /* Public */,
      11,    2,   88,    2, 0x0a /* Public */,
      13,    0,   93,    2, 0x0a /* Public */,
      14,    0,   94,    2, 0x0a /* Public */,
      15,    3,   95,    2, 0x0a /* Public */,
      19,    1,  102,    2, 0x0a /* Public */,
      20,    0,  105,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    6,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int, QMetaType::Int,   16,   17,   18,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotTestCompelete((*reinterpret_cast< HW_TEST_BLOCK_e(*)>(_a[1]))); break;
        case 1: _t->slotButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slotUsbBkp((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->slotFtpBkp((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->slotHideReport(); break;
        case 5: _t->slotTimeOut(); break;
        case 6: _t->slotPopUpAlertClose((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 7: _t->slotLoadNextPage(); break;
        case 8: _t->slotManualTimeout(); break;
        case 9: _t->slotCtrlBtnClicked((*reinterpret_cast< HW_TEST_BLOCK_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 10: _t->slotShutDownClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->slotUpdateSysTime(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
