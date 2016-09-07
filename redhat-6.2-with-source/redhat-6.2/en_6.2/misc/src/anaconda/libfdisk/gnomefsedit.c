/* gtk+ based frontend to libfdisk - loosely resembling fsedit.c */
/*                                                               */
/* Copyright (C) 1999 Red Hat, Inc.                              */
/* Original work by Michael Fulbright                            */


#include <ctype.h>
#include <gnome.h>

#undef _

#include "libfdisk.h"
#include "fstab.h"
#include "fsedit.h"
#include "gnomefsedit.h"
#include "Python.h"

#define MAIN_WIN_WIDTH  420
#define MAIN_WIN_HEIGHT 370

#define MAX_HARDDRIVES  16
#define SECPERMEG 2048

#define MNTPT_MAXLEN 40

#define NEW_PARTSPEC "NewPartition"

#define UNALLOCATED_COLOR_R 0xffff
#define UNALLOCATED_COLOR_G 0x0000
#define UNALLOCATED_COLOR_B 0x0000

/* define for lots of information */
/*#define GNOME_FSEDIT_DEBUG_OUTPUT */

/* make these globals for now */
static GtkWidget *mainwin;
static GtkWidget *mntpt_clist;
GtkAccelGroup *accelgroup;

static GtkWidget *drvsum_clist;
static GtkWidget *add_button;
static GtkWidget *del_button;
static GtkWidget *edit_button;
static GtkWidget *reset_button;
static GtkWidget *addraid_button;
static GtkWidget *auto_button;

static gboolean ignore_mntpt_clist_events = FALSE;

/* RAID meta-information */
/* this is a mapping between a RAID device (like '/dev/md0') and   */
/* the list of the names of the  partition specs which compose     */
/* that device (eg. 'Raid0000', 'Raid0001', etc). We use the names */
/* because they do not change, but index into partition spec list  */
/* can if user deletes partitions                                  */

char  raidtypesnames[][20] = { [RAID0] = "RAID0",
			       [RAID1] = "RAID1",
			       [RAID5] = "RAID5"};
RaidType   raidtypes[] = {RAID0, RAID1, RAID5};
int   numraidtypes   = sizeof(raidtypes)/sizeof(RaidType);

char *raiddevs[] = { "md0", "md1", "md2", "md3", "md4", "md5",
		     "md6", "md7", "md8" };
int   numraiddevs   = sizeof(raiddevs)/sizeof(char *);

/* XXXX - declared in fsedit.c */
extern struct attemptedPartition *normalPartitioning;
extern struct attemptedPartition *serverPartitioning;

typedef struct RAID_SPEC_ENTRY {
    char            *mntpt;
    char            *device;
    RaidType        type;
    guint           parttype;
    GList           *partitions;
} RaidSpecEntry;

typedef void (* enableCallback)(void *function, void *data, int enable);

int
setupGlobalContext(FseditContext *state);

struct callbackInfo {
    enableCallback marshaller;
    void *function, *data;
};

/* used to store current context so we can pass around to callbacks */
typedef struct _HardDriveContext {
/*    HardDrive **hdarr; */
/*    HardDrive **prestinehdarr; */
/*    PartitionSpec *spec; */
/*    unsigned int  numhd; */
/*    HardDrive **newhdarr; */

    FseditContext *state;

    GList     *raidarray; /* list of RaidSpecEntry's */
    int           dontPartition;
    int           *writeChanges;
    int           changesmade;
} HardDriveContext;

HardDriveContext globalContext;

typedef struct {
    gint      hilite;
    gint      specnum;
    RaidSpecEntry     *raidentry;
} MntptClistRowData;

static void update_mntpt_clist (HardDriveContext *context);
static void update_drvsum_clist (HardDriveContext *context);
static void update_buttonbar (HardDriveContext *context);
static void update_gui (HardDriveContext *context);

static void set_label_color (GtkWidget *label, guint red, 
			     guint green, guint blue);

#ifdef GNOME_FSEDIT_DEBUG_OUTPUT
static void print_context ( HardDriveContext *context );
#endif 

/* a callback into python land to enable/disable the Next button */
void enable_next (int enable);

/* give summary of why partitions weren't allocated */
void showReasons( PartitionSpec *spec );

/* misc functions */
static gchar *
deviceFromSpecEntry (HardDriveContext *context, gint specnum)
{
    Partition *p;
    gint hdidx;

    p = &(context->state->spec.entry[specnum].partition);
    
    for (hdidx=0; hdidx < context->state->numhd; hdidx++)
	if (context->state->newhdarr[hdidx]->num == p->drive.current)
	    break;;
    
    if (hdidx == context->state->numhd) {
	g_warning ("Couldn't find drive of partition!\n");
	return NULL;
    } else {
	return  g_strdup_printf ("%s%d", context->state->newhdarr[hdidx]->prefix,
				 p->num.current);
    }
}

/* RAID Array related commands (will be moved to raid.c) */
static GList *
findRaidDevice (GList *raidarray, gchar *raid_device)
{
    GList *s=raidarray;

    if (!raidarray)
	return NULL;

    while (s) {
	if (!strcmp (raid_device, ((RaidSpecEntry *)s->data)->device))
	    return s;
	s = s->next;
    }

    return NULL;
}

static gboolean
specInRaidSpecEntry (RaidSpecEntry *raidentry, PartitionSpec *spec, gint num)
{
    GList *parts;

    if (!raidentry || !spec || num < 0)
	return FALSE;

    parts = raidentry->partitions;

    while (parts) {
	if (!strcmp (spec->entry[num].name, parts->data))
	    return TRUE;
	parts = parts->next;
    }

    return FALSE;
}

static gboolean
specInRaidArray (GList *raidarray, PartitionSpec *spec, gint num)
{
    GList *s=raidarray;

    if (!raidarray || !spec || num < 0)
	return FALSE;

    while (s) {
	if (specInRaidSpecEntry ( (RaidSpecEntry *)s->data, spec, num))
	    return TRUE;
	s = s->next;
    }

    return FALSE;
}

static RaidSpecEntry *
findMntPtInRaidArray (GList *raidarray, gchar *mntpt)
{
    GList *s=raidarray;

    if (!raidarray || !mntpt)
	return NULL;

    while (s) {
	if (!strcmp (mntpt, ((RaidSpecEntry *)s->data)->mntpt))
	    return (RaidSpecEntry *)s->data;
	s = s->next;
    }

    return NULL;
}

static gboolean
isRaidEntryComplete (RaidType type, GList *sel)
{
    gboolean notenuf;

    notenuf = FALSE;
    switch (type) {
      case RAID0:
	if (g_list_length(sel) < 2)
	    notenuf = TRUE;
	break;
      case RAID1:
	if (g_list_length(sel) < 2)
	    notenuf = TRUE;
	break;
      case RAID5:
	if (g_list_length(sel) < 3)
	    notenuf = TRUE;
	break;
    }

    return !notenuf;
}

static guint
sizeRaidSpecEntry (RaidSpecEntry *raidspec, HardDriveContext *context)
{
    Partition *p;
    guint totsize=0;
    gint  rc, i;
    GList *s;

    if (!isRaidEntryComplete (raidspec->type, raidspec->partitions))
	return 0;

    /* go through all partitions and add sizes */
    s = raidspec->partitions;
    while (s) {
	rc = fdiskIndexPartitionSpec (&context->state->spec, s->data, &i);

	if (!rc) {
	    p = &(context->state->spec.entry[i].partition);
	    switch (raidspec->type) {
	      case RAID0:
		totsize += p->size.current;
		break;
	      case RAID1:
	      case RAID5:
		if (totsize)
		    totsize = MIN (totsize, p->size.current);
		else
		    totsize = p->size.current;
		break;
		
	      default:
		g_warning ("Bad RAID type in sizeRaidEntry!\n");
		break;
	    }
	} else {
	    g_warning ("Couldnt find partiton %s\n",(gchar *)s->data);
	}
	s=s->next;
    }

    if (raidspec->type == RAID5)
	totsize = totsize*(g_list_length(raidspec->partitions)-1);

    return totsize;
}

static void
freeRaidSpecEntry (RaidSpecEntry *p)
{
    if (!p)
	return;

    if (p->mntpt)
	g_free (p->mntpt);
    if (p->device)
	g_free (p->device);
    if (p->partitions)
	g_list_free (p->partitions);

    g_free (p);
}

static gint
deleteRaidSpecEntry (GList **raidarray, RaidSpecEntry *raidentry)
{
    GList *p;

    if (!*raidarray || !raidentry)
	return -1;
    
    p = g_list_find (*raidarray, raidentry);
    freeRaidSpecEntry (p->data);
    *raidarray = g_list_remove (*raidarray, p->data);

    return FDISK_SUCCESS;
}

static void
remove_accelgroup (void)
{
    GtkWidget *toplevel;

    toplevel = gtk_widget_get_toplevel (mainwin);
    
/*      g_assert (toplevel != NULL); */
/*      g_assert (GTK_IS_WINDOW (toplevel)); */

    if (toplevel && GTK_IS_WINDOW (toplevel)) {
	gtk_window_remove_accel_group (GTK_WINDOW (toplevel), accelgroup);
	gtk_accel_group_unref (accelgroup);
    }
    accelgroup = NULL;
}

void
wipeRaidArray (GList **raidarray)
{
    GList *l;

    if (!*raidarray)
	return;

    for (l=*raidarray; l; l=l->next)
	freeRaidSpecEntry (l->data);

    g_list_free (*raidarray);
    *raidarray = NULL;
}

/* callbacks to talk to python/anaconda framework */
void next_cb (void)
{
    HardDriveContext *context = &globalContext;
    gint i, j;
    struct raidInstance * raid;
    RaidSpecEntry * ri;
    GList * node, * node2;

    /* This could be a problem as we don't always bring up the interface
       for fsedit. 
	remove_accelgroup ();*/

    /* free up our locale allocated memory */
    /* XXXX - globalContext is a global variable which was */
    /*        created in StartMaster()                     */

    /* drmike - I don't think this is actually necessary, and in fact
     *          messes up the context for disk druid on next/back
     *          cycles in the installer
     */
#if 0
    for (i=0; i<context->state->numhd; i++) {
      memcpy(context->state->hdarr[i], context->state->newhdarr[i],
	     sizeof(HardDrive));
    }
#endif
/*
    for (i=0; i < context->state->numhd; i++) {
	g_print ("unallocated newhdarr[%d]\n",i);
	free (context->newhdarr[i]);
    }
*/
    /* save state so install can proceed with partitioning information */
    /* converting from the HardDriveContext we use here to the */
    /* FseditContext used in fsedit.c */

    i = 0;
    node = context->raidarray;
    while (node) 
	i++, node = node->next;
    raid = malloc(sizeof(*raid) * (i + 1));

    node = context->raidarray, i = 0;
    while (node) {
        ri = node->data;

	raid[i].mntpt = strdup(ri->mntpt);
	raid[i].device = strdup(ri->device);
	raid[i].type = ri->type;
	raid[i].parttype = ri->parttype;

	j = 0;
	node2 = ri->partitions;
	while (node2)
	    j++, node2 = node2->next;

	raid[i].devices = malloc(sizeof(char *) * (j + 1));

	j = 0;
	node2 = ri->partitions;
	while (node2) {
	    raid[i].devices[j] = strdup(node2->data);
	    j++, node2 = node2->next;
	}

	raid[i].devices[j] = NULL;

	i++, node = node->next;
    }

    raid[i].mntpt = NULL;

#ifdef GNOME_FSEDIT_DEBUG_OUTPUT
    printf ("Next pressed...\n");
    print_context (context);
#endif

    exitFsedit (context->state, raid);
    
    return;
}

void back_cb (void)
{
/*      HardDriveContext *context = &globalContext; */
/*      gint i; */

    /* free up our locale allocated memory */
    /* XXXX - globalContext is a global variable which was */
    /*        created in StartMaster()                     */

    /* This could be a problem as we don't always bring up the interface
       for fsedit. 
	remove_accelgroup ();*/

/*
    for (i=0; i < context->state->numhd; i++) {
	g_print ("unallocated newhdarr[%d]\n",i);
	free (context->newhdarr[i]);
    }
*/
    return;
}


void
mntpt_entry_insert_text_cb (GtkEditable    *editable, const gchar    *text,
                           gint length, gint *position,
                           void *data)
{
    gint i;
    gchar *cur;

    cur = gtk_entry_get_text (GTK_ENTRY (editable));
    if (*cur) { 
	for (i = 0; i < length; i++)
	    if (isspace(text[i])) {
		gtk_signal_emit_stop_by_name (GTK_OBJECT (editable), 
					      "insert_text");
		return;
	    }
    } /*else {
	if (text[0] != '/') {
	    gtk_signal_emit_stop_by_name (GTK_OBJECT (editable), 
					  "insert_text");
	    return;
	}
	}*/
}

void
size_entry_insert_text_cb (GtkEditable    *editable, const gchar    *text,
                           gint length, gint *position,
                           void *data)
{
    gint i;

    for (i = 0; i < length; i++)
	if (!isdigit(text[i])) {
	    gtk_signal_emit_stop_by_name (GTK_OBJECT (editable), 
					  "insert_text");
	    return;
	}
}

static void
type_menu_activate_cb (GtkWidget *widget, gint *curitem)
{

    gint newitem;
    GtkWidget *mntpt_entry;
    GtkWidget *mntpt_combo;
    GtkWidget *menu;
    gchar     *saved_mntpt;

    newitem = GPOINTER_TO_INT(gtk_object_get_data (GTK_OBJECT (widget),
						   "partition_type"));
    menu = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (widget),
						   "menu"));
    mntpt_entry = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (menu),
						   "mntpt_entry"));
    mntpt_combo = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (menu),
						   "mntpt_combo"));
    saved_mntpt = (gchar *) gtk_object_get_data (GTK_OBJECT (menu),
						 "saved_mntpt");

    if (newitem == *curitem)
	return;

