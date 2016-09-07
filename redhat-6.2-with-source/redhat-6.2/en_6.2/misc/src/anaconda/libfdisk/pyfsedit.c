#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "Python.h"
#include "fsedit.h"

#ifdef GNOME
#include "pygtk.h"
#endif

typedef struct fseditObject_t fseditObject;

static PyObject * libfdiskError;

#ifdef GNOME
static PyObject * doGetWindow(fseditObject * s, PyObject * args);
static PyObject * doGetConfirm (fseditObject * o, PyObject * args);
static PyObject * doSetCallback (fseditObject * o, PyObject * args);
static PyObject * doBack (fseditObject * o, PyObject * args);
static PyObject * doNext (fseditObject * o, PyObject * args);
#endif
static PyObject * doEdit(fseditObject * s, PyObject * args);
static PyObject * doSave(fseditObject * s, PyObject * args);
static PyObject * doGetFstab(fseditObject * s, PyObject * args);
static PyObject * doGetPartitions(fseditObject * s, PyObject * args);
static PyObject * doAttempt (fseditObject * o, PyObject * args);
static PyObject * doSetReadOnly (fseditObject * o, PyObject * args);
    
static PyMethodDef fseditObjectMethods[] = {
#ifdef GNOME
    { "getWindow", (PyCFunction) doGetWindow, METH_VARARGS, NULL },
    { "getConfirm", (PyCFunction) doGetConfirm, METH_VARARGS, NULL },
    { "setCallback", (PyCFunction) doSetCallback, METH_VARARGS, NULL },
    { "next", (PyCFunction) doNext, METH_VARARGS, NULL },
    { "back", (PyCFunction) doBack, METH_VARARGS, NULL },
#endif
    { "edit", (PyCFunction) doEdit, METH_VARARGS, NULL },
    { "save", (PyCFunction) doSave, METH_VARARGS, NULL },
    { "attempt", (PyCFunction) doAttempt, METH_VARARGS, NULL },
    { "getFstab", (PyCFunction) doGetFstab, METH_VARARGS, NULL },
    { "setReadOnly", (PyCFunction) doSetReadOnly, METH_VARARGS, NULL },
    { "partitionList", (PyCFunction) doGetPartitions, METH_VARARGS, NULL },
    { NULL },
};

static fseditObject * newFseditObject (PyObject * s, PyObject * args);

static PyMethodDef fseditModuleMethods[] = {
    { "fsedit", (PyCFunction) newFseditObject, METH_VARARGS, NULL },
    { NULL },
};

typedef void (* simpleCallback)(void);

struct fseditObject_t {
    PyObject_HEAD;
    int testing;
    void * state;
    char ** deviceList;
    FseditContext *context;
#ifdef GNOME
    PyObject *enableForward;
#endif
} ;


static PyObject * fseditGetAttr(fseditObject * o, char * name);
static void fseditDealloc (fseditObject * o);

static PyTypeObject fseditType = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,				/* ob_size */
	"fsedit",			/* tp_name */
	sizeof(fseditObject),		/* tp_size */
	0,				/* tp_itemsize */
	(destructor) fseditDealloc, 	/* tp_dealloc */
	0,				/* tp_print */
	(getattrfunc) fseditGetAttr, 	/* tp_getattr */
	0,				/* tp_setattr */
	0,				/* tp_compare */
	0,				/* tp_repr */
	0,				/* tp_as_number */
	0,				/* tp_as_sequence */
	0,				/* tp_as_mapping */
};

static PyObject * fseditGetAttr(fseditObject * o, char * name) {
    return Py_FindMethod(fseditObjectMethods, (PyObject * ) o, name);
}

static void fseditDealloc (fseditObject * o) {
    PyObject * function, * data;

    function = (PyObject *) o->context->cbi.function;
    data = (PyObject *) o->context->cbi.data;
    Py_XDECREF (function);
    Py_XDECREF (data);

    free (o->deviceList);
    delFseditContext (o->context);
}

#ifdef GNOME
GtkWidget * create_main_window (void);
GtkWidget * create_confirm_window (void);
typedef void (* enableCallback)(PyObject *cb, int disable);

void fsedit_enable_next (void *function, void *data, int enable)
{
    PyObject *args, *result;
    PyObject * enableFunction = function;

    PyGtk_BlockThreads ();
    if (!PyCallable_Check(enableFunction)) {
	PyGtk_UnblockThreads ();
	return;
    }

    args = Py_BuildValue("(i)", enable);

    result = PyEval_CallObject(enableFunction, args);

    if (!result) {
	PyErr_Print();
	PyErr_Clear();
    }

    Py_DECREF (args);
    Py_XDECREF (result);
    PyGtk_UnblockThreads ();
}

void next_cb (void);
void back_cb (void);

