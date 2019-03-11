#include "sequoia-types.h"

static int Py_port_init(Py_port *self, PyObject *args, PyObject *kwds) {

    int type; // use enum port_type?
    char *name;

    if (!PyArg_ParseTuple(args, "is", &type, &name)) {
        return -1;
    }

    sq_port_init(&self->port, type, name);

    return 0;

}

static PyObject *Py_port_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    Py_port *self;
    self = (Py_port *) type->tp_alloc(type, 0);
    return (PyObject *) self;

}

static void Py_port_del(Py_port *self) {

    Py_TYPE(self)->tp_free((PyObject *) self);

}

static PyObject *Py_port_repr(Py_port *self, PyObject *unused) {

    PyObject *result = NULL;
    char result_str[96];

    sprintf(result_str, "<sequoia outport: %s>", self->port.name);

    result = PyUnicode_FromString(result_str);

    return result;

}

static PyMethodDef Py_port_methods[] = {

    {NULL}

};

PyTypeObject Py_portType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "sequoia.outport",                 /* tp_name           */
    sizeof (Py_port),             /* tp_basicsize      */
    0,                            /* tp_itemsize       */
    (destructor) Py_port_del,     /* tp_dealloc        */
    0,                            /* tp_print          */
    0,                            /* tp_getattr        */
    0,                            /* tp_setattr        */
    0,                            /* tp_compare        */
    (reprfunc) Py_port_repr,      /* tp_repr           */
    0,                            /* tp_as_number      */
    0, //&Py_cvec_tp_as_outport, /* tp_as_outport    */
    0,                            /* tp_as_mapping     */
    0,                            /* tp_hash           */
    0,                            /* tp_call           */
    0,                            /* tp_str            */
    0,                            /* tp_getattro       */
    0,                            /* tp_setattro       */
    0,                            /* tp_as_buffer      */
    Py_TPFLAGS_DEFAULT,           /* tp_flags          */

    // TODO
    //Py_port_doc,                  /* tp_doc            */
    0,                  /* tp_doc            */

    0,                            /* tp_traverse       */
    0,                            /* tp_clear          */
    0,                            /* tp_richcompare    */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter           */
    0,                            /* tp_iternext       */

    // TODO
    Py_port_methods,              /* tp_methods        */
    //Py_port_members,              /* tp_members        */
    //Py_port_getseters,            /* tp_getset         */
    0,              /* tp_members        */
    0,            /* tp_getset         */

    0,                            /* tp_base           */
    0,                            /* tp_dict           */
    0,                            /* tp_descr_get      */
    0,                            /* tp_descr_set      */
    0,                            /* tp_dictoffset     */
    (initproc) Py_port_init,      /* tp_init           */
    0,                            /* tp_alloc          */
    Py_port_new,                  /* tp_new            */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};