/*      g_print ("Partition type 0x%02x selected\n", newitem); */
/*      g_print ("old       type 0x%02x         \n", *curitem); */

    if (newitem == LINUX_SWAP_PARTITION) {
	if (*curitem != LINUX_RAID_PARTITION) {
	    saved_mntpt = g_strdup ( gtk_entry_get_text (GTK_ENTRY (mntpt_entry)));
	    gtk_object_set_data_full (GTK_OBJECT (menu), "saved_mntpt", 
				      saved_mntpt, g_free);
	}

	gtk_entry_set_text (GTK_ENTRY (mntpt_entry), _("<Swap Partition>"));
	gtk_widget_set_sensitive (mntpt_combo, FALSE);
    } else if (newitem == LINUX_RAID_PARTITION) {
	if (*curitem != LINUX_SWAP_PARTITION) {
	    saved_mntpt = g_strdup ( gtk_entry_get_text (GTK_ENTRY (mntpt_entry)));
	    gtk_object_set_data_full (GTK_OBJECT (menu), "saved_mntpt", 
				      saved_mntpt, g_free);
	}
	gtk_entry_set_text (GTK_ENTRY (mntpt_entry), _("<RAID Partition>"));
	gtk_widget_set_sensitive (mntpt_combo, FALSE);
    } else if (*curitem == LINUX_SWAP_PARTITION || *curitem == LINUX_RAID_PARTITION) {
	gtk_widget_set_sensitive (mntpt_combo, TRUE);
	gtk_entry_set_text ( GTK_ENTRY (mntpt_entry), 
			     (saved_mntpt) ? saved_mntpt : "");
    }

    *curitem = newitem;
}

static void
raidtype_menu_activate_cb (GtkWidget *widget, gint *curitem)
{

    gint newitem;

    newitem = GPOINTER_TO_INT(gtk_object_get_data (GTK_OBJECT (widget),
						   "raid_type"));

    if (newitem == *curitem)
	return;

/*      g_print ("Partition type 0x%02x selected\n", newitem); */
/*      g_print ("old       type 0x%02x         \n", *curitem); */

    *curitem = newitem;
}

/* sole purpose is so we can make the drives_clist always have AT LEAST */
/* one entry selected                                                   */
static gint
stop_drives_clist_btnpress (GtkWidget *clist, GdkEventButton *event,
			    gpointer data)
{
    GList *sel = GTK_CLIST (clist)->selection;
    gint row, column;

    if (sel != NULL && g_list_length (sel) == 1) {
	gtk_clist_get_selection_info (GTK_CLIST (clist), event->x, event->y, 
				      &row, &column);
	if (row == GPOINTER_TO_INT (sel->data)) {
	    gtk_signal_emit_stop_by_name (GTK_OBJECT (clist),
					  "button_press_event");
	    return TRUE;
	}
    }
    return FALSE;
}

void raidSetLimitsBootable( char *mntpt, HardDrive **hdarr, 
			    unsigned int numhd, PartitionSpec *spec,
			    GList *raidarray)
{
    RaidSpecEntry *entry;
    GList *parts;
    
    entry = findMntPtInRaidArray (raidarray, mntpt);
    if (entry == NULL) {
	g_warning ("Could not find mntpt %s in raidarray\n", mntpt);
	return;
    }

    parts = entry->partitions;
    
    while (parts) {
	char *partname = parts->data;
	
	fdiskSetPartitionLimitsBootable (partname, hdarr, numhd, spec);
	parts = parts->next;
    }
}

void raidUnsetLimitsBootable( char *mntpt, PartitionSpec *spec,
			      GList *raidarray)
{
    RaidSpecEntry *entry;
    GList *parts;
    
    entry = findMntPtInRaidArray (raidarray, mntpt);
    if (entry == NULL) {
	g_warning ("Could not find mntpt %s in raidarray\n", mntpt);
	return;
    }

    parts = entry->partitions;
    
    while (parts) {
	char *partname = parts->data;
	
	fdiskUnsetPartitionLimitsBootable (partname, spec);
	parts = parts->next;
    }
}


/* try to find a bootable candidate in raid and then normal partitions */
void gnomefseditHandleSpecialPartitions ( HardDrive **hdarr,
					  unsigned int numhd,
					  PartitionSpec *spec,
					  GList *raidarray)
{
    unsigned int i, j;
    RaidSpecEntry *raidentry1, *raidentry2;
    Partition *p1, *p2;

    /* find all locations that might have /boot and / */
    raidentry1 = findMntPtInRaidArray (raidarray, "/boot");
    raidentry2 = findMntPtInRaidArray (raidarray, "/");
    i = fdiskReturnPartitionSpec( spec, "/boot",     &p1 );
    if (i == FDISK_SUCCESS)
	free(p1);
    j = fdiskReturnPartitionSpec( spec, "/", &p2 );
    if (j == FDISK_SUCCESS)
        free(p2);

    /* if /boot in raid array or a normal partition, make its components */
    /* partitions bootable and remove bootable constraints from  "/" if  */
    /* it exists                                                         */
    if ( raidentry1 != NULL ) {
	/* remove bootable restrictions on "/" */
	if ( raidentry2 != NULL )
	    raidUnsetLimitsBootable( "/", spec, raidarray );
	else if (j == FDISK_SUCCESS)
	    fdiskUnsetPartitionLimitsBootable("/", spec);

	raidSetLimitsBootable ( "/boot", hdarr, numhd, spec, raidarray );
    } else if ( i == FDISK_SUCCESS ) {
	/* remove bootable restrictions on "/" */
	if ( raidentry2 != NULL )
	    raidUnsetLimitsBootable( "/", spec, raidarray );
	else if (j == FDISK_SUCCESS)
	    fdiskUnsetPartitionLimitsBootable("/", spec);

	fdiskSetPartitionLimitsBootable("/boot", hdarr, numhd, spec);
    } else if ( raidentry2 != NULL ) {
	raidSetLimitsBootable ( "/", hdarr, numhd, spec, raidarray );
    } else if ( j == FDISK_SUCCESS ) {
	fdiskSetPartitionLimitsBootable("/", hdarr, numhd, spec);
    }

    fdiskSortPartitionSpec( spec );
}

/* helper to setup popdown for default mntpt choices */
static GList *
get_default_mntpt_strings()
{
    GList *mntpt_strings = NULL;

    mntpt_strings = g_list_append (mntpt_strings, "/");
    mntpt_strings = g_list_append (mntpt_strings, "/boot");
    mntpt_strings = g_list_append (mntpt_strings, "/usr");
    mntpt_strings = g_list_append (mntpt_strings, "/home");
    mntpt_strings = g_list_append (mntpt_strings, "/var");
    mntpt_strings = g_list_append (mntpt_strings, "/opt");
    mntpt_strings = g_list_append (mntpt_strings, "/tmp");
    mntpt_strings = g_list_append (mntpt_strings, "/usr/local");

    return mntpt_strings;
}


/* partionspec commands - maybe should split out from newtfsedit and
   gnomefsedit since there is little difference */
static int DeletePartitionSpec( HardDrive **hdarr, unsigned int numhd,
			 PartitionSpec *spec, PartitionSpecEntry *entry,
				int force) {
    if (!force && ddruidYesNo(_("Delete Partition"), _("Yes"), _("No"),
			        _("Are you sure you want to delete "
			          "this partition?"), 0) != 1)
	return FDISK_ERR_USERABORT;

    return doDeletePartitionSpec(hdarr, numhd, spec, entry);
}

static int EditPartitionSpec( HardDrive **hdarr, unsigned int numhd,
			 PartitionSpec *spec, PartitionSpecEntry *entry)
{
    GtkWidget *dialog;
    GtkWidget *dialog_vbox;

    GtkWidget *hbox;
    GtkWidget *mntpt_entry, *mntpt_combo;
    GtkWidget *size_entry;
    GtkWidget *grow_checkbox;
    GtkWidget *type_omenu;
    GtkWidget *drives_clist, *scrollwin;

    GtkWidget *menu;
    GtkWidget *menu_item;

    gint      ret, match_type, current_type;

    int j, k, l;
    unsigned int hdidx;
    Partition *p;

    unsigned int tmpuint;
    int          tmpint;
    char         *eptr;

    char         *mntpt, *size;

    char titlestr[80];
    char tmpstr[80];
    gboolean grow, done;
    GList *mntpt_strings;

    int status=0;
    int newpartition;
    int   pcfstypes[] = {0x82, 0x83, 0xfd, 0x4, 0x6, -1};
    int   sunfstypes[] = {0x82, 0x83, 0xfd, 0x1, 0x2, 0x3, 0x4, 0x6, 0x7, 0x8, -1};
    int   *fstypes;

    p = (Partition *) alloca(sizeof(Partition));
    memcpy(p, &entry->partition, sizeof(Partition));

    newpartition = (strcmp(entry->name, NEW_PARTSPEC) == 0);

    /* make title line a little more descriptive */
    if (newpartition) {
	strcpy(titlestr, "Edit New Partition");
    } else if (p->immutable) {
	for (hdidx=0; hdidx < numhd &&
		 hdarr[hdidx]->num != p->drive.current; hdidx++);
	if (hdidx != numhd) {
	    snprintf(titlestr, 80, "%s: /dev/%s%d", _("Edit Partition"),
		     hdarr[hdidx]->prefix, p->num.current);
	    if (entry->name && *entry->name && strncmp(entry->name,"Exist", 5))
		snprintf(titlestr+strlen(titlestr), 80-strlen(titlestr),
			 " (%s)", entry->name);
	} else {
	    strcpy(titlestr, _("Edit Partition"));
	}
    } else {
	if (entry->name && *entry->name)
	    snprintf(titlestr, 80, "%s: %s", _("Edit Partition"), entry->name);
	else
	    strcpy(titlestr, _("Edit Partition"));
    }	    

    /* code to create dialog in gtk+ */
    dialog = gnome_dialog_new (titlestr, _("Ok"), _("Cancel"),NULL);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

    gnome_dialog_set_default (GNOME_DIALOG (dialog), 0);

    dialog_vbox = GNOME_DIALOG (dialog)->vbox;

    /* pack contents into vbox in dialog */

    /* mount point first */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), gtk_label_new (_("Mount Point:")),
			FALSE, FALSE, 2);


    mntpt_combo = gtk_combo_new ();
    mntpt_strings = get_default_mntpt_strings();
    gtk_combo_set_popdown_strings (GTK_COMBO (mntpt_combo), mntpt_strings);
    g_list_free (mntpt_strings);

    mntpt_entry = GTK_COMBO (mntpt_combo)->entry;
    gtk_entry_set_max_length (GTK_ENTRY (mntpt_entry), MNTPT_MAXLEN);
    gtk_box_pack_start (GTK_BOX (hbox), mntpt_combo, FALSE, FALSE, 2);

    /* make it so typing return inside entry box closes dialog with 'Ok' */
    gnome_dialog_editable_enters (GNOME_DIALOG (dialog), 
				  GTK_EDITABLE (mntpt_entry));

    if (p->type.current != LINUX_SWAP_PARTITION &&
	p->type.current != LINUX_RAID_PARTITION) {
	if (!newpartition && strncmp("Exist", entry->name, 5) &&
	    strncmp("Dos", entry->name, 3)) {
	    gtk_entry_set_text (GTK_ENTRY (mntpt_entry), entry->name);
	} else {
	    gtk_entry_set_text (GTK_ENTRY (mntpt_entry), "");
	}
	gtk_widget_set_sensitive (mntpt_combo, TRUE);
    } else {
	if (p->type.current == LINUX_SWAP_PARTITION)
	    gtk_entry_set_text (GTK_ENTRY (mntpt_entry), _("<Swap Partition>"));
	else
	    gtk_entry_set_text (GTK_ENTRY (mntpt_entry), _("<RAID Partition>"));
	gtk_widget_set_sensitive (mntpt_combo, FALSE);
    }

    gtk_signal_connect (GTK_OBJECT (mntpt_entry), "insert_text", 
			GTK_SIGNAL_FUNC (mntpt_entry_insert_text_cb),
			NULL);

    gtk_widget_show_all (hbox);
    gtk_box_pack_start (GTK_BOX (dialog_vbox), hbox, FALSE, FALSE, 0);

    /* requested size */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), gtk_label_new (_("Size (Megs):")),
			FALSE, FALSE, 2);
    if (p->immutable) {
	size_entry = NULL;
	snprintf(tmpstr,sizeof(tmpstr),"%d", p->size.current/SECPERMEG);
	gtk_box_pack_start (GTK_BOX (hbox), gtk_label_new (tmpstr),
			    FALSE, FALSE, 2);
    } else {
	size_entry = gtk_entry_new_with_max_length (10);
	gtk_box_pack_start (GTK_BOX (hbox), size_entry, FALSE, FALSE, 2);
	snprintf(tmpstr,sizeof(tmpstr),"%d", p->size.min/SECPERMEG);
	gtk_entry_set_text (GTK_ENTRY (size_entry), tmpstr);
	gnome_dialog_editable_enters (GNOME_DIALOG (dialog), 
				      GTK_EDITABLE (size_entry));
	gtk_signal_connect (GTK_OBJECT (size_entry), "insert_text", 
			    GTK_SIGNAL_FUNC (size_entry_insert_text_cb),
			    NULL);
    }

    gtk_widget_show_all (hbox);
    gtk_box_pack_start (GTK_BOX (dialog_vbox), hbox, FALSE, FALSE, 0);

    /* growable? */
    if (!newpartition) {
	grow = p->size.min != p->size.max;
    } else {
	grow = 0;
    }

    hbox = gtk_hbox_new (FALSE, 0);

    grow_checkbox = gtk_check_button_new_with_label (_("Grow to fill disk?"));
    gtk_box_pack_start (GTK_BOX (hbox), grow_checkbox, FALSE, FALSE, 2);

    if (p->immutable) {
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (grow_checkbox),FALSE);
	gtk_widget_set_sensitive (grow_checkbox, FALSE);
    } else {
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (grow_checkbox),grow);
    }

    gtk_widget_show_all (hbox);
    gtk_box_pack_start (GTK_BOX (dialog_vbox), hbox, FALSE, FALSE, 0);

    /* current status of request */
    if (!newpartition) {
	GtkWidget *label;

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), 
			    gtk_label_new (_("Allocation Status:")),
			    FALSE, FALSE, 2);

	if (entry->status != REQUEST_DENIED) {
	    label = gtk_label_new (_("Successful"));
	    set_label_color (label, 0x2200, 0x8d00, 0x2200);
	} else {
	    label = gtk_label_new (_("Failed"));
	    set_label_color (label, 0xffff, 0, 0); 
	}

	gtk_box_pack_start (GTK_BOX (hbox), label, 
			    FALSE, FALSE, 2);
	gtk_widget_show_all (hbox);
	gtk_box_pack_start (GTK_BOX (dialog_vbox), hbox, FALSE, FALSE, 0);

	if (entry->status == REQUEST_DENIED) {
	    hbox = gtk_hbox_new (FALSE, 0);
	    gtk_box_pack_start (GTK_BOX (hbox), 
				gtk_label_new (_("Failure Reason:")),
				FALSE, FALSE, 2);

	    label = gtk_label_new (GetReasonString(entry->reason));
	    set_label_color (label, 0xffff, 0, 0);
	    gtk_box_pack_start (GTK_BOX (hbox), label,FALSE, FALSE, 2);
	    gtk_widget_show_all (hbox);
	    gtk_box_pack_start (GTK_BOX (dialog_vbox), hbox, FALSE, FALSE, 0);
	}
    }

    /* partition type */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), 
			gtk_label_new (_("Partition Type:")),
			FALSE, FALSE, 2);

    type_omenu = gtk_option_menu_new ();
    menu = gtk_menu_new ();

    l = NONSUNPARTTYPE;
    for (hdidx = 0; hdidx < numhd; hdidx++) {
	if ((hdarr[hdidx]->num == p->drive.current &&
	    hdarr[hdidx]->part_type == FDISK_PART_TYPE_SUN)
#ifdef __sparc__
	    /* Hack */
	    || !p->drive.current
#endif	    
	    )
	    l = SUNPARTTYPE;
    }
    match_type = 0;
    if (p->immutable) {
	l |= p->type.current;
	for (j=0; j<nparttypes; j++) {
	    if (allparttypes[j].index == l || allparttypes[j].index == p->type.current)
		break;
	}
	if (j != nparttypes)
	    snprintf(tmpstr, sizeof(tmpstr), "%s", allparttypes[j].name);
	else
	    snprintf(tmpstr,sizeof(tmpstr),"%6s (0x%x)",
		     "Unknown", p->type.current);

	menu_item = gtk_menu_item_new_with_label (tmpstr);
	gtk_menu_append (GTK_MENU (menu), menu_item);
	gtk_widget_show (menu_item);
    } else {
	if (newpartition)
	    current_type = LINUX_NATIVE_PARTITION;
	else
	    current_type = p->type.current;
	if (l == SUNPARTTYPE)
	    fstypes = sunfstypes;
	else
	    fstypes = pcfstypes;
	for (j=0; fstypes[j] != -1; j++) {
	    for (k = 0; k < nparttypes; k++) {
		if (allparttypes[k].index == (l|fstypes[j]) || allparttypes[k].index == fstypes[j])
		    break;
	    }
	    if (k == nparttypes) continue;
	    snprintf(tmpstr,sizeof(tmpstr),"%s", allparttypes[k].name);
	    menu_item = gtk_menu_item_new_with_label (tmpstr);
	    gtk_menu_append (GTK_MENU (menu), menu_item);
	    if (fstypes[j] == p->type.current || 
		(p->type.current == 0 && fstypes[j]==LINUX_NATIVE_PARTITION)) {
		match_type = j;
	    }
	    
	    gtk_signal_connect (GTK_OBJECT (menu_item), "activate",
				GTK_SIGNAL_FUNC (type_menu_activate_cb), 
				&current_type);
	    gtk_object_set_data (GTK_OBJECT (menu_item), 
				 "partition_type",
				 GINT_TO_POINTER (fstypes[j]));
	    gtk_object_set_data (GTK_OBJECT (menu_item), 
				 "menu", menu);
	    gtk_widget_show (menu_item); 
	}
    }
    gtk_object_set_data (GTK_OBJECT (menu), "mntpt_entry", mntpt_entry);
    gtk_object_set_data (GTK_OBJECT (menu), "mntpt_combo", mntpt_combo);
    gtk_object_set_data (GTK_OBJECT (menu), "saved_pntpt", NULL);
    gtk_widget_show (menu);
    gtk_option_menu_set_menu (GTK_OPTION_MENU (type_omenu), menu); 
    gtk_option_menu_set_history (GTK_OPTION_MENU (type_omenu), match_type);

    gtk_box_pack_start (GTK_BOX (hbox), type_omenu, FALSE, FALSE, 2);
    gtk_widget_show_all (hbox);
    gtk_box_pack_start (GTK_BOX (dialog_vbox), hbox, FALSE, FALSE, 0);

    /* allowable drives */
    drives_clist = NULL;
    if (!p->immutable) {
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), 
			    gtk_label_new (_("Allowable Drives:")),
			    FALSE, FALSE, 2);
	
	scrollwin = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwin),
					GTK_POLICY_NEVER,
					GTK_POLICY_AUTOMATIC);

	drives_clist = gtk_clist_new (1);

	/* XXXX - yuck make absolute height for list! */
	gtk_widget_set_usize (drives_clist, -1, 70);

	gtk_clist_set_column_width (GTK_CLIST (drives_clist), 0, 50);
	gtk_clist_set_column_justification (GTK_CLIST (drives_clist), 0,
					    GTK_JUSTIFY_CENTER);
	gtk_clist_set_selection_mode (GTK_CLIST (drives_clist),
				      GTK_SELECTION_MULTIPLE);

	gtk_signal_connect (GTK_OBJECT (drives_clist), "button_press_event",
			GTK_SIGNAL_FUNC (stop_drives_clist_btnpress), NULL);

	for (j=0; j<numhd; j++) {
	    gchar *row[2];
	    gint  rowindex;

	    row[0] = hdarr[j]->name+5;
	    row[1] = NULL;

	    rowindex = gtk_clist_append (GTK_CLIST (drives_clist), row);
	    gtk_clist_set_row_data (GTK_CLIST (drives_clist), rowindex,
				    GINT_TO_POINTER (j));
	    if (fdiskThisDriveSetIsActive(&p->drive, hdarr[j]->num))
		gtk_clist_select_row (GTK_CLIST (drives_clist), rowindex, -1);
	    else
		gtk_clist_unselect_row (GTK_CLIST (drives_clist), rowindex,-1);
	}

	gtk_container_add (GTK_CONTAINER (scrollwin), drives_clist);
	gtk_box_pack_start (GTK_BOX (hbox), scrollwin, FALSE, FALSE, 2);

	gtk_widget_show_all (hbox);
	gtk_box_pack_start (GTK_BOX (dialog_vbox), hbox, FALSE, FALSE, 0);
    }

