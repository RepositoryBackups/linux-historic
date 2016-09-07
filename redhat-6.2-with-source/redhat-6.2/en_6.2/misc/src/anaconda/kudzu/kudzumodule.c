/* Copyright 1999 Red Hat, Inc.
 *
 * This software may be freely redistributed under the terms of the GNU
 * public license.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "kudzu.h"

#include "Python.h"

typedef struct {
    char * name;
    int value;
} TableEntry;

static TableEntry classTable[] = {
    { "CLASS_UNSPEC",	CLASS_UNSPEC },
    { "CLASS_OTHER",	CLASS_OTHER },
    { "CLASS_NETWORK",	CLASS_NETWORK },
    { "CLASS_SCSI",	CLASS_SCSI },
    { "CLASS_VIDEO",	CLASS_VIDEO },
    { "CLASS_AUDIO",	CLASS_AUDIO },
    { "CLASS_MOUSE",	CLASS_MOUSE },    
    { "CLASS_MODEM",	CLASS_MODEM },    
    { "CLASS_CDROM",	CLASS_CDROM },    
    { "CLASS_TAPE",	CLASS_TAPE },    
    { "CLASS_FLOPPY",	CLASS_FLOPPY },    
    { "CLASS_SCANNER",	CLASS_SCANNER },    
    { "CLASS_HD",	CLASS_HD },    
    { "CLASS_RAID",	CLASS_RAID },    
    { "CLASS_PRINTER",	CLASS_PRINTER },
    { "CLASS_CAPTURE",	CLASS_CAPTURE },
    { "CLASS_KEYBOARD",	CLASS_KEYBOARD },
    { NULL },
} ;
static int numClassEntries = sizeof (classTable) / sizeof classTable[0];

static TableEntry busTable[] = {
    { "BUS_UNSPEC",	BUS_UNSPEC },
    { "BUS_OTHER",	BUS_OTHER },
    { "BUS_PCI",	BUS_PCI },
    { "BUS_SBUS",	BUS_SBUS },
    { "BUS_PSAUX",	BUS_PSAUX },
    { "BUS_SERIAL",	BUS_SERIAL },
    { "BUS_PARALLEL",	BUS_PARALLEL },
    { "BUS_SCSI",	BUS_SCSI },
    { "BUS_IDE",	BUS_IDE },
    { "BUS_KEYBOARD",	BUS_KEYBOARD },
    /* { "BUS_ISAPNP",	BUS_ISAPNP }, */
    { NULL },
} ;
static int numBusEntries = sizeof (busTable) / sizeof busTable[0];

static TableEntry modeTable[] = {
    { "PROBE_ALL",	PROBE_ALL },
    { "PROBE_SAFE",	PROBE_SAFE },
    { "PROBE_ONE",	PROBE_ONE },
    { NULL },
} ;
static int numModeEntries = sizeof (modeTable) / sizeof modeTable[0];

static PyObject * doProbe (PyObject * self, PyObject * args) {
    int class, bus, mode;
    struct device ** devices, ** tmp;
    PyObject * list;
    
    if (!PyArg_ParseTuple(args, "iii", &class, &bus, &mode))
	return NULL;

    devices = probeDevices(class, bus, mode);

    list = PyList_New(0);

    if (!devices)
	return list;
    
    tmp = devices;
    while (*tmp) {
	PyList_Append (list, Py_BuildValue("(sss)",
					   (*tmp)->device,
					   (*tmp)->driver,
					   (*tmp)->desc));
	tmp++;
    }

    tmp = devices;
    while (*tmp) {
	(*tmp)->freeDevice (*tmp);
	tmp++;
    }

    free(devices);

    return list;
}

static PyMethodDef kudzuMethods[] = {
    { "probe", (PyCFunction) doProbe, METH_VARARGS, NULL },
    { NULL }
} ;

static void registerTable (PyObject * dict, TableEntry * table) {
    int i;
    
    i = 0;
    while (table[i].name) {
	PyDict_SetItemString(dict, table[i].name,
			     PyInt_FromLong (table[i].value));
	i++;
    }
}

void initkudzu (void) {
    PyObject * m, * dict;
    int i;
    
    m = Py_InitModule("kudzu", kudzuMethods);
    dict = PyModule_GetDict(m);

    registerTable (dict, classTable);
    registerTable (dict, busTable);
    registerTable (dict, modeTable);
	
    initializeDeviceList();
}
