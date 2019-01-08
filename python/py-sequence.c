#include "sequoia-types.h"

static int Py_sequence_init(Py_sequence *self, PyObject *args, PyObject *kwds) {

    int nsteps, tps;

    if (!PyArg_ParseTuple(args, "ii", &nsteps, &tps)) {
        return -1;
    }

    sq_sequence_init(&self->seq, nsteps, tps);

    return 0;

}

static PyObject *Py_sequence_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    Py_sequence *self;
    self = (Py_sequence *) type->tp_alloc(type, 0);
    return (PyObject *) self;

}

static void Py_sequence_del(Py_sequence *self) {

    Py_TYPE(self)->tp_free((PyObject *) self);

}

static PyObject *Py_sequence_repr(Py_sequence *self, PyObject *unused) {

    PyObject *result = NULL;
    char result_str[96];

    sprintf(result_str, "<sequoia sequence: '%s' with %d steps and %d tps>",
            self->seq.name, self->seq.nsteps, self->seq.tps);

    result = PyUnicode_FromString(result_str);

    return result;


}

static PyObject *Py_sequence_set_name(Py_sequence *self, PyObject *args) {

    char *name;

    if (!PyArg_ParseTuple(args, "s", &name)) {
        return NULL;
    }

    sq_sequence_set_name(&self->seq, name);

    Py_RETURN_NONE;

}

static PyObject *Py_sequence_set_transpose(Py_sequence *self, PyObject *args) {

    int transpose;

    if (!PyArg_ParseTuple(args, "i", &transpose)) {
        return NULL;
    }

    sq_sequence_set_transpose(&self->seq, transpose);

    Py_RETURN_NONE;

}

static PyObject *Py_sequence_set_playhead(Py_sequence *self, PyObject *args) {

    int ph;

    if (!PyArg_ParseTuple(args, "i", &ph)) {
        return NULL;
    }

    sq_sequence_set_transpose(&self->seq, ph);

    Py_RETURN_NONE;

}

static PyObject *Py_sequence_set_trig(Py_sequence *self, PyObject *args) {

    int stepIndex;
    PyObject *object;

    if (!PyArg_ParseTuple(args, "iO", &stepIndex, &object)) {
        return NULL;
    }

    sq_sequence_set_trig(&self->seq, stepIndex, &((Py_trigger*)object)->trig);

    Py_RETURN_NONE;

}

static PyObject *Py_sequence_clear_trig(Py_sequence *self, PyObject *args) {

    int stepIndex;

    if (!PyArg_ParseTuple(args, "i", &stepIndex)) {
        return NULL;
    }

    sq_sequence_clear_trig(&self->seq, stepIndex);

    Py_RETURN_NONE;

}

static PyObject *Py_sequence_set_outport(Py_sequence *self, PyObject *args) {

    PyObject *object;
    jack_port_t *outport;

    if (!PyArg_ParseTuple(args, "O", &object)) {
        return NULL;
    }

    outport = (jack_port_t*) PyCObject_AsVoidPtr(object);

    sq_sequence_set_outport(&self->seq, outport);

    Py_RETURN_NONE;

}

static PyObject *Py_sequence_pprint(Py_sequence *self, PyObject *args) {

    sq_sequence_pprint(&self->seq);

    Py_RETURN_NONE;

}

static PyMethodDef Py_sequence_methods[] = {

    {"set_name", (PyCFunction) Py_sequence_set_name, METH_VARARGS, NULL},
    {"set_outport", (PyCFunction) Py_sequence_set_outport, METH_VARARGS, NULL},
    {"set_transpose", (PyCFunction) Py_sequence_set_transpose, METH_VARARGS, NULL},
    {"set_playhead", (PyCFunction) Py_sequence_set_playhead, METH_VARARGS, NULL},
    {"set_trig", (PyCFunction) Py_sequence_set_trig, METH_VARARGS, NULL},
    {"clear_trig", (PyCFunction) Py_sequence_clear_trig, METH_VARARGS, NULL},
    {"pprint", (PyCFunction) Py_sequence_pprint, METH_NOARGS, NULL},
    {NULL}

};

PyTypeObject Py_sequenceType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "sequoia.sequence",                 /* tp_name           */
    sizeof (Py_sequence),             /* tp_basicsize      */
    0,                            /* tp_itemsize       */
    (destructor) Py_sequence_del,     /* tp_dealloc        */
    0,                            /* tp_print          */
    0,                            /* tp_getattr        */
    0,                            /* tp_setattr        */
    0,                            /* tp_compare        */
    (reprfunc) Py_sequence_repr,      /* tp_repr           */
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
    //Py_sequence_doc,                  /* tp_doc            */
    0,                  /* tp_doc            */

    0,                            /* tp_traverse       */
    0,                            /* tp_clear          */
    0,                            /* tp_richcompare    */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter           */
    0,                            /* tp_iternext       */

    // TODO
    Py_sequence_methods,              /* tp_methods        */
    //Py_sequence_members,              /* tp_members        */
    //Py_sequence_getseters,            /* tp_getset         */
    0,              /* tp_members        */
    0,            /* tp_getset         */

    0,                            /* tp_base           */
    0,                            /* tp_dict           */
    0,                            /* tp_descr_get      */
    0,                            /* tp_descr_set      */
    0,                            /* tp_dictoffset     */
    (initproc) Py_sequence_init,      /* tp_init           */
    0,                            /* tp_alloc          */
    Py_sequence_new,                  /* tp_new            */
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
