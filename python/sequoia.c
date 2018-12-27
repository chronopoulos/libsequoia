#include <Python.h>

static PyObject *session_init(PyObject *self, PyObject *args) {

    Py_INCREF(Py_None);
    return Py_None;

}

static PyObject *session_get(PyObject *self, PyObject *args) {

    Py_INCREF(Py_None);
    return Py_None;

}

static PyMethodDef sessionMethods[] = {

    {"__init__", session_init, METH_VARARGS, "my doc string"},
    {"get", session_get, METH_VARARGS, "my doc string"},
    {NULL, NULL, 0, NULL}

};

static PyMethodDef moduleMethods[] = {

    {NULL, NULL, 0, NULL}

};

PyMODINIT_FUNC initsequoia(void) {

    // set up the module
    PyObject *module = Py_InitModule("sequoia", moduleMethods);
    PyObject *moduleDict = PyModule_GetDict(module);

    // set up the session class
    PyObject *sessionDict = PyDict_New();
    PyObject *sessionName = PyString_FromString("session");
    PyObject *sessionClass = PyClass_New(NULL, sessionDict, sessionName);
    for (PyMethodDef *def=sessionMethods; def->ml_name != NULL; def++) {
        PyObject *func = PyCFunction_New(def, NULL);
        PyObject *method = PyMethod_New(func, NULL, sessionClass);
        PyDict_SetItemString(sessionDict, def->ml_name, method);
        Py_DECREF(func);
        Py_DECREF(method);
    }
    PyDict_SetItemString(moduleDict, "session", sessionClass);
    Py_DECREF(sessionDict);
    Py_DECREF(sessionName);
    Py_DECREF(sessionClass);

    // set up the sequence class (TODO)

    // set up the sequence class (TODO)

}

