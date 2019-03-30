#include "types_py.h"

#if PY_MAJOR_VERSION >= 3
// Python3 module definition
static struct PyModuleDef moduledef = {
   PyModuleDef_HEAD_INIT,
   "sequoia",         /* m_name */
   NULL,              /* m_doc */
   -1,                /* m_size */
   NULL,     /* m_methods */
   NULL,              /* m_reload */
   NULL,              /* m_traverse */
   NULL,              /* m_clear */
   NULL,              /* m_free */
};
#endif

static PyObject *initsequoia_worker(void) {

    PyObject *m = NULL;

    if (PyType_Ready(&Session_Type) < 0) {
        return m;
    }

    if (PyType_Ready(&Sequence_Type) < 0) {
        return m;
    }

    if (PyType_Ready(&Trigger_Type) < 0) {
        return m;
    }

    if (PyType_Ready(&Outport_Type) < 0) {
        return m;
    }

    // inport needs special handling for class attributes
    if ( !(Inport_Type.tp_dict = PyDict_New()) ) {
        return m;
    }
    if (PyType_Ready(&Inport_Type) < 0) {
        return m;
    }
    PyDict_SetItem(Inport_Type.tp_dict, PyString_FromString("NONE"),
                    PyInt_FromLong(INPORT_NONE));
    PyDict_SetItem(Inport_Type.tp_dict, PyString_FromString("TRANSPOSE"),
                    PyInt_FromLong(INPORT_TRANSPOSE));
    PyDict_SetItem(Inport_Type.tp_dict, PyString_FromString("PLAYHEAD"),
                    PyInt_FromLong(INPORT_PLAYHEAD));
    PyDict_SetItem(Inport_Type.tp_dict, PyString_FromString("CLOCKDIVIDE"),
                    PyInt_FromLong(INPORT_CLOCKDIVIDE));
    PyDict_SetItem(Inport_Type.tp_dict, PyString_FromString("DIRECTION"),
                    PyInt_FromLong(INPORT_DIRECTION));
    PyDict_SetItem(Inport_Type.tp_dict, PyString_FromString("MUTE"),
                    PyInt_FromLong(INPORT_MUTE));
    PyDict_SetItem(Inport_Type.tp_dict, PyString_FromString("FIRST"),
                    PyInt_FromLong(INPORT_FIRST));
    PyDict_SetItem(Inport_Type.tp_dict, PyString_FromString("LAST"),
                    PyInt_FromLong(INPORT_LAST));

#if PY_MAJOR_VERSION >= 3
    m = PyModule_Create(&moduledef);
#else
    m = Py_InitModule3 ("sequoia", NULL, NULL);
#endif

    Py_INCREF (&Session_Type);
    PyModule_AddObject (m, "session", (PyObject *) &Session_Type);

    Py_INCREF (&Sequence_Type);
    PyModule_AddObject (m, "sequence", (PyObject *) &Sequence_Type);

    Py_INCREF (&Trigger_Type);
    PyModule_AddObject (m, "trigger", (PyObject *) &Trigger_Type);

    Py_INCREF (&Outport_Type);
    PyModule_AddObject (m, "outport", (PyObject *) &Outport_Type);

    Py_INCREF (&Inport_Type);
    PyModule_AddObject (m, "inport", (PyObject *) &Inport_Type);

    return m;

}

#if PY_MAJOR_VERSION >= 3
    // Python3 init
    PyMODINIT_FUNC PyInit_sequoia(void)
    {
        return initsequoia_worker();
    }
#else
    // Python 2 init
    PyMODINIT_FUNC initsequoia(void)
    {
        initsequoia_worker();
    }
#endif
