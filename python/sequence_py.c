#include "types_py.h"

static int Sequence_init(Sequence_Data *self, PyObject *args, PyObject *kwds) {

    int nsteps, tps;

    if (!PyArg_ParseTuple(args, "ii", &nsteps, &tps)) {
        return -1;
    }

    sq_sequence_init(&self->seq, nsteps, tps);

    return 0;

}

static PyObject *Sequence_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    Sequence_Data *self;
    self = (Sequence_Data *) type->tp_alloc(type, 0);
    return (PyObject *) self;

}

static void Sequence_del(Sequence_Data *self) {

    Py_TYPE(self)->tp_free((PyObject *) self);

}

static PyObject *Sequence_repr(Sequence_Data *self, PyObject *unused) {

    PyObject *result = NULL;
    char result_str[96];

    sprintf(result_str, "<sequoia sequence: '%s' with %d steps and %d tps>",
            self->seq.name, self->seq.nsteps, self->seq.tps);

    result = PyUnicode_FromString(result_str);

    return result;


}

static PyObject *Sequence_set_name(Sequence_Data *self, PyObject *args) {

    char *name;

    if (!PyArg_ParseTuple(args, "s", &name)) {
        return NULL;
    }

    sq_sequence_set_name(&self->seq, name);

    Py_RETURN_NONE;

}

static PyObject *Sequence_set_transpose(Sequence_Data *self, PyObject *args) {

    int transpose;

    if (!PyArg_ParseTuple(args, "i", &transpose)) {
        return NULL;
    }

    sq_sequence_set_transpose(&self->seq, transpose);

    Py_RETURN_NONE;

}

static PyObject *Sequence_set_playhead(Sequence_Data *self, PyObject *args) {

    int ph;

    if (!PyArg_ParseTuple(args, "i", &ph)) {
        return NULL;
    }

    sq_sequence_set_transpose(&self->seq, ph);

    Py_RETURN_NONE;

}

static PyObject *Sequence_set_clockdivide(Sequence_Data *self, PyObject *args) {

    int div;

    if (!PyArg_ParseTuple(args, "i", &div)) {
        return NULL;
    }

    sq_sequence_set_clockdivide(&self->seq, div);

    Py_RETURN_NONE;

}

static PyObject *Sequence_set_mute(Sequence_Data *self, PyObject *args) {

    bool mute;

    if (!PyArg_ParseTuple(args, "b", &mute)) {
        return NULL;
    }

    sq_sequence_set_mute(&self->seq, mute);

    Py_RETURN_NONE;

}

static PyObject *Sequence_set_trig(Sequence_Data *self, PyObject *args) {

    int stepIndex;
    PyObject *object;

    if (!PyArg_ParseTuple(args, "iO", &stepIndex, &object)) {
        return NULL;
    }

    sq_sequence_set_trig(&self->seq, stepIndex, &((Trigger_Data*)object)->trig);

    Py_RETURN_NONE;

}

static PyObject *Sequence_clear_trig(Sequence_Data *self, PyObject *args) {

    int stepIndex;

    if (!PyArg_ParseTuple(args, "i", &stepIndex)) {
        return NULL;
    }

    sq_sequence_clear_trig(&self->seq, stepIndex);

    Py_RETURN_NONE;

}

static PyObject *Sequence_set_outport(Sequence_Data *self, PyObject *args) {

    PyObject *object;

    if (!PyArg_ParseTuple(args, "O", &object)) {
        return NULL;
    }

    sq_sequence_set_outport(&self->seq, &((Outport_Data*)object)->outport);

    Py_RETURN_NONE;

}

static PyObject *Sequence_pprint(Sequence_Data *self, PyObject *args) {

    sq_sequence_pprint(&self->seq);

    Py_RETURN_NONE;

}

static PyObject *Sequence_get_nsteps(Sequence_Data *self, PyObject *args) {

    int result;
    result = sq_sequence_get_nsteps(&self->seq);

    return PyInt_FromLong(result);

}

static PyMethodDef Sequence_methods[] = {

    {"set_name", (PyCFunction) Sequence_set_name, METH_VARARGS, NULL},
    {"set_outport", (PyCFunction) Sequence_set_outport, METH_VARARGS, NULL},
    {"set_transpose", (PyCFunction) Sequence_set_transpose, METH_VARARGS, NULL},
    {"set_playhead", (PyCFunction) Sequence_set_playhead, METH_VARARGS, NULL},
    {"set_clockdivide", (PyCFunction) Sequence_set_clockdivide, METH_VARARGS, NULL},
    {"set_mute", (PyCFunction) Sequence_set_mute, METH_VARARGS, NULL},
    {"set_trig", (PyCFunction) Sequence_set_trig, METH_VARARGS, NULL},
    {"clear_trig", (PyCFunction) Sequence_clear_trig, METH_VARARGS, NULL},
    {"pprint", (PyCFunction) Sequence_pprint, METH_NOARGS, NULL},
    {"get_nsteps", (PyCFunction) Sequence_get_nsteps, METH_NOARGS, NULL},
    {NULL}

};

PyTypeObject Sequence_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "sequoia.sequence",                 /* tp_name           */
    sizeof (Sequence_Data),             /* tp_basicsize      */
    0,                            /* tp_itemsize       */
    (destructor) Sequence_del,     /* tp_dealloc        */
    0,                            /* tp_print          */
    0,                            /* tp_getattr        */
    0,                            /* tp_setattr        */
    0,                            /* tp_compare        */
    (reprfunc) Sequence_repr,      /* tp_repr           */
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
    //Sequence_doc,                  /* tp_doc            */
    0,                  /* tp_doc            */

    0,                            /* tp_traverse       */
    0,                            /* tp_clear          */
    0,                            /* tp_richcompare    */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter           */
    0,                            /* tp_iternext       */

    // TODO
    Sequence_methods,              /* tp_methods        */
    //Sequence_members,              /* tp_members        */
    //Sequence_getseters,            /* tp_getset         */
    0,              /* tp_members        */
    0,            /* tp_getset         */

    0,                            /* tp_base           */
    0,                            /* tp_dict           */
    0,                            /* tp_descr_get      */
    0,                            /* tp_descr_set      */
    0,                            /* tp_dictoffset     */
    (initproc) Sequence_init,      /* tp_init           */
    0,                            /* tp_alloc          */
    Sequence_new,                  /* tp_new            */
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
