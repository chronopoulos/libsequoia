#include <Python.h>

#include <sequoia.h>

typedef struct {

    PyObject_HEAD

    sq_session_t sesh;

} Session_Data;

typedef struct {

    PyObject_HEAD

    sq_sequence_t seq;

} Sequence_Data;

typedef struct {

    PyObject_HEAD

    sq_trigger_t trig;

} Trigger_Data;

typedef struct {

    PyObject_HEAD

    sq_outport_t outport;

} Outport_Data;

typedef struct {

    PyObject_HEAD

    sq_inport_t inport;

} Inport_Data;

extern PyTypeObject Session_Type;
extern PyTypeObject Sequence_Type;
extern PyTypeObject Trigger_Type;
extern PyTypeObject Outport_Type;
extern PyTypeObject Inport_Type;

