/****************************************************************************
** Meta object code from reading C++ file 'AdvancedCurveStyler.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../AdvancedCurveStyler.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AdvancedCurveStyler.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AdvancedCurveStyler_t {
    QByteArrayData data[21];
    char stringdata0[283];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AdvancedCurveStyler_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AdvancedCurveStyler_t qt_meta_stringdata_AdvancedCurveStyler = {
    {
QT_MOC_LITERAL(0, 0, 19), // "AdvancedCurveStyler"
QT_MOC_LITERAL(1, 20, 12), // "styleChanged"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 5), // "index"
QT_MOC_LITERAL(4, 40, 13), // "stylesUpdated"
QT_MOC_LITERAL(5, 54, 16), // "colorModeChanged"
QT_MOC_LITERAL(6, 71, 9), // "ColorMode"
QT_MOC_LITERAL(7, 81, 4), // "mode"
QT_MOC_LITERAL(8, 86, 17), // "applyRandomColors"
QT_MOC_LITERAL(9, 104, 21), // "applySequentialColors"
QT_MOC_LITERAL(10, 126, 22), // "applyCategoricalColors"
QT_MOC_LITERAL(11, 149, 22), // "togglePointsVisibility"
QT_MOC_LITERAL(12, 172, 4), // "show"
QT_MOC_LITERAL(13, 177, 16), // "setAllLineWidths"
QT_MOC_LITERAL(14, 194, 5), // "width"
QT_MOC_LITERAL(15, 200, 16), // "setAllPointSizes"
QT_MOC_LITERAL(16, 217, 4), // "size"
QT_MOC_LITERAL(17, 222, 12), // "RandomColors"
QT_MOC_LITERAL(18, 235, 16), // "SequentialColors"
QT_MOC_LITERAL(19, 252, 17), // "CategoricalColors"
QT_MOC_LITERAL(20, 270, 12) // "CustomColors"

    },
    "AdvancedCurveStyler\0styleChanged\0\0"
    "index\0stylesUpdated\0colorModeChanged\0"
    "ColorMode\0mode\0applyRandomColors\0"
    "applySequentialColors\0applyCategoricalColors\0"
    "togglePointsVisibility\0show\0"
    "setAllLineWidths\0width\0setAllPointSizes\0"
    "size\0RandomColors\0SequentialColors\0"
    "CategoricalColors\0CustomColors"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AdvancedCurveStyler[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       1,   78, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x06 /* Public */,
       4,    0,   62,    2, 0x06 /* Public */,
       5,    1,   63,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    0,   66,    2, 0x0a /* Public */,
       9,    0,   67,    2, 0x0a /* Public */,
      10,    0,   68,    2, 0x0a /* Public */,
      11,    1,   69,    2, 0x0a /* Public */,
      13,    1,   72,    2, 0x0a /* Public */,
      15,    1,   75,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6,    7,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   12,
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void, QMetaType::Int,   16,

 // enums: name, alias, flags, count, data
       6,    6, 0x0,    4,   83,

 // enum data: key, value
      17, uint(AdvancedCurveStyler::RandomColors),
      18, uint(AdvancedCurveStyler::SequentialColors),
      19, uint(AdvancedCurveStyler::CategoricalColors),
      20, uint(AdvancedCurveStyler::CustomColors),

       0        // eod
};

void AdvancedCurveStyler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AdvancedCurveStyler *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->styleChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->stylesUpdated(); break;
        case 2: _t->colorModeChanged((*reinterpret_cast< ColorMode(*)>(_a[1]))); break;
        case 3: _t->applyRandomColors(); break;
        case 4: _t->applySequentialColors(); break;
        case 5: _t->applyCategoricalColors(); break;
        case 6: _t->togglePointsVisibility((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->setAllLineWidths((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->setAllPointSizes((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AdvancedCurveStyler::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AdvancedCurveStyler::styleChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (AdvancedCurveStyler::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AdvancedCurveStyler::stylesUpdated)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (AdvancedCurveStyler::*)(ColorMode );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AdvancedCurveStyler::colorModeChanged)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject AdvancedCurveStyler::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_AdvancedCurveStyler.data,
    qt_meta_data_AdvancedCurveStyler,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *AdvancedCurveStyler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AdvancedCurveStyler::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AdvancedCurveStyler.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AdvancedCurveStyler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void AdvancedCurveStyler::styleChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void AdvancedCurveStyler::stylesUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void AdvancedCurveStyler::colorModeChanged(ColorMode _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