#if 0
    /* setup data on dialog so we can pull it out when user closes it */
    gtk_object_set_data (GTK_OBJECT (dialog), "mntpt_entry", mntpt_entry);
    gtk_object_set_data (GTK_OBJECT (dialog), "drives_clist", drives_clist);
    gtk_object_set_data (GTK_OBJECT (dialog), "size_entry", size_entry);
    gtk_object_set_data (GTK_OBJECT (dialog), "grow_checkbox", grow_checkbox);
    gtk_object_set_data (GTK_OBJECT (dialog), "type_omenu", type_omenu);
    
    gtk_signal_connect (GTK_OBJECT (dialog), "close",
			GTK_SIGNAL_FUNC (edit_dialog_close_cb), 
			NULL);
#endif



    /* Most of the sanity checking code here is shared with the
     * newt frontend as well. Need to pull out to common file
     * so we can avoid this duplication
     */

    done = 0;
    while (!done) {
    
	/* run dialog and wait for user to select button */
	gtk_widget_grab_focus (GTK_WIDGET (mntpt_entry));
	gnome_dialog_close_hides (GNOME_DIALOG (dialog), TRUE);
	ret = gnome_dialog_run (GNOME_DIALOG (dialog));

	/* did user kill dialog with window manager? */
	if (ret == -1) {
	    gtk_widget_destroy (dialog);
	    return FDISK_ERR_USERABORT;
	}

	/* user hit the 'Ok' button */
	if (ret == 0) {
	    mntpt =  gtk_entry_get_text (GTK_ENTRY (mntpt_entry));

	    /* modify partition request based on the entry boxes */
	    if (!p->immutable) {
		guint tmpuint;

		menu_item = gtk_menu_get_active (GTK_MENU (menu));
		tmpuint = GPOINTER_TO_INT (gtk_object_get_data (
		    GTK_OBJECT (menu_item), "partition_type"));
		fdiskSetConstraint(&p->type, tmpuint, tmpuint, tmpuint, 1);
	    }
	    
	    /* make sure mount point is valid */
	    if (p->type.current != LINUX_SWAP_PARTITION && 
		p->type.current != LINUX_RAID_PARTITION) {
		int valid=1;
		int skiprest=0;

		TrimWhitespace(mntpt);
		
		/* see if they even gave the partition a name  */
		/* we will ask them if they really want to not */
		/* assign the partition a name at this time if */
		/* they have just created a non-ext2 partition */
		if (!*mntpt && p->type.current != LINUX_NATIVE_PARTITION) {
		    if (ddruidYesNo(_("No Mount Point"), _("Yes"), _("No"),
				      _("You have not selected a mount point "
				        "for this partition. Are you sure you "
				        "want to do this?"), 0) != 1)
			continue;
		    else {
			/* we need a name for this partition    */
			/* we'll name them like swap partitions */
			/* except use 'DOSxxx'                  */
			if (strncmp("Dos", entry->name, 4)) {
			    char *t;
			    fdiskMakeUniqSpecName( spec, "Dos", &t );
			    fdiskRenamePartitionSpec(spec, entry->name, t);
			}
			skiprest = 1;
		    }
		}
		
		/* do old test first */
		if (!skiprest) {
		    if (entry->status != REQUEST_ORIGINAL || *mntpt)
			if (badMountPoint(p->type.current, mntpt))
			    continue;
		    
		    if (entry->status == REQUEST_ORIGINAL) {
			/* this is an original partition, should have a */
			/* mount point of "" or a valid path            */
			if (*mntpt && 
			    (*mntpt != '/' || ((strcmp(entry->name, mntpt) &&
			!fdiskIndexPartitionSpec(spec, mntpt, &tmpuint))))) {
			    valid = 0;
			}
		    } else if (*mntpt != '/' || (strcmp(entry->name, mntpt) &&
			 !fdiskIndexPartitionSpec(spec, mntpt, &tmpuint))) {
			valid = 0;
		    }
		}

		/* make sure this mount point isn't already in use    */
		/* XXXX referencing global context to get to raidtab! */
		if (findMntPtInRaidArray (globalContext.raidarray, mntpt))
		    valid = 0;

		if (!valid) {
		    ddruidShowMessage(_("Mount Point Error"), _("Ok"),
			   _("The mount point requested is either an illegal "
			     "path or is already in use. Please select a "
			     "valid mount point."));
		    
		    continue;
		}
	    }

	    if (size_entry) {
		size = gtk_entry_get_text (GTK_ENTRY (size_entry));

		tmpint=strtol(size, &eptr, 10);
		if (eptr != size && *eptr == 0 && tmpint > 0) {
		    tmpint *= SECPERMEG;
		    if (grow_checkbox)
			grow = gtk_toggle_button_get_active( 
			    GTK_TOGGLE_BUTTON (grow_checkbox));
		    else
			grow = FALSE;

		    if (grow)
			fdiskSetConstraint(&p->size,0,tmpint,FDISK_SIZE_MAX,1);
		    else
			fdiskSetConstraint(&p->size,0,tmpint,tmpint,1);
		} else {
		    ddruidShowMessage(_("Size Error"), _("Ok"),
			   _("The size requested is illegal. Make sure the "
			     "size is greater and zero (0), and is specified "
			     "int decimal (base 10) format."));
		    continue;
		}
	    }
	    
            /* The size limit is now 2 gb in kernels > 2.1.117 */
            /* For 2.2.1+ kernels the size differs among       */
            /* different architectures.                        */
	    /* make sure swap partitions are not too large     */
	    /* (PAGESIZE - 10)*8*PAGESIZE                      */
	    /* on the right arch's                             */
	    if (p->type.current == LINUX_SWAP_PARTITION) {
		unsigned long long maxswap = fdiskMaxSwap();

		if (p->size.min > maxswap/SECTORSIZE) {
		    ddruidShowMessage(_("Swap Size Error"), _("Ok"),
		       _("You have created a swap partition which is too "
			"large. The maximum size of a swap partition is "
			"%ld Megabytes."), (long)(maxswap / 1024 / 1024));
			continue;
		}
	    }

	    if (drives_clist) {
		GList *sel;

		fdiskDeactivateAllDriveSet( &p->drive );
		sel = GTK_CLIST (drives_clist)->selection;

		if (p->type.current == LINUX_RAID_PARTITION &&
		    g_list_length (sel) != 1) {
#if 0
		    if (ddruidYesNo(_("No RAID Drive Constraint"),
				    _("Yes"), _("No"),
				    _("You have configured a RAID partition "
				      "without constraining the partition to "
				      "a single drive.\n Are you sure you "
				      "want to do this?", 0)) == 0)
#else
		    ddruidShowMessage(_("No RAID Drive Constraint"),
				    _("Close"), 
				    _("You have configured a RAID partition "
				      "without constraining the partition to "
				      "a single drive. Please select one drive "
				      "to constrain this partition to."));
#endif
			continue;
		}

		while (sel) {
		    gint drv;

		    drv =  GPOINTER_TO_INT (gtk_clist_get_row_data( 
			GTK_CLIST (drives_clist),GPOINTER_TO_INT (sel->data)));

		    fdiskActivateDriveSet( &p->drive, hdarr[drv]->num );

		    sel = sel->next;
		}
	    }
	    
	    /* fdiskHandleSpecialPartitions() will do this for us */
	    /* so I'm taking the boot entry out for now           */

	    if (p->type.current == LINUX_SWAP_PARTITION) {
		/* make sure we have a valid swap partition name */
		if (strncmp("Swap", entry->name, 4)) {
		    char *t;
		    fdiskMakeSwapSpecName( spec, &t );
		    fdiskRenamePartitionSpec(spec, entry->name, t);
		    free(t);
		}
	    } else if (p->type.current == LINUX_RAID_PARTITION) {
		/* make sure we have a valid raid partition name */
		if (strncmp("Raid", entry->name, 4)) {
		    char *t;
		    fdiskMakeRaidSpecName( spec, &t );
		    fdiskRenamePartitionSpec(spec, entry->name, t);
		    free(t);
		}
	    }
	    
	    /* first see if they changed the mount point    */
	    /* we only worry about ext2 and dos partitions  */
	    /* which have a valid mntpt                     */
	    /* LOGIC is not the word for how all this works */
	    if (p->type.current != LINUX_SWAP_PARTITION &&
		p->type.current != LINUX_RAID_PARTITION &&
		strncmp("Dos", mntpt, 3)) {
		TrimWhitespace(mntpt);
		if (p->immutable)
		    status = REQUEST_ORIGINAL;
		else
		    status = REQUEST_PENDING;
	    
		if (strcmp(mntpt, entry->name)) {
		    /* if this is an original partition which we just set  */
		    /* the name back to '' from a real name, set name back */
		    /* to the 'Existxxxxx' name                            */
		    if (entry->status == REQUEST_ORIGINAL && !*mntpt) {
			for (hdidx=0; hdidx < numhd; hdidx++) 
			    if (hdarr[hdidx]->num == p->drive.current)
				break;
			
			if (hdidx != numhd)
			    sprintf(tmpstr, "Exist%03d%03d",
				    hdarr[hdidx]->num, p->num.current);
			else
			    strcpy(tmpstr,"Exist999999");
			
			fdiskRenamePartitionSpec( spec, entry->name, tmpstr );
			fdiskModifyPartitionSpec( spec, tmpstr, p, status);
		    } else {
			fdiskRenamePartitionSpec( spec, entry->name, mntpt );

			/*  this is a big kludge! */
			/* reset bootable partition handling so if we   */
			/* rename '/' to '/usr', we don't enforce rules */
			fdiskSetConstraint(&p->endcyl,
			       0,FDISK_ENDCYL_MIN,FDISK_ENDCYL_MAX,0);
			fdiskModifyPartitionSpec( spec, mntpt, p, status);
		    }
		} else {
		    fdiskModifyPartitionSpec( spec, mntpt, p, status);
		}	    
	    } else {
		fdiskModifyPartitionSpec( spec, entry->name, p, status);
	    }

	    /* cheat here - look at raid table first and see if there is */
	    /* a /boot partition which is RAID-1. If so make it bootable */
	    /* otherwise go through regular partitions and try to find   */
	    /* a valid candidate for being the bootable partition        */

	    gnomefseditHandleSpecialPartitions (hdarr, numhd, spec, 
						globalContext.raidarray);

	    status = FDISK_SUCCESS;
	    done = 1;
	} else {
	    status = FDISK_ERR_USERABORT;
	    done = 1;
	}
    }

    gtk_widget_destroy (dialog);

    return status;

}

