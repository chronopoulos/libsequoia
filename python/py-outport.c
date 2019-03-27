#include "sequoia-types.h"

static int Outport_init(Outport_Data *self, PyObject *args, PyObject *kwds) {

    char *name;

    if (!PyArg_ParseTuple(args, "s", &name)) {
        return -1;
    }

    sq_outport_init(&self->outport, name);

    return 0;

}

static PyObject *Outport_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    Outport_Data *self;
    self = (Outport_Data *) type->tp_alloc(type, 0);
    return (PyObject *) self;

}

static void Outport_del(Outport_Data *self) {

    Py_TYPE(self)->tp_free((PyObject *) self);

}

static PyObject *Outport_repr(Outport_Data *self, PyObject *unused) {

    PyObject *result = NULL;
    char result_str[96];

    sprintf(result_str, "<sequoia out-port: %s>", self->outport.name);
    result = PyUnicode_FromString(result_str);

    return result;

}

static PyObject *Outport_set_name(Outport_Data *self, PyObject *args) {

    char *name;

    PyArg_ParseTuple(args, "s", &name);

    sq_outport_set_name(&self->outport, name);

    Py_RETURN_NONE;

}

static PyObject *Outport_get_name(Outport_Data *self, PyObject *unused) {

    PyObject *result = NULL;

    result = PyString_FromString(sq_outport_get_name(&self->outport));

    return result;

}

static PyMethodDef Outport_methods[] = {

    {"set_name", (PyCFunction) Outport_set_name, METH_VARARGS, NULL},
    {"get_name", (PyCFunction) Outport_get_name, METH_NOARGS, NULL},
    {NULL}

};

PyTypeObject Outport_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "sequoia.outport",                 /* tp_name           */
    sizeof (Outport_Data),             /* tp_basicsize      */
    0,                            /* tp_itemsize       */
    (destructor) Outport_del,     /* tp_dealloc        */
    0,                            /* tp_print          */
    0,                            /* tp_getattr        */
    0,                            /* tp_setattr        */
    0,                            /* tp_compare        */
    (reprfunc) Outport_repr,      /* tp_repr           */
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
    //Outport_doc,                  /* tp_doc            */
    0,                  /* tp_doc            */

    0,                            /* tp_traverse       */
    0,                            /* tp_clear          */
    0,                            /* tp_richcompare    */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter           */
    0,                            /* tp_iternext       */

    // TODO
    Outport_methods,              /* tp_methods        */
    //Outport_members,              /* tp_members        */
    //Outport_getseters,            /* tp_getset         */
    0,              /* tp_members        */
    0,            /* tp_getset         */

    0,                            /* tp_base           */
    0,                            /* tp_dict           */
    0,                            /* tp_descr_get      */
    0,                            /* tp_descr_set      */
    0,                            /* tp_dictoffset     */
    (initproc) Outport_init,      /* tp_init           */
    0,                            /* tp_alloc          */
    Outport_new,                  /* tp_new            */
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
