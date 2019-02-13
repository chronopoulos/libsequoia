#include "sequoia-types.h"

static int Py_session_init(Py_session *self, PyObject *args, PyObject *kwds) {

    char *name;
    int tps;

    PyArg_ParseTuple(args, "si", &name, &tps);

    sq_session_init(&self->sesh, name, tps);

    return 0;

}

static PyObject *Py_session_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    Py_session *self;
    self = (Py_session *) type->tp_alloc(type, 0);
    return (PyObject *) self;

}

static void Py_session_del(Py_session *self) {

    Py_TYPE(self)->tp_free((PyObject *) self);

}

static PyObject *Py_session_repr(Py_session * self, PyObject *unused) {

    PyObject *result = NULL;
    char result_str[96];

    sprintf(result_str, "<sequoia session: '%s' with %d tps at %.3f bpm>",
            sq_session_get_name(&self->sesh), self->sesh.tps, self->sesh.bpm);

    result = PyUnicode_FromString(result_str);

    return result;

}

static PyObject *Py_session_set_bpm(Py_session *self, PyObject *args) {

    float bpm;

    if (!PyArg_ParseTuple(args, "f", &bpm)) {
        return NULL;
    }

    sq_session_set_bpm(&self->sesh, bpm);

    Py_RETURN_NONE;

}

static PyObject *Py_session_start(Py_session *self, PyObject *unused) {

    sq_session_start(&self->sesh);

    Py_RETURN_NONE;

}

static PyObject *Py_session_stop(Py_session *self, PyObject *unused) {

    sq_session_stop(&self->sesh);

    Py_RETURN_NONE;

}

static PyObject *Py_session_get_name(Py_session *self, PyObject *unused) {

    return PyString_FromString(sq_session_get_name(&self->sesh));

}

static PyObject *Py_session_create_outport(Py_session *self, PyObject *args) {

    char *name;
    jack_port_t *outport;

    if (!PyArg_ParseTuple(args, "s", &name)) {
        return NULL;
    }

    outport = sq_session_create_outport(&self->sesh, name);

    return PyCObject_FromVoidPtr((void*) outport, NULL);

}

static PyObject *Py_session_add_sequence(Py_session *self, PyObject *args) {

    PyObject *object;

    if (!PyArg_ParseTuple(args, "O", &object)) {
        return NULL;
    }

    sq_session_add_sequence(&self->sesh, &((Py_sequence*)object)->seq);

    Py_RETURN_NONE;

}

static PyObject *Py_session_rm_sequence(Py_session *self, PyObject *args) {

    PyObject *object;

    if (!PyArg_ParseTuple(args, "O", &object)) {
        return NULL;
    }

    sq_session_rm_sequence(&self->sesh, &((Py_sequence*)object)->seq);

    Py_RETURN_NONE;

}

static PyObject *Py_session_get_tps(Py_session *self, PyObject *args) {

    int result;
    result = sq_session_get_tps(&self->sesh);

    return PyInt_FromLong(result);

}

static PyObject *Py_session_get_bpm(Py_session *self, PyObject *args) {

    int result;
    result = sq_session_get_bpm(&self->sesh);

    return PyInt_FromLong(result);

}

static PyMethodDef Py_session_methods[] = {

    {"set_bpm", (PyCFunction) Py_session_set_bpm, METH_VARARGS, NULL},
    {"start", (PyCFunction) Py_session_start, METH_NOARGS, NULL},
    {"stop", (PyCFunction) Py_session_stop, METH_NOARGS, NULL},
    {"get_name", (PyCFunction) Py_session_get_name, METH_NOARGS, NULL},
    {"create_outport", (PyCFunction) Py_session_create_outport, METH_VARARGS, NULL},
    {"add_sequence", (PyCFunction) Py_session_add_sequence, METH_VARARGS, NULL},
    {"rm_sequence", (PyCFunction) Py_session_rm_sequence, METH_VARARGS, NULL},
    {"get_tps", (PyCFunction) Py_session_get_tps, METH_VARARGS, NULL},
    {"get_bpm", (PyCFunction) Py_session_get_bpm, METH_VARARGS, NULL},
    {NULL}

};

PyTypeObject Py_sessionType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "sequoia.session",                 /* tp_name           */
    sizeof (Py_session),             /* tp_basicsize      */
    0,                            /* tp_itemsize       */
    (destructor) Py_session_del,     /* tp_dealloc        */
    0,                            /* tp_print          */
    0,                            /* tp_getattr        */
    0,                            /* tp_setattr        */
    0,                            /* tp_compare        */
    (reprfunc) Py_session_repr,      /* tp_repr           */
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
    //Py_session_doc,                  /* tp_doc            */
    0,                  /* tp_doc            */

    0,                            /* tp_traverse       */
    0,                            /* tp_clear          */
    0,                            /* tp_richcompare    */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter           */
    0,                            /* tp_iternext       */

    // TODO
    Py_session_methods,              /* tp_methods        */
    //Py_session_members,              /* tp_members        */
    //Py_session_getseters,            /* tp_getset         */
    0,              /* tp_members        */
    0,            /* tp_getset         */

    0,                            /* tp_base           */
    0,                            /* tp_dict           */
    0,                            /* tp_descr_get      */
    0,                            /* tp_descr_set      */
    0,                            /* tp_dictoffset     */
    (initproc) Py_session_init,      /* tp_init           */
    0,                            /* tp_alloc          */
    Py_session_new,                  /* tp_new            */
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