/* add a partition spec */
static int AddPartitionSpec(HardDrive **hdarr, unsigned int numhd,
		     PartitionSpec *spec) {

    Partition template;
    int      status;
    unsigned int i;
    
    /* create a template partitionspec to send to editpartition */
    memset(&template, 0, sizeof(Partition));
    template.size.min = SECPERMEG;

    /* insert with a name we know to mean its a new partition */
    fdiskInsertPartitionSpec(spec, NEW_PARTSPEC, &template, REQUEST_PENDING);
    fdiskIndexPartitionSpec( spec, NEW_PARTSPEC, &i );
    status = EditPartitionSpec(hdarr, numhd, spec, &spec->entry[i]);
    if (status == FDISK_SUCCESS) {
	return FDISK_SUCCESS;
    } else {
	fdiskDeletePartitionSpec(spec, NEW_PARTSPEC);
	return FDISK_ERR_USERABORT;
    }
}

/* returns TRUE if everything ok, FALSE if there is a problem */
gboolean
checkRaidSpecEntryIntegrity (RaidSpecEntry *entry, HardDriveContext *context)
{
    GList *p;
    PartitionSpec *spec;

    if (!entry || !context) {
	g_warning ("Bad data in checkRaidSpecEntryIntegrity\n");
	return TRUE; /* XXXX bad but what else can we do? */
    }

    spec = &context->state->spec;
    p = entry->partitions;
    while (p) {
	gint rc, i;

	rc = fdiskIndexPartitionSpec (spec, p->data, &i);
	if (rc) {
	    g_warning ("Couldnt find partition named %s\n",(char *)p->data);
	} else {
	    if (spec->entry[i].status == REQUEST_DENIED) {
		/* found a problem, return failure code */
		return FALSE;
	    } 
	}
	    
	p = p->next;
    }
    return TRUE;
}

/* do simple check to make sure all raid entries are still complete */
void
checkRaidArrayIntegrity (HardDriveContext *context)
{
    GList *s;

    if (!context) {
	g_warning ("Bad data in checkRaidArrayIntegrity\n");
	return;
    }

    if (!context->raidarray)
	return;

    s = context->raidarray;
    while (s) {
	RaidSpecEntry *entry = (RaidSpecEntry *)s->data;
	if (!checkRaidSpecEntryIntegrity (entry, context)) {
	    /* XXXXX - for now destroy the raid entry since it */
	    /*         now contains unallocated partitions!    */
	    ddruidShowMessage (_("RAID Entry Incomplete"), _("Ok"),
			       _("The raid device /dev/%s now contains "
				 "partitions which are unallocated. "
				 "The raid device /dev/%s will now be decomposed "
				 "into its component partitions. Please "
				 "recompose the raid device with allocated "
				 "partitions."), entry->device, entry->device);
	    s = s->next;
	    deleteRaidSpecEntry (&context->raidarray, entry);
	} else {
	    s = s->next;
	}
    }
}

/* do a operation on a partition */
static int
DoMenuFunction(char *function, HardDriveContext *context,
	       unsigned int num)
{
    HardDrive **orighd, **curhd;
    unsigned int numhd;
    PartitionSpec *spec;
    
    int          status;
    int          i;
    HardDrive    *tmphdarr[MAX_HARDDRIVES];

    orighd = context->state->hdarr;
    numhd  = context->state->numhd;
    curhd  = context->state->newhdarr;
    spec   = &context->state->spec;

    for (i=0; i<numhd; i++) {
	tmphdarr[i] = (HardDrive *) malloc(sizeof(HardDrive));
	memcpy(tmphdarr[i], orighd[i], sizeof(HardDrive));
    }

    if (!strcmp("ADD", function)) {
	status = AddPartitionSpec(tmphdarr, numhd, spec);
    } else if (num >= 0 && !strcmp("EDIT", function)) {
	status = EditPartitionSpec(tmphdarr, numhd, spec, &spec->entry[num]);
    } else if (num >= 0 && !strcmp("DEL", function)) {
	status = DeletePartitionSpec(orighd, numhd, spec, &spec->entry[num],0);
    } else {
	status = FDISK_ERR_BADNUM;
    }

    if (status == FDISK_SUCCESS) {
	fdiskAutoInsertPartitions(orighd, numhd, tmphdarr, spec );
	showReasons( spec );
	fdiskGrowPartitions(orighd, numhd, tmphdarr, spec);

	/* if any original partitions were REMOVED we have to */
	/* sync up their entries in the partition spec table  */
	/* with their actual representation in 'orighd'.      */
	/* Mainly fixes up logical partition #'s which change */
	/* when other logical partitions are removed          */
	if (!strcmp("DEL", function))
	    fdiskCleanOriginalSpecs( orighd, numhd, spec );
	
	for (i=0; i<numhd; i++)
	    memcpy(curhd[i],tmphdarr[i], sizeof(HardDrive));

	/* check to see if we broke any raid entries by causing     */
	/* a component partition of the entry to become unallocated */
	checkRaidArrayIntegrity (context);

	/* debugging */
#ifdef GNOME_FSEDIT_DEBUG_OUTPUT
	printf ("Operation complete, context is now:\n");
	print_context( context);
#endif
			    
	return FDISK_SUCCESS;
    } else {
	return FDISK_ERR_BADNUM;
    }
	
}

/* GUI stuff */



/* give summary of why partitions weren't allocated */
void showReasons( PartitionSpec *spec ) {
    GtkWidget *dialog, *hack_widget;
    int i;

    /* were there any unallocated partitions? */
    for (i=0; i<spec->num; i++)
	if (spec->entry[i].status == REQUEST_DENIED)
	    break;

    /* nothing going on here, keep moving along */
    if (i == spec->num)
	return;

#if 0
    dialog = gnome_dialog_new (_("Unallocated Partitions"), _("Ok"), NULL);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

    hack_widget = gtk_label_new (
	_("There are currently unallocated partition(s) "
	  "present in the list of requested partitions. The "
	  "unallocated partition(s) are shown below, along with "
	  "the reason they were not allocated."));
    gtk_label_set_line_wrap (GTK_LABEL (hack_widget), TRUE);
    gtk_widget_show (hack_widget);
    gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), hack_widget,
			FALSE, FALSE, 0);
#else
    dialog = gnome_message_box_new (
	_("There are currently unallocated partition(s) "
	  "present in the list of requested partitions. The "
	  "unallocated partition(s) are shown below, along with "
	  "the reason they were not allocated."), 
	GNOME_MESSAGE_BOX_ERROR, _("Ok"), NULL);
    gtk_window_set_title (GTK_WINDOW (dialog), _("Unallocated Partitions"));
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

    /* Kids, don't try this at home */
    /* this is pretty evil... <-:  */
    hack_widget = GNOME_DIALOG (dialog)->vbox;
    hack_widget = ((GtkBoxChild *) GTK_BOX (hack_widget)->children->data)->widget;
    hack_widget = ((GtkBoxChild *) GTK_BOX (hack_widget)->children->next->data)->widget;
    gtk_label_set_line_wrap (GTK_LABEL (hack_widget), TRUE);
#endif

    for (i=0; i<spec->num; i++)
	if (spec->entry[i].status == REQUEST_DENIED) {
	    char tmpstr[80];
	    char *pname = spec->entry[i].name;
	    char *s, *t;
	    GtkWidget *label;
	    
	    memset(tmpstr, ' ', 80);
	    if (strncmp("Exist", pname, 5) && strncmp("Swap", pname, 4) &&
		strncmp("Dos", pname, 3) && strncmp("Raid", pname, 4))
		t = pname;
	    else if (!strncmp("Swap", pname, 4))
		t = "<Swap Partition>";
	    else if (!strncmp("Raid", pname, 4))
		t = "<RAID Partition>";
	    else
		t = NULL;
	    for (s=tmpstr;t && *t; t++,s++)
		*s = *t;
	    
	    t = GetReasonString(spec->entry[i].reason);
	    for (s=tmpstr+20;t && *t; t++,s++)
		*s = *t;
	    *s = '\0';

	    label = gtk_label_new (tmpstr);
	    gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox),
				label, FALSE, FALSE, 2);
	    gtk_widget_show (label);
	}

    gnome_dialog_run_and_close (GNOME_DIALOG (dialog));

}


void
ddruidInitUI () {
    int argc=1;
    char *argv[] = {"gnomefsedit"};

    /* fake args for now */
    gnome_init ("gnomefsedit", "0.0", argc, argv);
}

void
ddruidShowMessage(char *title, char *button, char *text, ...)
{
    GtkWidget *dialog;
    GtkWidget *hack_widget;
    gchar *realtext;

    va_list args;

    va_start (args, text);
    realtext = g_strdup_vprintf (text, args);
    va_end (args);

    dialog = gnome_message_box_new (realtext, GNOME_MESSAGE_BOX_ERROR,
				    button, NULL);
    g_free (realtext);

/*      if (mainwin) */
/*  	gnome_dialog_set_parent (GNOME_DIALOG (dialog), GTK_WINDOW (mainwin)); */

    gtk_window_set_title (GTK_WINDOW (dialog), title);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

    /* Kids, don't try this at home */
    /* this is pretty evil... <-:  */
    hack_widget = GNOME_DIALOG (dialog)->vbox;
    hack_widget = ((GtkBoxChild *) GTK_BOX (hack_widget)->children->data)->widget;
    hack_widget = ((GtkBoxChild *) GTK_BOX (hack_widget)->children->next->data)->widget;
    gtk_label_set_line_wrap (GTK_LABEL (hack_widget), TRUE);
    
    gnome_dialog_run_and_close (GNOME_DIALOG (dialog));

}

/* default = 0 picks no, default = 1 picks yes */
int
ddruidYesNo(char *title, char *yes, char *no, char *text, int def)
{
    GtkWidget *dialog;
    GtkWidget *hack_widget;
    gint ret;

    dialog = gnome_message_box_new (text, GNOME_MESSAGE_BOX_QUESTION,
				    yes, no, NULL);
/*      if (mainwin) */
/*  	gnome_dialog_set_parent (GNOME_DIALOG (dialog), GTK_WINDOW (mainwin)); */

    gnome_dialog_set_default (GNOME_DIALOG (dialog), (def) ? 0 : 1);

    gtk_window_set_title (GTK_WINDOW (dialog), title);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

    /* Kids, don't try this at home */
    /* this is pretty evil... <-:  */
    hack_widget = GNOME_DIALOG (dialog)->vbox;
    hack_widget = ((GtkBoxChild *) GTK_BOX (hack_widget)->children->data)->widget;
    hack_widget = ((GtkBoxChild *) GTK_BOX (hack_widget)->children->next->data)->widget;
    gtk_label_set_line_wrap (GTK_LABEL (hack_widget), TRUE);

    ret = gnome_dialog_run_and_close (GNOME_DIALOG (dialog));

    if (ret == -1)
	return -1;
    else
	return (ret == 0);
}


int
ddruidTernary(char *title, char *yes, char *no, char *cancel, char *text)
{
    GtkWidget *dialog;
    GtkWidget *hack_widget;
    gint ret;

    dialog = gnome_message_box_new (text, GNOME_MESSAGE_BOX_QUESTION,
				    yes, no, cancel, NULL);
/*      if (mainwin) */
/*  	gnome_dialog_set_parent (GNOME_DIALOG (dialog), GTK_WINDOW (mainwin)); */

    gtk_window_set_title (GTK_WINDOW (dialog), title);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

    /* Kids, don't try this at home */
    /* this is pretty evil... <-:  */
    hack_widget = GNOME_DIALOG (dialog)->vbox;
    hack_widget = ((GtkBoxChild *) GTK_BOX (hack_widget)->children->data)->widget;
    hack_widget = ((GtkBoxChild *) GTK_BOX (hack_widget)->children->next->data)->widget;
    gtk_label_set_line_wrap (GTK_LABEL (hack_widget), TRUE);

    ret = gnome_dialog_run_and_close (GNOME_DIALOG (dialog));

    return ret;
}


/* local functions */

static void
set_label_color (GtkWidget *label, guint red, guint green, guint blue)
{
	GtkRcStyle *rcstyle;

	rcstyle = gtk_rc_style_new ();
	rcstyle->color_flags[GTK_STATE_NORMAL] = GTK_RC_FG;
	rcstyle->fg[GTK_STATE_NORMAL].red = red;
	rcstyle->fg[GTK_STATE_NORMAL].green = green;
	rcstyle->fg[GTK_STATE_NORMAL].blue = blue;

	gtk_widget_modify_style (label, rcstyle);

/* XXXXX - temp hack - remove when gtk_rc_styles fixed by owen ! */
	gtk_rc_style_ref (rcstyle);
}


static void
add_button_cb (GtkButton *button, HardDriveContext *context)
{
    gint status;

    status=DoMenuFunction("ADD", context, -1);

    if (status == FDISK_SUCCESS) {
	update_gui (context);
	context->changesmade = TRUE;
    }
}

static gboolean
PartitionSpecHasRaid (PartitionSpec *spec, GList *raidarray)
{
    gint i, nraid;
    Partition *p;

    /* see if any raid partitions are available */
    for (i=0, nraid=0; i<spec->num; i++) {
	if (spec->entry[i].status == REQUEST_DENIED)
	    continue;

	p = &(spec->entry[i].partition);
	if (p->type.current == LINUX_RAID_PARTITION && 
	    !specInRaidArray (raidarray, spec, i))
	    nraid++;
    }

    return (nraid > 1);
}

