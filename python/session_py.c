#include "types_py.h"

static int Session_init(Session_Data *self, PyObject *args, PyObject *kwds) {

    char *name;
    int tps;

    if (!PyArg_ParseTuple(args, "si", &name, &tps)) {
        return -1;
    }

    sq_session_init(&self->sesh, name, tps);

    return 0;

}

static PyObject *Session_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    Session_Data *self;
    self = (Session_Data *) type->tp_alloc(type, 0);
    return (PyObject *) self;

}

static void Session_del(Session_Data *self) {

    Py_TYPE(self)->tp_free((PyObject *) self);

}

static PyObject *Session_repr(Session_Data * self, PyObject *unused) {

    PyObject *result = NULL;
    char result_str[96];

    sprintf(result_str, "<sequoia session: '%s' with %d tps at %.3f bpm>",
            sq_session_get_name(&self->sesh), self->sesh.tps, self->sesh.bpm);

    result = PyUnicode_FromString(result_str);

    return result;

}

static PyObject *Session_set_bpm(Session_Data *self, PyObject *args) {

    float bpm;

    if (!PyArg_ParseTuple(args, "f", &bpm)) {
        return NULL;
    }

    sq_session_set_bpm(&self->sesh, bpm);

    Py_RETURN_NONE;

}

static PyObject *Session_start(Session_Data *self, PyObject *unused) {

    sq_session_start(&self->sesh);

    Py_RETURN_NONE;

}

static PyObject *Session_stop(Session_Data *self, PyObject *unused) {

    sq_session_stop(&self->sesh);

    Py_RETURN_NONE;

}

static PyObject *Session_get_name(Session_Data *self, PyObject *unused) {

    return PyString_FromString(sq_session_get_name(&self->sesh));

}

static PyObject *Session_register_outport(Session_Data *self, PyObject *args) {

    PyObject *object;

    if (!PyArg_ParseTuple(args, "O", &object)) {
        return NULL;
    }

    sq_session_register_outport(&self->sesh, &((Outport_Data*)object)->outport);

    Py_RETURN_NONE;

}

static PyObject *Session_add_sequence(Session_Data *self, PyObject *args) {

    PyObject *object;

    if (!PyArg_ParseTuple(args, "O", &object)) {
        return NULL;
    }

    sq_session_add_sequence(&self->sesh, &((Sequence_Data*)object)->seq);

    Py_RETURN_NONE;

}

static PyObject *Session_rm_sequence(Session_Data *self, PyObject *args) {

    PyObject *object;

    if (!PyArg_ParseTuple(args, "O", &object)) {
        return NULL;
    }

    sq_session_rm_sequence(&self->sesh, &((Sequence_Data*)object)->seq);

    Py_RETURN_NONE;

}

static PyObject *Session_get_tps(Session_Data *self, PyObject *args) {

    int result;
    result = sq_session_get_tps(&self->sesh);

    return PyInt_FromLong(result);

}

static PyObject *Session_get_bpm(Session_Data *self, PyObject *args) {

    int result;
    result = sq_session_get_bpm(&self->sesh);

    return PyInt_FromLong(result);

}

static PyMethodDef Session_methods[] = {

    {"set_bpm", (PyCFunction) Session_set_bpm, METH_VARARGS, NULL},
    {"start", (PyCFunction) Session_start, METH_NOARGS, NULL},
    {"stop", (PyCFunction) Session_stop, METH_NOARGS, NULL},
    {"get_name", (PyCFunction) Session_get_name, METH_NOARGS, NULL},
    {"register_outport", (PyCFunction) Session_register_outport, METH_VARARGS, NULL},
    {"add_sequence", (PyCFunction) Session_add_sequence, METH_VARARGS, NULL},
    {"rm_sequence", (PyCFunction) Session_rm_sequence, METH_VARARGS, NULL},
    {"get_tps", (PyCFunction) Session_get_tps, METH_VARARGS, NULL},
    {"get_bpm", (PyCFunction) Session_get_bpm, METH_VARARGS, NULL},
    {NULL}

};

PyTypeObject Session_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "sequoia.session",                 /* tp_name           */
    sizeof (Session_Data),             /* tp_basicsize      */
    0,                            /* tp_itemsize       */
    (destructor) Session_del,     /* tp_dealloc        */
    0,                            /* tp_print          */
    0,                            /* tp_getattr        */
    0,                            /* tp_setattr        */
    0,                            /* tp_compare        */
    (reprfunc) Session_repr,      /* tp_repr           */
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
    //Session_doc,                  /* tp_doc            */
    0,                  /* tp_doc            */

    0,                            /* tp_traverse       */
    0,                            /* tp_clear          */
    0,                            /* tp_richcompare    */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter           */
    0,                            /* tp_iternext       */

    // TODO
    Session_methods,              /* tp_methods        */
    //Session_members,              /* tp_members        */
    //Session_getseters,            /* tp_getset         */
    0,              /* tp_members        */
    0,            /* tp_getset         */

    0,                            /* tp_base           */
    0,                            /* tp_dict           */
    0,                            /* tp_descr_get      */
    0,                            /* tp_descr_set      */
    0,                            /* tp_dictoffset     */
    (initproc) Session_init,      /* tp_init           */
    0,                            /* tp_alloc          */
    Session_new,                  /* tp_new            */
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