static PyObject * doBack (fseditObject * o, PyObject * args) {
    back_cb ();
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * doNext (fseditObject * o, PyObject * args) {
    next_cb ();
	
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * doSetCallback (fseditObject * o, PyObject * args) {
    PyObject *function = NULL, *data = NULL;

    if (!PyArg_ParseTuple(args, "O|O", &function, &data)) return NULL;

    o->context->cbi.function = function;
    o->context->cbi.data = data;
    Py_INCREF (function);
    Py_XINCREF (data);
    
    Py_INCREF(Py_None);
    return Py_None;
}

extern GtkAccelGroup *accelgroup;
static PyObject * doGetWindow (fseditObject * o, PyObject * args) {
    GtkObject *mainwin;

    mainwin = (GtkObject *) create_main_window ();
    gtk_object_set_data (mainwin, "accelgroup",
                         PyGtkAccelGroup_New (accelgroup));
    
    return PyGtk_New(mainwin);
}

static PyObject * doGetConfirm (fseditObject * o, PyObject * args) {
    GtkObject *window;

    window = (GtkObject *) create_confirm_window ();
    if (window)
	return PyGtk_New(window);
    else {
	Py_INCREF(Py_None);
	return Py_None;
    }
}

#endif

struct repartitionInfo {
    HardDrive ** hdarr;
    int numDrives;
};

static PyObject * doSave (fseditObject * o, PyObject * args) {
    int rc;

    if (o->context->repartInfo) {
	if ((rc = FSWritePartitions(o->deviceList, o->context->repartInfo))) {
	    char error[256];
	    struct repartitionInfo * rpi = o->context->repartInfo;
	    snprintf (error, sizeof (error) - 1, "Error partitioning drive %s: %s",
		      rpi->hdarr[rc - 1]->name, strerror (errno));
	    PyErr_SetString(PyExc_SystemError, error); 
	    return NULL;
	}
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * doAttempt (fseditObject * o, PyObject * args) {
    PyObject *tuple, *list;
    struct attemptedPartition * attempt;
    int i, size, type, grow, start, flags, rc, partNum = -1;
    int maxSize = -1;
    char *mount, *name;
    char * device; 

    if (!PyArg_ParseTuple(args, "Osi", &list, &name, &flags)) return NULL;
    
    if (!PyList_Check(list)) {
    	PyErr_SetString(PyExc_TypeError, "list argument expected"); 
	return NULL;
    }

    attempt = alloca (sizeof (struct attemptedPartition) * PyList_Size(list) + 1);
    
    for (i = 0; i < PyList_Size(list); i++) {
    	tuple = PyList_GetItem(list, i);
	if (!PyArg_ParseTuple(tuple, "siiiiis", &mount, &size, &maxSize, &type, 
			      &grow, &start, &device)) return NULL;
	attempt[i].mount = strdup (mount);
	attempt[i].size = size;
	attempt[i].type = type;
	attempt[i].grow = grow;
	attempt[i].start = start;
	attempt[i].partNum = partNum;
	attempt[i].maxSize = maxSize;
	if (!strlen(device))
	    attempt[i].device = NULL;
	else
	    attempt[i].device = device;
    }
    attempt[i].mount = NULL;

    rc = guessAtPartitioning (o->context, flags, name, attempt, 0);

    for (i = 0; i < PyList_Size(list); i++) {
	free (attempt[i].mount);
    }

    if (rc) {
	PyErr_SetString(libfdiskError, "autopartitioning failed\n");
	return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * doGetPartitions(fseditObject * o, PyObject * args) {
    PyObject * list;
    PyObject * raidList;
    PyObject * deviceList;
    partitionList * result;
    int i, j;
    char * raidType;

    result = FSPartitionList(o->context);
    list = PyList_New(0);
    for (i = 0; result[i].device; i++) {
	PyList_Append(list, Py_BuildValue("(ssiii)", result[i].device, 
		      result[i].name, result[i].type, result[i].start, result[i].size));
    }

    if (o->context->raid) {
	raidList = PyList_New(0);

	i = 0;
	while (o->context->raid[i].mntpt) {
	    deviceList = PyList_New(0);
	    
	    j = 0;
	    while (o->context->raid[i].devices[j]) {
		PyList_Append(deviceList, 
			  PyString_FromString(o->context->raid[i].devices[j]));
		j++;
	    }

	    if (o->context->raid[i].parttype == LINUX_SWAP_PARTITION)
		raidType = "swap";
	    else
		raidType = "ext2";

	    PyList_Append(raidList, Py_BuildValue("(sssiiiO)",
		    o->context->raid[i].mntpt, o->context->raid[i].device,
		    raidType, o->context->raid[i].type,
		    result[i].start, result[i].size, deviceList));

	    i++;
	}

    } else {
	raidList = Py_None;
	Py_INCREF(raidList);
    }

    for (i = 0; result[i].device; i++) {
	free(result[i].device);
	free(result[i].name);
    }

    free(result);
    
    return Py_BuildValue("(OO)", list, raidList);
}

static PyObject * doGetFstab (fseditObject * o, PyObject * args) {
    PyObject *list;
    int i;
    char *fsystem;
    
    list = PyList_New(o->context->fstab.numEntries);
    for (i = 0; i < o->context->fstab.numEntries; i++) {
	switch (o->context->fstab.entries[i].type) {
	case PART_EXT2:
	    fsystem = "ext2";
	    break;
	case PART_SWAP:
	    fsystem = "swap";
	    break;
	case PART_DOS:
	case PART_FAT32:
	    fsystem = "vfat";
	    break;
	default:
	    fsystem = "auto";
	    break;
	}
	
	PyList_SetItem(list, i,
		       Py_BuildValue("(sssi)",
				     o->context->fstab.entries[i].device,
				     o->context->fstab.entries[i].mntpoint,
				     fsystem,
				     o->context->fstab.entries[i].size));
    }
    return list;
}

static PyObject * doEdit(fseditObject * o, PyObject * args) {
    int dir;
    
    dir = FSEditPartitions(o->context);

    return Py_BuildValue("i", dir);
}

static PyObject * doSetReadOnly(fseditObject *o, PyObject *args ) {
    int readOnly;

    if (!PyArg_ParseTuple(args, "i", &readOnly))
	return NULL;
    
    if (readOnly)
	o->context->flags |= FSEDIT_READONLY;
    else
	o->context->flags &= ~FSEDIT_READONLY;

    Py_INCREF(Py_None);
    return Py_None;
}

static fseditObject * newFseditObject (PyObject * s, PyObject * args) {
    int i, rc;
    PyObject * devList, * oneObject, * fstab;
    int testing;
    fseditObject * o;
    struct fstab fs;
    struct fstabEntry entry;
    int clearMbr = 0;
    int readOnly = 0;
    int ignoreBad = 0;

    if (!PyArg_ParseTuple(args, "iOO|iii", &testing, &devList, &fstab,
			  &clearMbr, &readOnly, &ignoreBad)) return NULL;

    if (!PyList_Check(devList) || !PyList_Check(fstab)) {
    	PyErr_SetString(PyExc_TypeError, "list argument expected");
	return NULL;
    }

    o = PyObject_NEW(fseditObject, &fseditType);

    o->testing = testing;
    o->deviceList = malloc((PyList_Size(devList) + 1) * sizeof(char *));
#ifdef GNOME
    o->enableForward = NULL;
#endif
    for (i = 0; i < PyList_Size(devList); i++) {
    	oneObject = PyList_GetItem(devList, i);
	if (!PyString_Check(oneObject)) {
	    PyErr_SetString(PyExc_TypeError, "list of strings");
	    return NULL;
	}
	o->deviceList[i] = strdup (PyString_AsString(oneObject));
    }
    o->deviceList[i] = NULL;

    memset(&fs, 0, sizeof(fs));

    for (i = 0; i < PyList_Size(fstab); i++) {
    	oneObject = PyList_GetItem(fstab, i);
	memset(&entry, 0, sizeof(entry));
    	if (!PyArg_ParseTuple(oneObject, "ss|i", &entry.device, 
			      &entry.mntpoint, &entry.size)) return NULL;
	entry.device = strdup(entry.device);
	entry.mntpoint = strdup(entry.mntpoint);
	addFstabEntry(&fs, entry);
    }

    if ((rc = newFseditContext (o->deviceList, &fs,
				    (o->testing ? FSEDIT_TESTING : 0) |
				    (readOnly ? FSEDIT_READONLY : 0) |
				    (clearMbr ? FSEDIT_AUTOCLEARMBR : 0) |
				    (ignoreBad ? FSEDIT_IGNOREFAILEDDRIVES : 0),
				&o->context))) {
	PyErr_SetString(PyExc_TypeError, "list of strings");
	return NULL;
    }

    FSEditInitialize(o->context);
    
    return o;
}

void INITFN(void) {
    PyObject * m, * d;
    
    m = Py_InitModule(NAME, fseditModuleMethods);
    d = PyModule_GetDict(m);

    libfdiskError = PyString_FromString("libfdisk.error");
    PyDict_SetItemString(d, "error", libfdiskError);
    PyDict_SetItemString(d, "CLEARALL", PyInt_FromLong(FSEDIT_CLEARALL));
    PyDict_SetItemString(d, "CLEARLINUX", PyInt_FromLong(FSEDIT_CLEARLINUX));
#ifdef GNOME
    init_pygtk();
#endif
    bindtextdomain("anaconda","/usr/share/locale");
    textdomain("anaconda");
}