/* create edit dialog to configure a raid mount point from existing */
/* RAID type partitions                                             */
static gint
editraid_dialog_run (HardDriveContext *context, RaidSpecEntry *raidentry)
{
    GtkWidget *dialog;
    GtkWidget *dialog_vbox;

    GtkWidget *hbox;
    GtkWidget *mntpt_entry, *mntpt_combo;
    GtkWidget *raiddev_combo;
    GtkWidget *raidtype_omenu;
    GtkWidget *part_clist, *scrollwin;
    GtkWidget *type_omenu, *menu, *menu_item, *partmenu;

    GList     *raiddev_list, *mntpt_strings;

    gint      current_type, current_parttype, done, ret, j, status, matchtype;
    guint     raidtype, parttype;
    
    char *mntpt, *raiddev, tmpstr[80];

    RaidSpecEntry  *raidspec;

    GList *sel;
    char  fstypesnames[][20] = { "Linux Swap", "Linux Native" };
    int   fstypes[] = {0x82, 0x83 };
    int   numfstypes   = sizeof(fstypes)/sizeof(int);
    int   i;

    /* identify potential raid partitions and present user choices */
    dialog = gnome_dialog_new ("Add RAID Configuration", 
			       _("Ok"), _("Cancel"),NULL);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

    gnome_dialog_set_default (GNOME_DIALOG (dialog), 0);

    dialog_vbox = GNOME_DIALOG (dialog)->vbox;

    /* mount point */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), gtk_label_new (_("Mount Point:")),
			FALSE, FALSE, 2);

    mntpt_combo = gtk_combo_new ();
    mntpt_strings = get_default_mntpt_strings();
    gtk_combo_set_popdown_strings (GTK_COMBO (mntpt_combo), mntpt_strings);
    g_list_free (mntpt_strings);

    mntpt_entry = GTK_COMBO (mntpt_combo)->entry;
    gtk_entry_set_max_length (GTK_ENTRY (mntpt_entry), MNTPT_MAXLEN);
    gtk_box_pack_start (GTK_BOX (hbox), mntpt_combo, FALSE, FALSE, 2);

    gnome_dialog_editable_enters (GNOME_DIALOG (dialog), 
				  GTK_EDITABLE (mntpt_entry));
    gtk_signal_connect (GTK_OBJECT (mntpt_entry), "insert_text", 
			GTK_SIGNAL_FUNC (mntpt_entry_insert_text_cb),
			NULL);

    if (raidentry) {
	if (raidentry->parttype == LINUX_SWAP_PARTITION)
	    gtk_entry_set_text (GTK_ENTRY (mntpt_entry), _("<Swap Partition"));
	else
	    gtk_entry_set_text (GTK_ENTRY (mntpt_entry), raidentry->mntpt);
    } else {
	    gtk_entry_set_text (GTK_ENTRY (mntpt_entry), "");
    }

    gtk_widget_show_all (hbox);
    gtk_box_pack_start (GTK_BOX (dialog_vbox), hbox, FALSE, FALSE, 0);


    /* partition type */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), 
			gtk_label_new (_("Partition Type:")),
			FALSE, FALSE, 2);

    type_omenu = gtk_option_menu_new ();
    partmenu = gtk_menu_new ();

    matchtype = 0;
    if (!raidentry)
	current_parttype = LINUX_NATIVE_PARTITION;
    else
	current_parttype = raidentry->parttype;

    for (j=0; j<numfstypes; j++) {
	snprintf(tmpstr,sizeof(tmpstr),"%s", fstypesnames[j]);
	menu_item = gtk_menu_item_new_with_label (tmpstr);
	gtk_menu_append (GTK_MENU (partmenu), menu_item);
	if (fstypes[j] == current_parttype)
	    matchtype = j;
	
	gtk_signal_connect (GTK_OBJECT (menu_item), "activate",
			    GTK_SIGNAL_FUNC (type_menu_activate_cb), 
			    &current_parttype);
	gtk_object_set_data (GTK_OBJECT (menu_item), 
			     "partition_type",
			     GINT_TO_POINTER (fstypes[j]));
	gtk_object_set_data (GTK_OBJECT (menu_item), 
			     "menu", partmenu);
	gtk_widget_show (menu_item); 
    }
    
    gtk_object_set_data (GTK_OBJECT (partmenu), "mntpt_entry", mntpt_entry);
    gtk_object_set_data (GTK_OBJECT (partmenu), "saved_pntpt", NULL);
    gtk_widget_show (partmenu);
    gtk_option_menu_set_menu (GTK_OPTION_MENU (type_omenu), partmenu); 
    gtk_option_menu_set_history (GTK_OPTION_MENU (type_omenu), matchtype);

    gtk_box_pack_start (GTK_BOX (hbox), type_omenu, FALSE, FALSE, 2);
    gtk_widget_show_all (hbox);
    gtk_box_pack_start (GTK_BOX (dialog_vbox), hbox, FALSE, FALSE, 0);

    /* md device */

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), 
			gtk_label_new (_("RAID Device: /dev/")),
			FALSE, FALSE, 2);
    raiddev_combo = gtk_combo_new ();

    /* prime popdown with default choices */
    raiddev_list = NULL;
    for (j=0; j<numraiddevs; j++)
	raiddev_list = g_list_append (raiddev_list, raiddevs[j]);

    gtk_combo_set_popdown_strings( GTK_COMBO(raiddev_combo), raiddev_list) ;

    /* set to current value */
    gtk_entry_set_text (GTK_ENTRY ( GTK_COMBO (raiddev_combo)->entry), 
			(raidentry && raidentry->device) ? raidentry->device : "");
    gnome_dialog_editable_enters (GNOME_DIALOG (dialog), 
				  GTK_EDITABLE ( GTK_COMBO (raiddev_combo)->entry));
    gtk_box_pack_start (GTK_BOX (hbox), raiddev_combo, FALSE, FALSE, 2);
    gtk_widget_show_all (hbox);
    gtk_box_pack_start (GTK_BOX (dialog_vbox), hbox, FALSE, FALSE, 0);

    /* Raid type */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), 
			gtk_label_new (_("RAID Type:")),
			FALSE, FALSE, 2);

    raidtype_omenu = gtk_option_menu_new ();
    menu = gtk_menu_new ();

    current_type = -1;
    matchtype = 0;
    for (j=0; j<numraidtypes; j++) {
	snprintf(tmpstr,sizeof(tmpstr),"%s", raidtypesnames[raidtypes[j]]);
	menu_item = gtk_menu_item_new_with_label (tmpstr);
	gtk_menu_append (GTK_MENU (menu), menu_item);

	if (raidentry && raidtypes[j] == raidentry->type)
	    matchtype = j;

	gtk_signal_connect (GTK_OBJECT (menu_item), "activate",
			    GTK_SIGNAL_FUNC (raidtype_menu_activate_cb), 
			    &current_type);
	gtk_object_set_data (GTK_OBJECT (menu_item), 
			     "raid_type",
			     GINT_TO_POINTER (raidtypes[j]));
	gtk_widget_show (menu_item); 
    }

    gtk_widget_show (menu);
    gtk_option_menu_set_menu (GTK_OPTION_MENU (raidtype_omenu), menu); 
    gtk_option_menu_set_history (GTK_OPTION_MENU (raidtype_omenu), matchtype);

    gtk_box_pack_start (GTK_BOX (hbox), raidtype_omenu, FALSE, FALSE, 2);
    gtk_widget_show_all (hbox);
    gtk_box_pack_start (GTK_BOX (dialog_vbox), hbox, FALSE, FALSE, 0);

    /* devices to use for raid array */
    /* allowable drives */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), 
			gtk_label_new (_("Partitions For RAID Array:")),
			FALSE, FALSE, 2);
    
    scrollwin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwin),
				    GTK_POLICY_NEVER,
				    GTK_POLICY_AUTOMATIC);

    part_clist = gtk_clist_new (2);

    /* XXXX - yuck make absolute height for list! */
    gtk_widget_set_usize (part_clist, -1, 70);

    gtk_clist_set_column_width (GTK_CLIST (part_clist), 0, 50);
    gtk_clist_set_column_width (GTK_CLIST (part_clist), 1, 50);
    gtk_clist_set_column_justification (GTK_CLIST (part_clist), 0,
					GTK_JUSTIFY_LEFT);
    gtk_clist_set_column_justification (GTK_CLIST (part_clist), 1,
					GTK_JUSTIFY_RIGHT);
    gtk_clist_set_selection_mode (GTK_CLIST (part_clist),
				  GTK_SELECTION_MULTIPLE);
    
/*    gtk_signal_connect (GTK_OBJECT (drives_clist), "button_press_event",
			GTK_SIGNAL_FUNC (stop_drives_clist_btnpress), NULL);
*/    
    for (j=0; j<context->state->spec.num; j++) {
	gchar *row[3];
	gint  rowindex;
	Partition    *p;
	
	if (context->state->spec.entry[j].status == REQUEST_DENIED)
	    continue;

	if (specInRaidArray (context->raidarray, &context->state->spec, j) &&
	    !(raidentry && specInRaidSpecEntry (raidentry, &context->state->spec, j)))
	    continue;

	p = &(context->state->spec.entry[j].partition);
	if (p->type.current != LINUX_RAID_PARTITION)
	    continue;

	row[0] = deviceFromSpecEntry (context, j);
	row[1] = g_strdup_printf ("%6dM", p->size.current/SECPERMEG);

	row[2] = NULL;
	
	rowindex = gtk_clist_append (GTK_CLIST (part_clist), row);
	gtk_clist_set_row_data (GTK_CLIST (part_clist), rowindex,
				GINT_TO_POINTER (j));
	if (raidentry && specInRaidSpecEntry (raidentry, &context->state->spec, j))
	    gtk_clist_select_row (GTK_CLIST (part_clist), rowindex, -1);
	
	g_free (row[0]);
	g_free (row[1]);
    }

    gtk_container_add (GTK_CONTAINER (scrollwin), part_clist);
    gtk_box_pack_start (GTK_BOX (hbox), scrollwin, FALSE, FALSE, 2);

    gtk_widget_show_all (hbox);
    gtk_box_pack_start (GTK_BOX (dialog_vbox), hbox, FALSE, FALSE, 0);

    done = 0;
    status = 0;
    while (!done) {
	/* run dialog and wait for user to select button */
	gnome_dialog_close_hides (GNOME_DIALOG (dialog), TRUE);
	ret = gnome_dialog_run (GNOME_DIALOG (dialog));
	
	/* did user kill dialog with window manager? */
	if (ret == -1) {
	    gtk_widget_destroy (dialog);
	    return FDISK_ERR_USERABORT;
	}
	
	/* user hit the 'Ok' button */
	if (ret == 0) {
	    /* read out dialog */
	    mntpt = gtk_entry_get_text (GTK_ENTRY (mntpt_entry));

	    raiddev = gtk_entry_get_text (GTK_ENTRY ( 
		GTK_COMBO(raiddev_combo)->entry));

	    menu_item = gtk_menu_get_active (GTK_MENU (partmenu));
	    parttype = GPOINTER_TO_INT (gtk_object_get_data (
		GTK_OBJECT (menu_item), "partition_type"));

	    if (!*mntpt) {
		ddruidShowMessage (_("No Mount Point"), _("Ok"), 
				  _("You have not selected a mount point. "
				    "A mount point is required."));
		continue;
	    }

	    /* make sure mntpt is not already in use */
	    if (!fdiskIndexPartitionSpec (&context->state->spec, mntpt, &i)) {
		ddruidShowMessage(_("Mount Point Error"), _("Ok"),
				  _("The mount point requested is "
				    "already in use. Please select a "
				    "valid mount point."));
		
		continue;
	    }

	    /* XXXXX - need to do sanity checking if '/' or '/boot' is
             *         mount point - currently we do nothing and could
             *         end up with an unbootable machine!
             */
#if 0
	    if (!strcmp (mntpt, "/") || !strcmp (mntpt, "/boot"))
		ddruidShowMessage (_("Booting From RAID Warning"), _("Ok"),
				   _("You have made this raid device "
				     "mount as a booting partition.  Please "
				     "make sure all the component partitions "
				     "are bootable."));
#endif
	    
	    if (parttype != LINUX_SWAP_PARTITION && badMountPoint(parttype, mntpt))
		continue;

	    if (!*raiddev) {
		ddruidShowMessage(_("No RAID Device"), _("Ok"), 
				  _("You need to selected a RAID device."));
		continue;
	    }

	    if (!(raidentry && !strcmp (raidentry->device, raiddev)) &&
		findRaidDevice (context->raidarray, raiddev)) {
		ddruidShowMessage(_("Used Raid Device"), _("Ok"), 
				  _("The raid device \"/dev/%s\" is already "
				    "configured as a raid device. Please "
				    "select another."), raiddev);
		continue;
	    }

	    /* see what partitions they want to use */
	    menu_item = gtk_menu_get_active (GTK_MENU (menu));
	    raidtype = GPOINTER_TO_INT (gtk_object_get_data (
		GTK_OBJECT (menu_item), "raid_type"));
	    sel = GTK_CLIST (part_clist)->selection;

	    if (!isRaidEntryComplete (raidtype, sel)) {
		ddruidShowMessage(_("Not Enough Partitions"),
				_("Ok"),
				_("You have not configured enough partitions "
				  "for the RAID type you have selected."));
		continue;
	    }

	    /* check that /boot is on a RAID-1 type */
	    if (!strcmp (mntpt, "/boot") && raidtype != RAID1) {
		ddruidShowMessage(_("Illegal /boot RAID Type"),
				_("Ok"),
				_("Boot partitions (/boot) are only allowed "
				  "on RAID-1."));
		continue;
	    } 

#ifdef __alpha__
	    if (!strcmp (mntpt, "/")) {
		ddruidShowMessage(_("Illegal RAID mountpoint"),
				_("Ok"),
				_("RAID partitions cannot be mounted as root (/) "
                                  "on Alpha."));
		continue;
	    }
#endif

	    /* passed all tests but swap test, create RaidSpecEntry and test */
	    raidspec = g_new0( RaidSpecEntry, 1);
	    raidspec->mntpt  = g_strdup (mntpt);
	    raidspec->device = g_strdup (raiddev);
	    raidspec->type   = raidtype;
	    raidspec->parttype = parttype;
	    raidspec->partitions = NULL;

	    while (sel) {
		gint snum;
		gchar *pname;

		snum = GPOINTER_TO_INT (gtk_clist_get_row_data (GTK_CLIST (part_clist), 
					       GPOINTER_TO_INT (sel->data)));
		pname =  context->state->spec.entry[snum].name;
		raidspec->partitions = g_list_append (raidspec->partitions,
						      g_strdup(pname));
/*		g_print ("Added RAID partition %s\n", pname); */
		sel = sel->next;
	    }


            /* The size limit is now 2 gb in kernels > 2.1.117 */
            /* For 2.2.1+ kernels the size differs among       */
            /* different architectures.                        */
	    /* make sure swap partitions are not too large     */
	    /* (PAGESIZE - 10)*8*PAGESIZE                      */
	    /* on the right arch's                             */
	    if (parttype == LINUX_SWAP_PARTITION) {
		unsigned long long maxswap = fdiskMaxSwap();

		if (sizeRaidSpecEntry (raidspec,context) > maxswap/SECTORSIZE) {
		    ddruidShowMessage(_("Swap Size Error"), _("Ok"),
		       _("You have created a swap partition which is too "
			"large. The maximum size of a swap partition is "
			"%ld Megabytes."), (long)(maxswap / 1024 / 1024));
		    
		    freeRaidSpecEntry (raidspec);
		    raidspec=NULL;
		    continue;
		}
	    }


	    /* if this is a /boot partition go through and set */
	    /* cylinder constraints so it will be bootable     */
	    if (!strcmp (raidspec->mntpt, "/boot")) {
		GList *s = raidspec->partitions;
		Partition *p;

		/* first see if any immutable partitions are in the set */
		while (s) {
		    int rc, imm;

		    rc = fdiskReturnPartitionSpec(&context->state->spec, 
						  s->data, &p );
		    if (rc) {
			g_print ("Error finding partition %s\n", 
				 (char *)s->data);
			break;
		    }
		    
		    /* ask user if we really should use preexisting */
		    /* partitions for /boot  */
		    imm = p->immutable;
		    free (p);

		    if (imm) {
			gchar *txt, *device;
			int   rc;
			int   i;

			fdiskIndexPartitionSpec (&context->state->spec, s->data,
						&i);

			device = deviceFromSpecEntry (context, i);
			txt = g_strdup_printf (
			    _("The partition %s is a pre-existing partition in the "
			       "set of partitions for this RAID device.  The "
                               "mount point is set to /boot. Are you sure "
                               "that it is possible to boot from this "
			       "partition?"), device);
			
			g_free (device);
			rc = ddruidYesNo (_("Use Pre-existing Partition?"), 
					 _("Yes"), _("No"), txt, 0);

			g_free (txt);

			if (rc != 1)
			    break;
		    }
		    s = s->next;
		}

		if (s != NULL) {
		    freeRaidSpecEntry (raidspec);
		    raidspec=NULL;
		    continue;
		}

		/* ok now we're clear to set all immutable partitions      */
		/* to be under 1024 cylinders.  If there were immutable    */
		/* partitions then the user has told us its ok to use them */
		while (s) {
		    int rc;
		    
		    rc = fdiskReturnPartitionSpec(&context->state->spec,
						  s->data, &p );
		    if (rc) {
			g_print ("Error finding partition %s\n",
				 (char *)s->data);
			break;
		    }
		    
		    /* only set if this is not a preexisting partition */
		    if (!p->immutable) {
			fdiskSetConstraint(&p->endcyl, 0, 0, 1023, 1);
			fdiskModifyPartitionSpec (&context->state->spec, 
						  s->data, p, 
						  REQUEST_PENDING );
		    }

		    free (p);
		    
		    s = s->next;
		}
	    }

	    /* insert into array and we're done */
	    if (raidentry && context->raidarray) {
		GList *p;

		p = g_list_find (context->raidarray, raidentry);
		freeRaidSpecEntry (p->data);
		p->data = raidspec;
	    } else {
		context->raidarray = g_list_append (context->raidarray, raidspec);
	    }

	    done = 1;
	    status = 0;

	} else {
	    status = FDISK_ERR_USERABORT;
	    done = 1;
	}
	
	gtk_widget_destroy (dialog);
    }	

    return status;
}

