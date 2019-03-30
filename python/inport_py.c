#include "types_py.h"

static int Inport_init(Inport_Data *self, PyObject *args, PyObject *kwds) {

    char *name;

    if (!PyArg_ParseTuple(args, "s", &name)) {
        return -1;
    }

    sq_inport_init(&self->inport, name);

    return 0;

}

static PyObject *Inport_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    Inport_Data *self;
    self = (Inport_Data *) type->tp_alloc(type, 0);
    return (PyObject *) self;

}

static void Inport_del(Inport_Data *self) {

    Py_TYPE(self)->tp_free((PyObject *) self);

}

static PyObject *Inport_repr(Inport_Data *self, PyObject *unused) {

    PyObject *result = NULL;
    char result_str[96];

    sprintf(result_str, "<sequoia in-port: %s>", self->inport.name);
    result = PyUnicode_FromString(result_str);

    return result;

}

static PyObject *Inport_set_name(Inport_Data *self, PyObject *args) {

    char *name;

    PyArg_ParseTuple(args, "s", &name);

    sq_inport_set_name(&self->inport, name);

    Py_RETURN_NONE;

}

static PyObject *Inport_set_type(Inport_Data *self, PyObject *args) {

    int type;

    PyArg_ParseTuple(args, "i", &type);

    sq_inport_set_type(&self->inport, type);

    Py_RETURN_NONE;


}

static PyMethodDef Inport_methods[] = {

    {"set_name", (PyCFunction) Inport_set_name, METH_VARARGS, NULL},
    {"set_type", (PyCFunction) Inport_set_type, METH_VARARGS, NULL},
    {NULL}

};

PyTypeObject Inport_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "sequoia.inport",                 /* tp_name           */
    sizeof (Inport_Data),             /* tp_basicsize      */
    0,                            /* tp_itemsize       */
    (destructor) Inport_del,     /* tp_dealloc        */
    0,                            /* tp_print          */
    0,                            /* tp_getattr        */
    0,                            /* tp_setattr        */
    0,                            /* tp_compare        */
    (reprfunc) Inport_repr,      /* tp_repr           */
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
    //Inport_doc,                  /* tp_doc            */
    0,                  /* tp_doc            */

    0,                            /* tp_traverse       */
    0,                            /* tp_clear          */
    0,                            /* tp_richcompare    */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter           */
    0,                            /* tp_iternext       */

    // TODO
    Inport_methods,              /* tp_methods        */
    //Inport_members,              /* tp_members        */
    //Inport_getseters,            /* tp_getset         */
    0,              /* tp_members        */
    0,            /* tp_getset         */

    0,                            /* tp_base           */
    0,                            /* tp_dict           */
    0,                            /* tp_descr_get      */
    0,                            /* tp_descr_set      */
    0,                            /* tp_dictoffset     */
    (initproc) Inport_init,      /* tp_init           */
    0,                            /* tp_alloc          */
    Inport_new,                  /* tp_new            */
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
