#include <Python.h>
#include <structmember.h>

#include "quadtree.h"

using namespace std;

typedef struct {
    PyObject_HEAD
    QuadTree tree;
} PyQuadTree;

static PyObject *
QuadTree_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyQuadTree *self;

    self = (PyQuadTree *)type->tp_alloc(type, 0);
    self->tree = QuadTree();

    return (PyObject *)self;
}

PyObject *key;
Position getPos(void* item) {
    Position pos(0,0);
    if (!PyFunction_Check(key)) {
        cout << "not a function";
        exit(1);
    }
    PyObject *tuple = PyObject_CallFunctionObjArgs(key, (PyObject *)item, NULL);
    if (!PyTuple_Check(tuple)) {
        cout << "wrong type";
        exit(0);
    }
    PyArg_ParseTuple(tuple, "dd", &pos.x, &pos.y);
    
    return pos;
}

static int
QuadTree_init(PyQuadTree *self, PyObject *args, PyObject *kwds)
{
    PyObject *items=NULL;

    static char *kwlist[] = {"items", "key", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "OO", kwlist,
                                      &items, &key))
        return -1;

    int size = PyList_Size(items);
    PyObject **item_array = new PyObject*[size];
    for (int i=0; i<size; ++i) {
        item_array[i] = PyList_GetItem(items, i);
        Py_INCREF(item_array[i]);
    }

    self->tree.init(size, (void **)item_array, getPos);

    return 0;
}

void releaseItem(void *item) {
    Py_XDECREF((PyObject *)item);
}

static void
QuadTree_dealloc(PyQuadTree* self)
{
    self->tree.forAllItems(releaseItem);
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
QuadTree_items_near(PyQuadTree *self, PyObject *args)
{
    int num = 0;
    Position pos(0,0);

    if (! PyArg_ParseTuple(args, "i(dd)", &num, &pos.x, &pos.y))
        return NULL;

    PyObject **items = new PyObject*[num];
    self->tree.getItemsNear(num, pos, (void**)items);

    PyObject *list = (PyObject *)PyList_Type.tp_alloc(&PyList_Type, 0);
    for (int i=0; i<num; ++i) {
        PyList_Append(list, items[i]);
    }

    delete []items;

    return list;
}

static PyObject *
QuadTree_print_tree(PyQuadTree *self)
{
    self->tree.print();

    Py_RETURN_NONE;
}

static PyMemberDef quadtree_members[] = {
    {NULL}  /* Sentinel */
};

static PyMethodDef quadtree_methods[] = {
    {"items_near", (PyCFunction)QuadTree_items_near, METH_VARARGS,
    "Return the closest items to a point" },
    {"print_tree", (PyCFunction)QuadTree_print_tree, METH_NOARGS,
    "Print the quad tree"},
    {NULL}  /* Sentinel */
};

static PyTypeObject quadtree_QuadTreeType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "quadtree.QuadTree",       /*tp_name*/
    sizeof(PyQuadTree), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)QuadTree_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "QuadTree type",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    quadtree_methods,             /* tp_methods */
    quadtree_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)QuadTree_init,      /* tp_init */
    0,                         /* tp_alloc */
    QuadTree_new,                 /* tp_new */
};

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
initquadtree(void) 
{
    PyObject* m;

    quadtree_QuadTreeType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&quadtree_QuadTreeType) < 0)
        return;

    m = Py_InitModule3("quadtree", quadtree_methods,
                       "QuadTree");

    Py_INCREF(&quadtree_QuadTreeType);
    PyModule_AddObject(m, "QuadTree", (PyObject *)&quadtree_QuadTreeType);
}