static void
addraid_button_cb (GtkButton *button, HardDriveContext *context)
{
    gint status;

    if (!PartitionSpecHasRaid (&context->state->spec, context->raidarray)) {
	g_warning ("Got into addraid_button_cb w/o any raid partitions!\n");
	return;
    }

    status = editraid_dialog_run (context, NULL);
    if (status == FDISK_SUCCESS) {
	update_gui (context);
	context->changesmade = TRUE;
    }
    
}

static void
auto_partition_rbutton_cb (GtkButton *button, gpointer data)
{
    gint option;
    gint *choice = data;

    option = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (button), "Option"));

    *choice = option;
}

static void
auto_button_cb (GtkButton *button, HardDriveContext *context)
{
    gint status;
    gint flags, server;
    gint rc;

    GtkWidget *dialog, *dialog_vbox, *frame;
    GtkWidget *vbox, *rbutton;

    dialog = gnome_dialog_new (_("Auto-Partition"), _("Ok"), _("Cancel"),NULL);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

    gnome_dialog_set_default (GNOME_DIALOG (dialog), 0);

    dialog_vbox = GNOME_DIALOG (dialog)->vbox;

    frame = gtk_frame_new (_("Using Existing Disk Space"));
    gtk_box_pack_start (GTK_BOX (dialog_vbox), frame, TRUE, TRUE, 0);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (frame), vbox);

    flags = FSEDIT_CLEARALL;
    rbutton = gtk_radio_button_new_with_label (NULL, "Remove all data");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rbutton), TRUE);
    gtk_object_set_data (GTK_OBJECT (rbutton), "Option", 
			 GINT_TO_POINTER (FSEDIT_CLEARALL));
    gtk_signal_connect (GTK_OBJECT (rbutton), "toggled",
				 auto_partition_rbutton_cb,
				 &flags);
    gtk_box_pack_start (GTK_BOX (vbox), rbutton, TRUE, TRUE, 0);
    gtk_widget_show (rbutton);

    rbutton = gtk_radio_button_new_with_label (
	gtk_radio_button_group (GTK_RADIO_BUTTON (rbutton)), 
	_("Remove Linux partitions"));
    gtk_object_set_data (GTK_OBJECT (rbutton), "Option", 
			 GINT_TO_POINTER (FSEDIT_CLEARLINUX));
    gtk_signal_connect (GTK_OBJECT (rbutton), "toggled",
				 auto_partition_rbutton_cb,
				 &flags);
    gtk_box_pack_start (GTK_BOX (vbox), rbutton, TRUE, TRUE, 0);
    gtk_widget_show (rbutton);

    rbutton = gtk_radio_button_new_with_label (
	gtk_radio_button_group (GTK_RADIO_BUTTON (rbutton)), 
	_("Use existing free space"));
    gtk_object_set_data (GTK_OBJECT (rbutton), "Option", 
			 GINT_TO_POINTER (FSEDIT_USEEXISTING));
    gtk_signal_connect (GTK_OBJECT (rbutton), "toggled",
				 auto_partition_rbutton_cb,
				 &flags);
    gtk_box_pack_start (GTK_BOX (vbox), rbutton, TRUE, TRUE, 0);
    gtk_widget_show (rbutton);

    gtk_widget_show_all (frame);

    /* workstation or server? */
    frame = gtk_frame_new (_("Intended Use"));
    gtk_box_pack_start (GTK_BOX (dialog_vbox), frame, TRUE, TRUE, 0);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (frame), vbox);

    server = FALSE;
    rbutton = gtk_radio_button_new_with_label (NULL, "Workstation");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rbutton), TRUE);
    gtk_object_set_data (GTK_OBJECT (rbutton), "Option", 
			 GINT_TO_POINTER(FALSE));
    gtk_signal_connect (GTK_OBJECT (rbutton), "toggled",
				 auto_partition_rbutton_cb,
				 &server);
    gtk_box_pack_start (GTK_BOX (vbox), rbutton, TRUE, TRUE, 0);
    gtk_widget_show (rbutton);

    rbutton = gtk_radio_button_new_with_label (
	gtk_radio_button_group (GTK_RADIO_BUTTON (rbutton)), 
	_("Server"));
    gtk_object_set_data (GTK_OBJECT (rbutton), "Option", 
			 GINT_TO_POINTER (TRUE));

    gtk_signal_connect (GTK_OBJECT (rbutton), "toggled",
				 auto_partition_rbutton_cb,
				 &server);
    gtk_box_pack_start (GTK_BOX (vbox), rbutton, TRUE, TRUE, 0);
    gtk_widget_show (rbutton);

    gtk_widget_show_all (frame);

    rc = gnome_dialog_run_and_close (GNOME_DIALOG (dialog));

    g_print ("Choice = %d type = %d rc = %d\n", flags, server, rc);

    if (rc != 0)
	return;

    status = guessAtPartitioning (context->state, 
				  flags, (server) ? _("Server") : _("Workstation"),
				  (server) ? serverPartitioning : normalPartitioning, TRUE);

    if (status == 0) {
	update_gui (context);
	context->changesmade = TRUE;
    } /*else { */
/*  	if (status == INST_CANCEL) */
/*  	    ddruidShowMessage (_("Auto-partioning Aborted"), _("Ok"), */
/*  			       _("You have chosen to abort auto-partitioning.")); */
/*      } */
/*  	else */
/*  	    ddruidShowMessage (_("Auto-partitioning Failed"), _("Ok"), */
/*  			       _("Could not auto-partition your system.")); */
/*      } */
}


static void
editCurrentMntpt (HardDriveContext *context)
{
    gint selrow, selspec;
    gint status;
    RaidSpecEntry *raidentry;
    MntptClistRowData *rowdata;

    if (mntpt_clist && GTK_CLIST (mntpt_clist)->selection) {
	selrow = GPOINTER_TO_INT (GTK_CLIST (mntpt_clist)->selection->data);
	
	rowdata = gtk_clist_get_row_data (GTK_CLIST (mntpt_clist), selrow);
	selspec = rowdata->specnum;
	raidentry = rowdata->raidentry;
	if (selspec >= 0 && raidentry == NULL) {
	    status = DoMenuFunction("EDIT", context, selspec);
	} else if (raidentry) {
	    status = editraid_dialog_run (context, raidentry);
	}

	if (status == FDISK_SUCCESS) {
	    update_gui (context);
	    context->changesmade = TRUE;
	}
    }
}

static void
edit_button_cb (GtkButton *button, HardDriveContext *context)
{
    editCurrentMntpt (context);
}

static void
del_button_cb (GtkButton *button, HardDriveContext *context)
{
    gint selrow, selspec;
    gint status;
    RaidSpecEntry *raidentry;

    MntptClistRowData *rowdata;

    if (context->dontPartition)
	g_print ("dontPartition is TRUE and we DIDNT Check if NFS or not\n");

    if (mntpt_clist && GTK_CLIST (mntpt_clist)->selection) {
	selrow = GPOINTER_TO_INT (GTK_CLIST (mntpt_clist)->selection->data);

	rowdata = gtk_clist_get_row_data (GTK_CLIST (mntpt_clist), selrow);
	selspec = rowdata->specnum;
	raidentry = rowdata->raidentry;

	status = -1;
	if (selspec >= 0 && raidentry == NULL)
	    status=DoMenuFunction("DEL", context, selspec);
	else if (raidentry != NULL) {
	    if (ddruidYesNo (_("Delete RAID Device?"), _("Yes"), _("No"),
			     _("Are you sure you want to remove this RAID device?"), 0) != 1)
		status = FDISK_ERR_USERABORT;
	    else {
		/* see if this was a /boot mntpt, if so we need to remove */
		/* cylinder contraints for component partitions */
		if (!strcmp (raidentry->mntpt, "/boot")) {
		    GList *s = raidentry->partitions;
		    Partition *p;

		    while (s) {
			int rc;

			rc = fdiskReturnPartitionSpec(&context->state->spec,
						      s->data, &p );
			if (rc) {
			    g_print ("Error finding partition %s\n",
				     (char *)s->data);
			    break;
			}

			/* only set if this is not a preexisting partition */
			if (!p->immutable) {
			    fdiskSetConstraint(&p->endcyl, 0, FDISK_ENDCYL_MIN,
					       FDISK_ENDCYL_MAX,0);
			    fdiskModifyPartitionSpec (&context->state->spec,
						      s->data, p, 
						      REQUEST_PENDING );
			}

			free (p);

			s = s->next;
		    }
		}
		status = deleteRaidSpecEntry (&context->raidarray, raidentry);
	    }
	}

	if (status == FDISK_SUCCESS) {
	    update_gui (context);
	    context->changesmade=TRUE;
	}
    }
    
}

static void
reset_button_cb (GtkButton *button, HardDriveContext *context)
{
    gint i;

    if (ddruidYesNo(_("Reset Partition Table"),
		      _("Yes"), _("No"),
		      _("Reset partition table to original "
			"contents? "), 0) == 1) {
	for (i=0; i<context->state->numhd; i++)
	    memcpy(context->state->hdarr[i],context->state->prestinehdarr[i],
		   sizeof(HardDrive));

	wipeRaidArray (&context->raidarray);
	fdiskWipePartitionSpec(&context->state->spec);
	fdiskSetupPartitionSpec( context->state->hdarr, context->state->numhd, 
				 &context->state->spec );

	context->changesmade = FALSE;
	update_gui (context);
    }
}


static char **
create_mntpt_row (Partition *p, HardDrive *hd, PartitionSpecEntry *entry )
{
    unsigned int num, minsize, actsize;
    char         *pname, *devname;
    char         **row;
    int          k, l;

    row = g_new (gchar *, 6);

    num     = p->num.current;
    minsize = p->size.min;
    pname   = entry->name;

    /* create clist row */
    if (strncmp("Exist", pname, 5) && strncmp("Swap", pname, 4) &&
	strncmp("Dos", pname, 3) && strncmp("Raid", pname, 4))
	row[0] = pname;
    else if (!strncmp("Swap", pname, 4))
	row[0] = _("<Swap>");
    else if (!strncmp("Raid", pname, 4))
	row[0] = _("<RAID>");
    else
	row[0] = _("<not set>");
    
    row[2] = g_strdup_printf ("%6dM", minsize/SECPERMEG);

    if (entry->status != REQUEST_DENIED && hd != NULL) {
	actsize = p->size.current;
	devname = hd->prefix;
	row[1] = g_strdup_printf ("%s%d", devname, num);
	row[3] = g_strdup_printf ("%6dM", actsize/SECPERMEG);
    } else {
	row[1] = g_strdup ("");
	row[3] = g_strdup ("");
    }
    
    if (hd && hd->part_type == FDISK_PART_TYPE_SUN)
	l = p->type.current | SUNPARTTYPE;
    else
	l = p->type.current | NONSUNPARTTYPE;
    
    for (k=0; k<nparttypes; k++)
	if (allparttypes[k].index == p->type.current ||
	    allparttypes[k].index == l)
	    break;
    
    if (k != nparttypes)
	row[4] = g_strdup_printf("%s", allparttypes[k].name);
    else
	row[4] = g_strdup_printf("0x%02x", p->type.current);
    
    row[5] = NULL;

    return row;
}

