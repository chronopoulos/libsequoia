#include "sequoia-types.h"

static int Py_trigger_init(Py_trigger *self, PyObject *args, PyObject *kwds) {

    sq_trigger_init(&self->trig);

    return 0;

}

static PyObject *Py_trigger_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    Py_trigger *self;
    self = (Py_trigger *) type->tp_alloc(type, 0);
    return (PyObject *) self;

}

static void Py_trigger_del(Py_trigger *self) {

    Py_TYPE(self)->tp_free((PyObject *) self);

}

static PyObject *Py_trigger_repr(Py_trigger *self, PyObject *unused) {

    return PyUnicode_FromString("<sequoia trigger>");

}

static PyObject *Py_trigger_set_null(Py_trigger *self, PyObject *unused) {

    sq_trigger_set_null(&self->trig);

    Py_RETURN_NONE;

}

static PyObject *Py_trigger_set_note(Py_trigger *self, PyObject *args) {

    int note;
    int velocity;
    float length;

    if (!PyArg_ParseTuple(args, "iif", &note, &velocity, &length)) {
        return NULL;
    }

    sq_trigger_set_note(&self->trig, note, velocity, length);

    Py_RETURN_NONE;

}

static PyObject *Py_trigger_set_cc(Py_trigger *self, PyObject *args) {

    int cc_number;
    int cc_value;

    if (!PyArg_ParseTuple(args, "ii", &cc_number, &cc_value)) {
        return NULL;
    }

    sq_trigger_set_cc(&self->trig, cc_number, cc_value);

    Py_RETURN_NONE;

}

static PyObject *Py_trigger_set_probability(Py_trigger *self, PyObject *args) {

    float probability;

    if (!PyArg_ParseTuple(args, "f", &probability)) {
        return NULL;
    }

    sq_trigger_set_probability(&self->trig, probability);

    Py_RETURN_NONE;

}

static PyMethodDef Py_trigger_methods[] = {

    {"set_null", (PyCFunction) Py_trigger_set_null, METH_NOARGS, NULL},
    {"set_note", (PyCFunction) Py_trigger_set_note, METH_VARARGS, NULL},
    {"set_cc", (PyCFunction) Py_trigger_set_cc, METH_VARARGS, NULL},
    {"set_probability", (PyCFunction) Py_trigger_set_probability, METH_VARARGS, NULL},
    {NULL}

};

PyTypeObject Py_triggerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "sequoia.trigger",                 /* tp_name           */
    sizeof (Py_trigger),             /* tp_basicsize      */
    0,                            /* tp_itemsize       */
    (destructor) Py_trigger_del,     /* tp_dealloc        */
    0,                            /* tp_print          */
    0,                            /* tp_getattr        */
    0,                            /* tp_setattr        */
    0,                            /* tp_compare        */
    (reprfunc) Py_trigger_repr,      /* tp_repr           */
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
    //Py_trigger_doc,                  /* tp_doc            */
    0,                  /* tp_doc            */

    0,                            /* tp_traverse       */
    0,                            /* tp_clear          */
    0,                            /* tp_richcompare    */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter           */
    0,                            /* tp_iternext       */

    // TODO
    Py_trigger_methods,              /* tp_methods        */
    //Py_trigger_members,              /* tp_members        */
    //Py_trigger_getseters,            /* tp_getset         */
    0,              /* tp_members        */
    0,            /* tp_getset         */

    0,                            /* tp_base           */
    0,                            /* tp_dict           */
    0,                            /* tp_descr_get      */
    0,                            /* tp_descr_set      */
    0,                            /* tp_dictoffset     */
    (initproc) Py_trigger_init,      /* tp_init           */
    0,                            /* tp_alloc          */
    Py_trigger_new,                  /* tp_new            */
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
