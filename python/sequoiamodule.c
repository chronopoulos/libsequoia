#include "sequoia-types.h"

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

    if (PyType_Ready(&Py_sessionType) < 0) {
        return m;
    }

#if PY_MAJOR_VERSION >= 3
    m = PyModule_Create(&moduledef);
#else
    m = Py_InitModule3 ("sequoia", NULL, NULL);
#endif

    Py_INCREF (&Py_sessionType);
    PyModule_AddObject (m, "session", (PyObject *) & Py_sessionType);
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
