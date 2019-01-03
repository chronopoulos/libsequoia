#include <Python.h>

#include <sequoia.h>

typedef struct {

    PyObject_HEAD

    sq_session_t sesh;

} Py_session;

typedef struct {

    PyObject_HEAD

    sq_sequence_t seq;

} Py_sequence;

typedef struct {

    PyObject_HEAD

    sq_trigger_t trig;

} Py_trigger;

typedef struct {

    PyObject_HEAD

    sq_trigger_t trig;
    jack_port_t *outport;

} Py_outport;

extern PyTypeObject Py_sessionType;
extern PyTypeObject Py_sequenceType;
extern PyTypeObject Py_triggerType;
extern PyTypeObject Py_outportType;

