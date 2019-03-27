#include "sequoia-types.h"

static int Py_inport_init(Py_inport *self, PyObject *args, PyObject *kwds) {

    char *name;

    if (!PyArg_ParseTuple(args, "s", &name)) {
        return -1;
    }

    sq_inport_init(&self->inport, name);

    return 0;

}

static PyObject *Py_inport_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    Py_inport *self;
    self = (Py_inport *) type->tp_alloc(type, 0);
    return (PyObject *) self;

}

static void Py_inport_del(Py_inport *self) {

    Py_TYPE(self)->tp_free((PyObject *) self);

}

static PyObject *Py_inport_repr(Py_inport *self, PyObject *unused) {

    PyObject *result = NULL;
    char result_str[96];

    sprintf(result_str, "<sequoia in-port: %s>", self->inport.name);
    result = PyUnicode_FromString(result_str);

    return result;

}

static PyObject *Py_inport_set_name(Py_inport *self, PyObject *args) {

    char *name;

    PyArg_ParseTuple(args, "s", &name);

    sq_inport_set_name(&self->inport, name);

    Py_RETURN_NONE;

}

static PyObject *Py_inport_set_type(Py_inport *self, PyObject *args) {

    int type;

    PyArg_ParseTuple(args, "i", &type);

    sq_inport_set_type(&self->inport, type);

    Py_RETURN_NONE;


}

static PyMethodDef Py_inport_methods[] = {

    {"set_name", (PyCFunction) Py_inport_set_name, METH_VARARGS, NULL},
    {"set_type", (PyCFunction) Py_inport_set_type, METH_VARARGS, NULL},
    {NULL}

};

PyTypeObject Py_inportType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "sequoia.inport",                 /* tp_name           */
    sizeof (Py_inport),             /* tp_basicsize      */
    0,                            /* tp_itemsize       */
    (destructor) Py_inport_del,     /* tp_dealloc        */
    0,                            /* tp_print          */
    0,                            /* tp_getattr        */
    0,                            /* tp_setattr        */
    0,                            /* tp_compare        */
    (reprfunc) Py_inport_repr,      /* tp_repr           */
    0,                            /* tp_as_number      */
    0, //&Py_cvec_tp_as_sequence, /* tp_as_sequence    */
    0,                            /* tp_as_mapping     */
    0,                            /* tp_hash           */
    0,                            /* tp_call           */
    0,                            /* tp_str            */
    0,                            /* tp_getattro       */
    0,                            /* tp_setattro       */
    0,                            /* tp_as_buffer      */
    Py_TPFLAGS_DEFAULT,           /* tp_flags          */

    // TODO
    //Py_inport_doc,                  /* tp_doc            */
    0,                  /* tp_doc            */

    0,                            /* tp_traverse       */
    0,                            /* tp_clear          */
    0,                            /* tp_richcompare    */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter           */
    0,                            /* tp_iternext       */

    // TODO
    Py_inport_methods,              /* tp_methods        */
    //Py_inport_members,              /* tp_members        */
    //Py_inport_getseters,            /* tp_getset         */
    0,              /* tp_members        */
    0,            /* tp_getset         */

    0,                            /* tp_base           */
    0,                            /* tp_dict           */
    0,                            /* tp_descr_get      */
    0,                            /* tp_descr_set      */
    0,                            /* tp_dictoffset     */
    (initproc) Py_inport_init,      /* tp_init           */
    0,                            /* tp_alloc          */
    Py_inport_new,                  /* tp_new            */
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
