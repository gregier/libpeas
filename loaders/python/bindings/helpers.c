#include "helpers.h"

static PyObject *
_helper_wrap_gobject_glist (const GList *list)
{
    PyObject *py_list;
    const GList *tmp;

    if ((py_list = PyList_New(0)) == NULL) {
        return NULL;
    }
    for (tmp = list; tmp != NULL; tmp = tmp->next) {
        PyObject *py_obj = pygobject_new(G_OBJECT(tmp->data));

        if (py_obj == NULL) {
            Py_DECREF(py_list);
            return NULL;
        }
        PyList_Append(py_list, py_obj);
        Py_DECREF(py_obj);
    }
    return py_list;
}