static char **
create_raidarr_row (RaidSpecEntry *raidspec, HardDriveContext *context)
{
    GString      *drives;
    char         **row;
    GList       *s;

    row = g_new (gchar *, 6);

    /* create clist row */
    if (raidspec->parttype != LINUX_SWAP_PARTITION)
	row[0] = raidspec->mntpt;
    else 
	row[0] = _("<Swap>");
    row[1] = g_strdup_printf ("%s", raidspec->device);
    row[2] = "";
    row[3] = g_strdup_printf ("%6dM",sizeRaidSpecEntry (raidspec,context)/SECPERMEG);

    s = raidspec->partitions;

    drives = g_string_new (raidtypesnames[raidspec->type]);
    g_string_append (drives, " [");

    while (s) {
	int rc, i;
	
	rc = fdiskIndexPartitionSpec (&context->state->spec, s->data, &i);

	if (rc)
	    g_warning ("Couldnt find partition named %s\n",(char *)s->data);
	else
	    g_string_append (drives, deviceFromSpecEntry (context, i));

	if (s->next)
	    g_string_append (drives, ", ");

	s = s->next;
    }

    g_string_append (drives, "]");
    row[4] = drives->str;

    g_string_free (drives, FALSE);

    row[5] = NULL;

    return row;
}

static void
free_mntpt_row (gchar **row)
{
    g_free (row[1]);
    g_free (row[2]);
    g_free (row[3]);
    g_free (row[4]);

    g_free (row);
}

static void
free_raidarr_row (gchar **row)
{
    g_free (row[1]);
    g_free (row[3]);
    g_free (row[4]);

    g_free (row);
}

static void 
mntpt_clist_select_cb(GtkWidget *widget, gint row, gint column,
		    GdkEventButton *event, gpointer data)
{
    gint hilite, hilite_row, selrow;
    MntptClistRowData *rowdata;
    HardDriveContext *context=data;

    if (ignore_mntpt_clist_events)
	return;

    if (event && event->type == GDK_2BUTTON_PRESS && event->button == 1) {
	editCurrentMntpt (context);
    } else {
	selrow = GPOINTER_TO_INT (GTK_CLIST (mntpt_clist)->selection->data);

	rowdata = gtk_clist_get_row_data (GTK_CLIST (widget), row);

	if (!rowdata)
	    return;
	
	/* see if raid entry or not */
	if (rowdata->hilite < 0)
	    return;

	hilite = rowdata->hilite;

	/* WARNING - drvsum_clist is a GLOBAL variable! */
	hilite_row = gtk_clist_find_row_from_data (GTK_CLIST (drvsum_clist),
						   GINT_TO_POINTER (hilite));
	gtk_clist_select_row (GTK_CLIST (drvsum_clist), hilite_row, -1);
    } 
}

static void
update_mntpt_clist (HardDriveContext *context)
{
    int  hdidx, drivenum, foundpart;
    int  i, totalused;
    int  rowindex;
    GList *tmplist;
    GdkColor unalloc_color;
    GtkStyle *style;

    g_return_if_fail (mntpt_clist != NULL);

    ignore_mntpt_clist_events = TRUE;

    gtk_clist_clear (GTK_CLIST (mntpt_clist));

    for (hdidx=0; hdidx < context->state->numhd; hdidx++) {
	drivenum = context->state->newhdarr[hdidx]->num;

/* display all spec'd partitions for this drive */
	foundpart = 0;
	totalused = 0;
	for (i=0; i<context->state->spec.num; i++) {
	    MntptClistRowData *rowdata;
	    Partition    *p;
	    unsigned int drive;
	    char         **row;

	    if (context->state->spec.entry[i].status == REQUEST_DENIED)
		continue;
	    
	    p = &(context->state->spec.entry[i].partition);
	    if ((drive = p->drive.current) != drivenum)
		continue;

	    if (fdiskIsExtended(p->type.current))
		continue;

	    if (specInRaidArray (context->raidarray, &context->state->spec, i))
		continue;

	    row = create_mntpt_row (p, context->state->newhdarr[hdidx], 
			      &context->state->spec.entry[i]);
	    rowindex = gtk_clist_append (GTK_CLIST (mntpt_clist), row);
	    rowdata = g_new (MntptClistRowData, 1);
	    rowdata->hilite = hdidx;
	    rowdata->specnum = i;
	    rowdata->raidentry = NULL;
	    gtk_clist_set_row_data_full (GTK_CLIST (mntpt_clist), rowindex,
				    rowdata, g_free);
	    free_mntpt_row (row);
	}
    }

    /* now add raid partitions */
    for (tmplist=context->raidarray; tmplist; tmplist=tmplist->next) {
	MntptClistRowData *rowdata;
	char **row;

	row = create_raidarr_row (tmplist->data, context);

	rowindex = gtk_clist_append (GTK_CLIST (mntpt_clist), row);

	rowdata = g_new (MntptClistRowData, 1);
	rowdata->hilite = -1;
	rowdata->specnum = -1;
	rowdata->raidentry = (RaidSpecEntry *)tmplist->data;
	gtk_clist_set_row_data_full (GTK_CLIST (mntpt_clist), rowindex,
				     rowdata, g_free);

	free_raidarr_row (row);
    }

    unalloc_color.red     = UNALLOCATED_COLOR_R;
    unalloc_color.green   = UNALLOCATED_COLOR_G;
    unalloc_color.blue    = UNALLOCATED_COLOR_B;

    style = gtk_style_new();
    style->fg[GTK_STATE_NORMAL] = unalloc_color;

/* this makes it hard to read when selected */
/*    style->fg[GTK_STATE_SELECTED] = unalloc_color;  */

      
  
    /* now add unallocated partitions */
    for (i=0; i<context->state->spec.num; i++) {
	MntptClistRowData *rowdata;
	Partition    *p;
	char         **row;

	if (context->state->spec.entry[i].status != REQUEST_DENIED)
	    continue;
	    
	p = &(context->state->spec.entry[i].partition);

	if (fdiskIsExtended(p->type.current))
	    continue;

	if (specInRaidArray (context->raidarray, &context->state->spec, i))
	    continue;
	
	row = create_mntpt_row (p, NULL, &context->state->spec.entry[i]);
	rowindex = gtk_clist_append (GTK_CLIST (mntpt_clist), row);

/*
	red.red   = 0xffff;
	red.blue  = 0x0000;
	red.green = 0x0000;
	gtk_clist_set_foreground (GTK_CLIST (mntpt_clist), rowindex,&red);
*/
	gtk_clist_set_row_style (GTK_CLIST (mntpt_clist), rowindex, style);

	rowdata = g_new (MntptClistRowData, 1);
	rowdata->hilite = -1;
	rowdata->specnum = i;
	rowdata->raidentry = NULL;
	gtk_clist_set_row_data_full (GTK_CLIST (mntpt_clist), rowindex,
				     rowdata, g_free);
	
	free_mntpt_row (row);
    }

/*    gtk_clist_columns_autosize (GTK_CLIST (mntpt_clist)); */

    ignore_mntpt_clist_events = FALSE;

}

static void
create_mntpt_clist (GtkWidget **retval, GtkWidget **scrollwin ) 
{
    gchar *titles[] = { "Mount Point", 
			"Device", 
			"Requested",
			"Actual",
			"Type",
			NULL
    };
    GtkJustification col_justs[] = { GTK_JUSTIFY_LEFT,
				     GTK_JUSTIFY_LEFT,
				     GTK_JUSTIFY_RIGHT,
				     GTK_JUSTIFY_RIGHT,
				     GTK_JUSTIFY_LEFT,
				     -1
    };
   gint i;

    *scrollwin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (*scrollwin),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);

    *retval = gtk_clist_new_with_titles (5, titles);
    gtk_clist_set_selection_mode (GTK_CLIST (*retval), GTK_SELECTION_BROWSE);
    gtk_clist_column_titles_passive(GTK_CLIST (*retval));
    for (i=0; i<5; i++) {
	gtk_clist_set_column_resizeable (GTK_CLIST (*retval), i, FALSE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (*retval), i, TRUE);
	
	gtk_clist_set_column_justification (GTK_CLIST (*retval), i,
					    col_justs[i]);
    }
						
    gtk_widget_set_usize (*retval, -1, 100);    
    gtk_container_add (GTK_CONTAINER (*scrollwin), *retval);

    gtk_widget_show_all (*scrollwin);
}


static unsigned int
drive_space_used (HardDrive **hdarr, PartitionSpec *spec, 
		  unsigned int num)
{
    unsigned int drivenum = hdarr[num]->num;
    unsigned int totalused;
    int i;

    totalused = 0;
    for (i=0; i<spec->num; i++) {
	Partition    *p;
	
	if (spec->entry[i].status == REQUEST_DENIED)
	    continue;
	
	p = &spec->entry[i].partition;
	if (p->drive.current != drivenum)
	    continue;
	
	if (fdiskIsExtended(p->type.current))
	    continue;

	totalused += p->size.current;

    }
    return totalused;
}

static void
update_drvsum_clist (HardDriveContext *context)
{
    int  hdidx, per, rowindex;
    GdkPixmap *pixmap;
    static GdkGC *newgc=NULL;
    static GdkColor red     = {0, 0xffff,      0,      0};
    static GdkColor green   = {0, 0     , 0xfff0,      0};
    static GdkColor yellow  = {0, 0xf000, 0xf000, 0x3000};
    static gboolean colors_init = FALSE;

    g_return_if_fail (drvsum_clist != NULL);

    if (!colors_init) {
	    gdk_colormap_alloc_color (gtk_widget_get_colormap (drvsum_clist),
				      &red, FALSE, TRUE);
	    gdk_colormap_alloc_color (gtk_widget_get_colormap (drvsum_clist),
				      &green, FALSE, TRUE);
	    gdk_colormap_alloc_color (gtk_widget_get_colormap (drvsum_clist),
				      &yellow, FALSE, TRUE);
	    colors_init = TRUE;
    }

    gtk_clist_clear (GTK_CLIST (drvsum_clist));

    for (hdidx=0; hdidx < context->state->numhd; hdidx++) {
	char     *row[7];
	int      i;
	unsigned int drvused = drive_space_used (context->state->newhdarr, 
						 &context->state->spec, hdidx);
	
	row[0] = g_strdup_printf ("%-8s", context->state->newhdarr[hdidx]->name+5);
	row[1] = g_strdup_printf ("[%5d/%3d/%2d]", 
				  context->state->newhdarr[hdidx]->geom.cylinders,
				  context->state->newhdarr[hdidx]->geom.heads,
				  context->state->newhdarr[hdidx]->geom.sectors);
	row[2] = g_strdup_printf ("%6dM", context->state->newhdarr[hdidx]->totalsectors/SECPERMEG);
	row[3] = g_strdup_printf ("%6dM", context->state->hdarr[hdidx]->totalsectors/SECPERMEG - drvused/SECPERMEG);
	row[4] = g_strdup_printf ("%6dM", drvused/SECPERMEG);
	per = (100 * (drvused/SECPERMEG)/(context->state->hdarr[hdidx]->totalsectors /
					  SECPERMEG));
	row[5] = g_strdup_printf ("%3d%%", per);
	row[6] = NULL;
	rowindex = gtk_clist_append (GTK_CLIST (drvsum_clist), row);
	gtk_clist_set_row_data (GTK_CLIST (drvsum_clist), rowindex,
				GINT_TO_POINTER (hdidx));
#define PERCENTBAR_LEN 32
#define PERCENTBAR_HT  12
#define PERCENTBAR_BORDER 1

	per = ((PERCENTBAR_LEN - PERCENTBAR_BORDER*2) * (per))/100;

	pixmap = gdk_pixmap_new (NULL, PERCENTBAR_LEN, PERCENTBAR_HT, 
			 gtk_widget_get_visual (drvsum_clist)->depth);

	if (!newgc)
	    newgc = gdk_gc_new (pixmap);

	if (per > 0.75*PERCENTBAR_LEN)
	    gdk_gc_set_foreground (newgc, &red);
	else if (per > 0.50*PERCENTBAR_LEN)
	    gdk_gc_set_foreground (newgc, &yellow);
	else
	    gdk_gc_set_foreground (newgc, &green);

	gdk_draw_rectangle (pixmap, drvsum_clist->style->black_gc, 
			    TRUE, 0, 0, 
			    PERCENTBAR_LEN, PERCENTBAR_HT);
	gdk_draw_rectangle (pixmap, drvsum_clist->style->white_gc, 
			    TRUE, PERCENTBAR_BORDER, PERCENTBAR_BORDER, 
			    PERCENTBAR_LEN-PERCENTBAR_BORDER*2,
			    PERCENTBAR_HT-PERCENTBAR_BORDER*2);

	gdk_draw_rectangle (pixmap, newgc, TRUE, 
			    PERCENTBAR_BORDER, PERCENTBAR_BORDER, 
			    per, PERCENTBAR_HT-PERCENTBAR_BORDER*2);

	gtk_clist_set_pixtext (GTK_CLIST (drvsum_clist), rowindex, 5, 
			       row[5], 5, pixmap, NULL);

	gdk_pixmap_unref (pixmap);

	for (i=0; i<6; i++)
	    g_free (row[i]);
	
    }

}



/* sole purpose is so we can make the drvsum_clist insensitive to user */
/* button events                                                       */
static gint
stop_clist_btnpress (GtkWidget *clist, GdkEvent *event, gpointer data)
{

    gtk_signal_emit_stop_by_name (GTK_OBJECT (clist), "button_press_event");
    return TRUE;
}


