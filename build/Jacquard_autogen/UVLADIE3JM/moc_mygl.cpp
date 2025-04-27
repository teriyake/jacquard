/****************************************************************************
** Meta object code from reading C++ file 'mygl.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/mygl.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mygl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN4MyGLE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN4MyGLE = QtMocHelpers::stringData(
    "MyGL",
    "glInitialized",
    "",
    "slot_setRed",
    "slot_setGreen",
    "slot_setBlue",
    "slot_setMetallic",
    "slot_setRoughness",
    "slot_setAO",
    "slot_setDisplacement",
    "slot_loadEnvMap",
    "slot_updateLoomState",
    "warp_ends",
    "shafts",
    "treadles",
    "weft_picks",
    "QList<int>",
    "threading",
    "tieup",
    "treadling",
    "glm::vec2",
    "patternScale",
    "warpMaterial",
    "glm::vec3",
    "warpColor",
    "weftMaterial",
    "weftColor",
    "tick"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN4MyGLE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   80,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       3,    1,   81,    2, 0x0a,    2 /* Public */,
       4,    1,   84,    2, 0x0a,    4 /* Public */,
       5,    1,   87,    2, 0x0a,    6 /* Public */,
       6,    1,   90,    2, 0x0a,    8 /* Public */,
       7,    1,   93,    2, 0x0a,   10 /* Public */,
       8,    1,   96,    2, 0x0a,   12 /* Public */,
       9,    1,   99,    2, 0x0a,   14 /* Public */,
      10,    0,  102,    2, 0x0a,   16 /* Public */,
      11,   12,  103,    2, 0x0a,   17 /* Public */,
      27,    0,  128,    2, 0x08,   30 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int, 0x80000000 | 16, 0x80000000 | 16, 0x80000000 | 16, 0x80000000 | 20, QMetaType::Int, 0x80000000 | 23, QMetaType::Int, 0x80000000 | 23,   12,   13,   14,   15,   17,   18,   19,   21,   22,   24,   25,   26,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject MyGL::staticMetaObject = { {
    QMetaObject::SuperData::link<OpenGLContext::staticMetaObject>(),
    qt_meta_stringdata_ZN4MyGLE.offsetsAndSizes,
    qt_meta_data_ZN4MyGLE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN4MyGLE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MyGL, std::true_type>,
        // method 'glInitialized'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'slot_setRed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'slot_setGreen'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'slot_setBlue'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'slot_setMetallic'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'slot_setRoughness'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'slot_setAO'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'slot_setDisplacement'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'slot_loadEnvMap'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'slot_updateLoomState'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<int> &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<int> &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<int> &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const glm::vec2 &, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const glm::vec3 &, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const glm::vec3 &, std::false_type>,
        // method 'tick'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void MyGL::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MyGL *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->glInitialized(); break;
        case 1: _t->slot_setRed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->slot_setGreen((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->slot_setBlue((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->slot_setMetallic((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->slot_setRoughness((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->slot_setAO((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->slot_setDisplacement((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 8: _t->slot_loadEnvMap(); break;
        case 9: _t->slot_updateLoomState((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[6])),(*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[7])),(*reinterpret_cast< std::add_pointer_t<glm::vec2>>(_a[8])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[9])),(*reinterpret_cast< std::add_pointer_t<glm::vec3>>(_a[10])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[11])),(*reinterpret_cast< std::add_pointer_t<glm::vec3>>(_a[12]))); break;
        case 10: _t->tick(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 9:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 6:
            case 5:
            case 4:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (MyGL::*)();
            if (_q_method_type _q_method = &MyGL::glInitialized; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *MyGL::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyGL::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN4MyGLE.stringdata0))
        return static_cast<void*>(this);
    return OpenGLContext::qt_metacast(_clname);
}

int MyGL::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = OpenGLContext::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void MyGL::glInitialized()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