static void
create_drvsum_clist (GtkWidget **retval, GtkWidget **scrollwin )
{
    gchar *titles[] = { "Drive", 
			"Geom [C/H/S]", 
			"Total (M)",
			"Free (M)",
			"Used (M)",
			"Used (%)",
			NULL
    };
    GtkJustification col_justs[] = { GTK_JUSTIFY_LEFT,
				     GTK_JUSTIFY_LEFT,
				     GTK_JUSTIFY_RIGHT,
				     GTK_JUSTIFY_RIGHT,
				     GTK_JUSTIFY_RIGHT,
				     GTK_JUSTIFY_LEFT,
				     -1
    };
    gint i;

    *scrollwin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (*scrollwin),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);

    *retval = gtk_clist_new_with_titles (6, titles);
    gtk_clist_set_selection_mode (GTK_CLIST (*retval), GTK_SELECTION_BROWSE);
    gtk_clist_column_titles_passive(GTK_CLIST (*retval));
    for (i=0; i<6; i++) {
	gtk_clist_set_column_resizeable (GTK_CLIST (*retval), i, FALSE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (*retval), i, TRUE);
	gtk_clist_set_column_justification (GTK_CLIST (*retval), i,
					    col_justs[i]);
    }
    gtk_container_add (GTK_CONTAINER (*scrollwin), *retval);

    gtk_signal_connect (GTK_OBJECT (*retval), "button_press_event",
			GTK_SIGNAL_FUNC (stop_clist_btnpress), NULL);

    gtk_widget_show_all (*scrollwin);
}


/* returns TRUE if requested mount point has been defined and received */
/* its requested allocation. Checks raidtab and fstab.                 */
/* Also tests that the mount point is a valid boot partition (ext2 or RAID1) */
/* if bootable is passed as TRUE                                       */
static gboolean
mntpt_present_and_allocated (HardDriveContext *context, gchar *mntpt,
				  gboolean bootable)
{
    gint status, i;
    RaidSpecEntry *raidentry;
    GList *s;

    /* first test fstab */
    status = fdiskIndexPartitionSpec(&context->state->spec, mntpt, &i);
    if (!status) {
	if (!bootable)
	    return TRUE;

	if (context->state->spec.entry[i].partition.type.current != LINUX_NATIVE_PARTITION ||
	    context->state->spec.entry[i].status == REQUEST_DENIED)
	    return FALSE;
	else
	    return TRUE;
    }

    /* now test raidtab */
    raidentry = findMntPtInRaidArray (context->raidarray, mntpt);
    if (raidentry) {

	/* make sure all component partitions are allocated */
	s = raidentry->partitions;
	while (s) {
	    status = fdiskIndexPartitionSpec(&context->state->spec,
					     s->data, &i);
	    if (status)
		return FALSE;

	    if (context->state->spec.entry[i].status == REQUEST_DENIED)
		return FALSE;

	    s=s->next;
	}

	if (!bootable)
	    return TRUE;

	/* must be RAID1 and ext2 */
	if (raidentry->type != RAID1 || 
	    raidentry->parttype != LINUX_NATIVE_PARTITION)
	    return FALSE;
	else
	    return TRUE;
    } else {
	return FALSE;
    }

}


/* returns TRUE if swap exists in raidtab and/or fstab and is allocated */
static gboolean
swap_present_and_allocated (HardDriveContext *context)
{
    gint status, i;
    RaidSpecEntry *raidentry;
    GList *s, *t;
    gboolean swap_found;

    /* first test fstab */
    /* make sure some swapspace is defined */
    status = 1;
    for (i=0; i < context->state->spec.num; i++)
	if (context->state->spec.entry[i].partition.type.current == LINUX_SWAP_PARTITION &&
	    context->state->spec.entry[i].status != REQUEST_DENIED) {
	    status = 0;
	    break;
	}

    if (!status)
	return TRUE;

    /* now test raidtab */
    t = context->raidarray;
    swap_found = FALSE;
    while (t) {

	raidentry = (RaidSpecEntry *)t->data;
	if (raidentry->parttype != LINUX_SWAP_PARTITION) {
	    t = t->next;
	    continue;
	}

	swap_found = TRUE;
	/* make sure all component partitions are allocated */
	s = raidentry->partitions;
	while (s) {
	    status = fdiskIndexPartitionSpec(&context->state->spec,
					     s->data, &i);
	    if (status)
		return FALSE;

	    if (context->state->spec.entry[i].status == REQUEST_DENIED)
		return FALSE;

	    s=s->next;
	}

	t=t->next;
    }

    return swap_found;
}

/* returns true if minimum requirements for partitioning are met */
static gboolean
meets_partition_requirements (HardDriveContext *context)
{
    gboolean pass;
    gint status;
    gint i;

    if (validLoopbackSetup(&context->state->spec)) return TRUE;

    /* check for a valid /boot */
    pass = mntpt_present_and_allocated (context, "/boot", TRUE);

    if (!pass) {
	/* ok no /boot so we must have a bootable "/" */
	pass = mntpt_present_and_allocated (context, "/", TRUE);
    } else {
	/* we have a bootable /boot, so just make sure "/" exists */
	pass = mntpt_present_and_allocated (context, "/", FALSE);
    }

    /* see if we passed tests so far */
    if (!pass)
	return FALSE;

    return swap_present_and_allocated(context);
}

static GtkWidget *
make_confirm_msgbox (gchar *message)
{
    GtkWidget *msgvbox, *msgframe;
    GtkWidget *label;

    msgframe = gtk_frame_new ("");
    msgvbox = gtk_vbox_new (FALSE, 0);
    
    label = gtk_label_new (message);
    gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

    gtk_container_add (GTK_CONTAINER (msgframe), msgvbox);
    gtk_box_pack_start (GTK_BOX (msgvbox), label, FALSE, FALSE, 0);
    return msgframe;
}

GtkWidget *
create_confirm_window (void)
{
    HardDriveContext *context;
    RaidSpecEntry *s;
    gboolean  boot_ok;
    gboolean pass;

    gint i, status;
 
    /* XXXX using global context here */
    context = &globalContext;

    /* see if there are any conditions we want to warn user about */
    /* make sure there are no unallocated partitions */
    status = 0;
    for (i=0; i < context->state->spec.num; i++)
	if (context->state->spec.entry[i].status == REQUEST_DENIED) {
	    status = 1;
	    break;
	}
    
    if (status)
	return make_confirm_msgbox (_("Unallocated Partitions Exist..."));

    /* see if we have a root partition defined */
    pass = mntpt_present_and_allocated (context, "/", FALSE);
    if (!pass)
	return make_confirm_msgbox (
	    _("You must assign a root (/) "
	      "partition "
	      "to a Linux native partition (ext2) "
	      "or a RAID partition for the install to "
	      "proceed."));

    /* check out /boot if it exists */
    boot_ok = mntpt_present_and_allocated (context, "/boot", TRUE);

    if (!boot_ok) {
	/* make sure that since we have no /boot that our */
	/* / is bootable                                  */
	if (!mntpt_present_and_allocated (context, "/", TRUE))
	    return make_confirm_msgbox (
		_("You must assign a root (/) "
		  "partition "
		  "to a Linux native partition (ext2) "
		  "or a RAID partition for the install to "
		  "proceed."));
    }

    return NULL;
}
    
GtkWidget *
create_main_window (void)
{
    GtkWidget *retval;
    
    /* old toplevel window code */
    /*retval = gtk_window_new (GTK_WINDOW_TOPLEVEL); */
    /*    gtk_window_set_title (GTK_WINDOW (retval), "Gdiskdruid");  */

    accelgroup = gtk_accel_group_new();
    mainwin = retval = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (retval), GTK_SHADOW_NONE);
    gtk_container_set_border_width (GTK_CONTAINER (retval), 0);
    gtk_widget_set_usize (mainwin, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);
    return retval;
}


static void
update_buttonbar (HardDriveContext *context)
{
    gtk_widget_set_sensitive (del_button, (context->state->spec.num != 0));
    gtk_widget_set_sensitive (addraid_button, 
			      PartitionSpecHasRaid (&context->state->spec,
						    context->raidarray));
}

static GtkWidget *
accel_button(GtkAccelGroup *group, const char *text)
{
    GtkWidget *button, *label;
    gchar key;
    
    label = gtk_label_new(text);
    gtk_misc_set_padding (GTK_MISC (label), 5, 0);
    key = gtk_label_parse_uline(GTK_LABEL(label), text);
    button = gtk_button_new();
    gtk_container_add (GTK_CONTAINER(button), label);

/* uncomment to get key bound with NO modifier */
/*    gtk_widget_add_accelerator (button, "clicked", group, key, 0, 0); */

    gtk_widget_add_accelerator (button, "clicked", group, key, GDK_MOD1_MASK,
				0);

    return button;
}

static void
init_gui (HardDriveContext *context)
{
    GtkWidget *vbox, *vbox2;
    GtkWidget *hbox;
    GtkWidget *frame;
    GtkWidget *scrollwin;
    GtkWidget *toplevel;

    vbox = gtk_vbox_new (FALSE, /*5*/ 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
    gtk_container_add (GTK_CONTAINER (mainwin), vbox);

    frame = gtk_frame_new (_("Partitions"));

    gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

    vbox2 = gtk_vbox_new (FALSE, 2);
    gtk_container_add (GTK_CONTAINER (frame), vbox2);
    create_mntpt_clist (&mntpt_clist, &scrollwin);
    gtk_box_pack_start (GTK_BOX (vbox2), scrollwin, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (mntpt_clist), "select_row",
			GTK_SIGNAL_FUNC (mntpt_clist_select_cb),
			context);

    /* XXXXX - temp hack */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 3);
    gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);
    gtk_widget_show (hbox);

    /* create key accelerators */
    toplevel = gtk_widget_get_toplevel (mainwin);
    
/*      g_assert (toplevel != NULL); */
/*      g_assert (GTK_IS_WINDOW (toplevel)); */

/*      if (toplevel != NULL && GTK_IS_WINDOW (toplevel)) */
/*  	gtk_window_add_accel_group (GTK_WINDOW (toplevel), accelgroup); */
    
    if (context->dontPartition) {
	add_button = NULL;
    } else {
	add_button = accel_button (accelgroup, _("_Add...")); 
	gtk_box_pack_start (GTK_BOX (hbox), add_button, TRUE, FALSE, 0);

	gtk_signal_connect (GTK_OBJECT (add_button), "clicked",
			    GTK_SIGNAL_FUNC (add_button_cb), context);
    }

    edit_button = accel_button (accelgroup, _("_Edit..."));
    reset_button = accel_button (accelgroup, _("_Reset"));

    gtk_box_pack_start (GTK_BOX (hbox), edit_button, TRUE, FALSE, 0);

    if (!context->dontPartition) {
      del_button = accel_button (accelgroup, _("_Delete"));
      gtk_box_pack_start (GTK_BOX (hbox), del_button, TRUE, FALSE, 0);
      gtk_signal_connect (GTK_OBJECT (del_button), "clicked",
			  GTK_SIGNAL_FUNC (del_button_cb), context);
    }

    gtk_box_pack_start (GTK_BOX (hbox), reset_button, TRUE, FALSE, 0);
    gtk_signal_connect (GTK_OBJECT (edit_button), "clicked",
			GTK_SIGNAL_FUNC (edit_button_cb), context);
    gtk_signal_connect (GTK_OBJECT (reset_button), "clicked",
			GTK_SIGNAL_FUNC (reset_button_cb), context);

    /* try having make raid device button available in read-only mode too */
    addraid_button = accel_button (accelgroup, _("_Make RAID Device"));
    gtk_box_pack_start (GTK_BOX (hbox), addraid_button, TRUE, FALSE, 0);

    gtk_signal_connect (GTK_OBJECT (addraid_button), "clicked",
			GTK_SIGNAL_FUNC (addraid_button_cb), context);


/* XXXX - uncomment if you want the add raid button */
    if (!context->dontPartition) {

  	auto_button = accel_button (accelgroup, _("Auto Partition"));
/* uncomment to get auto-partition button to be packed */
#if 0
	gtk_box_pack_start (GTK_BOX (hbox), auto_button, TRUE, FALSE, 0);
	gtk_signal_connect (GTK_OBJECT (auto_button), "clicked",
			    GTK_SIGNAL_FUNC (auto_button_cb), context);
#endif
    }

    frame = gtk_frame_new (_("Drive Summary"));
    gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

    create_drvsum_clist (&drvsum_clist, &scrollwin);
    gtk_container_add (GTK_CONTAINER (frame), scrollwin);

    gtk_widget_show_all (mainwin);
}

static void
update_gui (HardDriveContext *context)
{
	update_mntpt_clist (context); 
	update_drvsum_clist (context);
	update_buttonbar (context);

	/* set state of the 'Next' button in anaconda framework */

	fsedit_enable_next (context->state->cbi.function,
			    context->state->cbi.data,
			    meets_partition_requirements (context));
}

#ifdef GNOME_FSEDIT_DEBUG_OUTPUT
static void
print_context ( HardDriveContext *context )
{
/* print out information useful for debugging allocation problems */
    printf ("---------------------------------------------------\n");
    printf ("Prestine partition table   (prestinehdarr)...\n");
    print_user_partition_table (context->state->prestinehdarr[0]);
    printf ("\nStarting partition table (hdarr)...\n");
    print_user_partition_table (context->state->hdarr[0]);
    printf ("\nCurrent  partition table (newhdarr)...\n");
    print_user_partition_table (context->state->newhdarr[0]);
    printf ("\nPartition Spec...\n");
    print_partitionspec (&context->state->spec);
    printf ("\n");
}
#endif

int
setupGlobalContext(FseditContext *state) 
{
    HardDriveContext *context;
    struct raidInstance * raid;

    context = &globalContext;
    context->state = state;

    context->dontPartition = 1;
    context->writeChanges = NULL;
    context->changesmade = FALSE;
    context->raidarray = NULL;

    raid = state->raid;
    if (raid) {
	int i=0;
	int j;

	while (raid[i].mntpt) {
	    RaidSpecEntry *raidspec;
	    
	    raidspec = g_new0( RaidSpecEntry, 1);
	    raidspec->mntpt  = g_strdup (raid[i].mntpt);
	    raidspec->device = g_strdup (raid[i].device);
	    raidspec->type   = raid[i].type;
	    raidspec->parttype = raid[i].parttype;
	    raidspec->partitions = NULL;

	    j = 0;
	    while (raid[i].devices[j]) {
		raidspec->partitions = g_list_append (raidspec->partitions,
						      raid[i].devices[j]);
		j++;
	    }

	    context->raidarray = g_list_append (context->raidarray, raidspec);

	    i++;
	}
    }

    return 0;
}

int
StartMaster( FseditContext *state,
	     struct fstab *remotefs,
	     int dontPartition,
	     int *writeChanges)
{
    HardDriveContext *context;

    g_return_val_if_fail (mainwin != NULL, -2);
    
    /* setup context so we can pass to callbacks */
    setupGlobalContext(state);

    context = &globalContext;
    context->dontPartition = dontPartition;
    context->writeChanges = writeChanges;

#ifdef GNOME_FSEDIT_DEBUG_OUTPUT
    printf ("On entry...\n");
    print_context (context);
#endif

    /* get things started */
    init_gui (context);

    update_gui (context);

    return 0;
}

