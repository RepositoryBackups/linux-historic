/* Newt based fdisk program
 *
 * Michael Fulbright (msf@redhat.com)
 *
 * Copyright 1999 Red Hat Software 
 *
 * This software may be freely redistributed under the terms of the GNU
 * public license.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <alloca.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <popt.h>

#include <newt.h>

#include "libfdisk.h"
#include "fstab.h"
#include "fsedit.h"
#include "newtfsedit.h"

#define VERSION_STR "1.00"


/* hardcoded, maybe someday we'll get these from tty ? */
int screen_width=80;
int screen_height=25;


static void
print_context ( FseditContext *state, FILE *file )
{
/* print out information useful for debugging allocation problems */
    fprintf (file, "---------------------------------------------------\n");
    fprintf (file, "Prestine partition table   (prestinehdarr)...\n");
    print_user_partition_table (state->prestinehdarr[0], file);
    fprintf (file, "\nStarting partition table (hdarr)...\n");
    print_user_partition_table (state->hdarr[0], file);
    fprintf (file, "\nCurrent  partition table (newhdarr)...\n");
    print_user_partition_table (state->newhdarr[0], file);
    fprintf (file, "\nPartition Spec...\n");
    print_partitionspec (&state->spec, file);
    fprintf (file, "\n");
    fflush(file);
}


/*                              */
/* NEWT/screen related routines */
/*                              */


void ddruidInitUI(void)
{
    newtInit();
    newtCls();
}

/* def = 1 for yes default'd, 0 for no default'd */
int ddruidYesNo(char * title, char * yes, char * no, char * text, int def){
    int retcode;

    /* XXXX don't set default currently */

    if (def == 0)
	retcode = newtWinChoice(title, yes, no, text);
    else
	retcode = newtWinChoice(title, no, yes, text);

    if (retcode == 2) return 0; else return 1;
}

/* give summary of why partitions weren't allocated */
static void showReasons( PartitionSpec *spec ) {
    newtComponent tbox, form, ok, lbox;
    int i;
    
    for (i=0; i<spec->num; i++)
	if (spec->entry[i].status == REQUEST_DENIED)
	    break;

    /* nothing going on here, keep moving along */
    if (i == spec->num)
	return;
    
    /* build list of why they all failed */
    newtCenteredWindow(60, 18, _("Unallocated Partitions"));
    form = newtForm(NULL,NULL,0);

    tbox = newtTextbox(5, 1, 50, 5, NEWT_FLAG_WRAP );
    newtTextboxSetText(tbox, _("There are currently unallocated partition(s) "
		       "present in the list of requested partitions. The "
		       "unallocated partition(s) are shown below, along with "
		       "the reason they were not allocated."));

    lbox = newtListbox(10, 6, 5, NEWT_FLAG_RETURNEXIT | NEWT_FLAG_SCROLL );
    for (i=0; i<spec->num; i++)
	if (spec->entry[i].status == REQUEST_DENIED) {
	    char tmpstr[80];
	    char *pname = spec->entry[i].name;
	    char *s, *t;
	    
	    memset(tmpstr, ' ', 80);
	    if (strncmp("Exist", pname, 5) && strncmp("Swap", pname, 4) &&
		strncmp("Dos", pname, 3))
		t = pname;
	    else
		t = NULL;
	    for (s=tmpstr;t && *t; t++,s++)
		*s = *t;
	    
	    t = GetReasonString(spec->entry[i].reason);
	    for (s=tmpstr+20;t && *t; t++,s++)
		*s = *t;
	    *s = '\0';
	    newtListboxAddEntry(lbox, tmpstr, NULL);
	}

    ok = newtButton(25, 13, _("Ok"));
    newtFormAddComponents(form, tbox, lbox, ok, NULL);
    newtFormSetCurrent(form, ok);

    newtRunForm(form);

    newtPopWindow();
    newtFormDestroy(form);
}

/* edit an existing partition spec */
/* callback for type listbox */
struct typebox_cbstruct {
    newtComponent *entry;
    char          *val;
};

static char typebox_mp[100];
static int  inswapentry;

static void typebox_scroll(newtComponent box, struct typebox_cbstruct *s ) {
    int type;
    
    type = (long) newtListboxGetCurrent(box);
    if (type == LINUX_SWAP_PARTITION && !inswapentry) {
	strncpy(typebox_mp, s->val, 100);
	newtEntrySet(*s->entry, _("Swap Partition"), 0);
	inswapentry = 1;
	newtEntrySetFlags(*s->entry, NEWT_FLAG_DISABLED, NEWT_FLAGS_SET);
    } else if (inswapentry) {
	newtEntrySetFlags(*s->entry, NEWT_FLAG_DISABLED, NEWT_FLAGS_RESET);
	if (typebox_mp[0] == -1) /* just clear string if it isnt initialized */
	    typebox_mp[0] = '\0';
	newtEntrySet(*s->entry, typebox_mp, 1);
	inswapentry = 0;
    }
}

struct driveentry_struct {
    newtComponent cb;
    char          state;
};

struct entrybox_cbstruct {
    newtComponent *form;
    char          *val;
    unsigned char *val2;
    DriveSet      curds, origds;
    int           numhd;
    HardDrive     **hdarr;
    struct driveentry_struct *de;
    int           dobootable;
};

static void entrybox_cb(newtComponent box, struct entrybox_cbstruct *s) {
    unsigned char boot;
    int           j;
    
    if (s->dobootable) {
	boot = (!strcmp(s->val, "/") || !strcmp(s->val, "/boot")) ? '*' : ' ';
	if (boot == '*' && *(s->val2) == ' ')
	    memcpy(&s->origds, &s->curds, sizeof(DriveSet));
        *(s->val2) = boot;
	if (boot == '*') {
	    fdiskDeactivateAllDriveSet( &s->curds );
	    fdiskActivateDriveSet( &s->curds, 1 );
	    fdiskActivateDriveSet( &s->curds, 2 );
	} else {
	    memcpy(&s->curds, &s->origds, sizeof(DriveSet));
	}

	for (j=0; j<s->numhd; j++)
	    s->de[j].state=fdiskThisDriveSetIsActive(&s->curds,
						     s->hdarr[j]->num)?'*':' ';
	
	newtDrawForm(*s->form);
    }
}    

#define NEW_PARTSPEC "NewPartition"

static int sizeEntryFilter(newtComponent entry, void * data, int ch,
			   int cursor)
{
    if ((ch < ' ' || ch >= NEWT_KEY_EXTRA_BASE) || (ch >= '0' && ch <= '9'))
	return ch;

    return 0;
}

static int mntptEntryFilter(newtComponent entry, void * data, int ch,
			    int cursor)
{
    if (ch == ' ')
	return 0;
    return ch;
}

static int EditPartitionSpec(HardDrive **hdarr, unsigned int numhd,
		      PartitionSpec      *spec, 
		      PartitionSpecEntry *entry) {
    int j, k, l;
    unsigned int hdidx, tmpuint;
    int          tmpint;
    char tmpstr[80];
    
    Partition *p;
    newtComponent form, mntptentry;
    newtComponent sizeentry, growentry, bootentry, typeentry;
    newtComponent sb, driveform;
    newtComponent ok, cancel, answer;

    struct typebox_cbstruct  cb1;
    struct entrybox_cbstruct cb2;
    struct driveentry_struct driveentry[MAX_HARDDRIVES];
    
    char *mntpt=NULL, *size=NULL, *eptr;
    char titlestr[80];
    unsigned char boot, grow;

    int row, col;
    int status=0;
    int done;
    int newpartition;
    int cval;
    int pcfstypes[] = {0x82, 0x83, 0xfd, 0x4, 0x6, -1};
    /* XXX I removed 0xfd from sunfstypes to make Jay Turner happy */
    int sunfstypes[] = {0x82, 0x83, 0x1, 0x2, 0x3, 0x4, 0x6, 0x7, 0x8, -1};
    int *fstypes;

    p = (Partition *) alloca(sizeof(Partition));
    memcpy(p, &entry->partition, sizeof(Partition));

    newpartition = (strcmp(entry->name, NEW_PARTSPEC) == 0);

    if (p->immutable)
	cval = -2;
    else
	cval = ((numhd > 3) ? 4 : numhd);

    /* make title line a little more descriptive */
    if (newpartition) {
	strcpy(titlestr, "Edit New Partition");
    } else if (p->immutable) {
	for (hdidx=0; hdidx < numhd &&
		 hdarr[hdidx]->num != p->drive.current; hdidx++);
	if (hdidx != numhd) {
	    snprintf(titlestr, 80, "%s: /dev/%s%d", _("Edit Partition"),
		     hdarr[hdidx]->prefix, p->num.current);
	    if (entry->name && *entry->name && strncmp(entry->name, "Exist", 5))
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
	
    newtCenteredWindow(70, 13+cval, titlestr );
    form = newtForm(NULL,NULL,0);

    /* mount point goes at top and is centered */
    row = 1;
    col = 3;
    newtFormAddComponent(form, newtLabel(col, row, "Mount Point:"));
    if (p->type.current != LINUX_SWAP_PARTITION) {
	if (!newpartition && strncmp("Exist", entry->name, 5) &&
	    strncmp("Dos", entry->name, 3)) {
	    mntptentry = newtEntry(22, row, entry->name, 30,
				   &mntpt, NEWT_FLAG_RETURNEXIT);
	} else {
	    mntptentry = newtEntry(22, row, "", 30,
				   &mntpt, NEWT_FLAG_RETURNEXIT);
	}
    } else {
	mntptentry = newtEntry(22, row, "Swap Partition", 30, &mntpt,
			       NEWT_FLAG_RETURNEXIT | NEWT_FLAG_DISABLED);
    }

    newtEntrySetFilter(mntptentry, mntptEntryFilter, NULL);
    
    /* size, grow and boot flags on left under mount point */
    row = 3;
    newtFormAddComponent(form, newtLabel(col, row, "Size (Megs):"));
    if (p->immutable) {
	sizeentry = NULL;
	snprintf(tmpstr,sizeof(tmpstr),"%d", p->size.current/SECPERMEG);
	newtFormAddComponent(form, newtLabel(22, row, tmpstr));
    } else {
	snprintf(tmpstr,sizeof(tmpstr),"%d", p->size.min/SECPERMEG);
	sizeentry = newtEntry(22, row, tmpstr, 8,
			  &size, NEWT_FLAG_RETURNEXIT);
	newtEntrySetFilter(sizeentry, sizeEntryFilter, NULL);
    }
    row++;
    
    if (!newpartition) {
	grow = p->size.min != p->size.max;
    } else {
	grow = 0;
    }
    grow = (grow) ? '*' : ' ';
    
    newtFormAddComponent(form, newtLabel(col, row, "Grow to fill disk?:"));
    if (p->immutable) {
	growentry = NULL;
	newtFormAddComponent(form, newtLabel(22, row, "[ ]"));
    } else {
	growentry = newtCheckbox(22, row, "", grow, NULL, &grow);
    }
    row++;

    /* give status */
    if (!newpartition) {
	newtFormAddComponent(form, newtLabel(col, row, 
				_("Allocation Status:")));
	if (entry->status != REQUEST_DENIED)
	    newtFormAddComponent(form, newtLabel(22, row, _("Successful")));
	else
	    newtFormAddComponent(form, newtLabel(22, row, _("Failed")));
	row++;

	if (entry->status == REQUEST_DENIED) {
	    newtFormAddComponent(form, newtLabel(col, row, 
				 _("Failure Reason:")));
	    newtFormAddComponent(form,
			 newtLabel(22,row,GetReasonString(entry->reason)));
	}
	row++;
    }
    
    /* blow this bootable stuff for now, its confusing */
    bootentry = NULL;

    /* type goes on right side under the mount point */
    row = 3;
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
    newtFormAddComponent(form, newtLabel(43, row, "Type:"));
    if (p->immutable) {
	typeentry = NULL;

	l |= p->type.current;

	for (j=0; j<nparttypes; j++) {
	    if (allparttypes[j].index == p->type.current)
		break;
	}
	if (j != nparttypes)
	    snprintf(tmpstr, sizeof(tmpstr), "%s", allparttypes[j].name);
	else
	    snprintf(tmpstr,sizeof(tmpstr),"%6s (0x%x)",
		     "Unknown", p->type.current);
	    
	newtFormAddComponent(form, newtLabel(48, row, tmpstr));
	row++;
    } else {
	typeentry = newtListbox( 48, row, 4, 
				 NEWT_FLAG_RETURNEXIT | NEWT_FLAG_SCROLL);
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
	    newtListboxAddEntry(typeentry, tmpstr,
				(void *) (long)fstypes[j]);
	    if (fstypes[j] == p->type.current)
		newtListboxSetCurrent(typeentry, j);
	    else if (p->type.current == 0 &&
		     fstypes[j] == LINUX_NATIVE_PARTITION)
		newtListboxSetCurrent(typeentry, j);
	}
    }

    /* have to fix this later */
    /* allowable drives goes in center under rest */
    row = 8;
    driveform = NULL;
    if (!p->immutable) {
	newtFormAddComponent(form, newtLabel(col, row, "Allowable Drives:"));

	sb = newtVerticalScrollbar(40, row, 4, 9, 10);
	driveform = newtForm(sb, NULL, 0);
        newtFormSetBackground(driveform, NEWT_COLORSET_CHECKBOX);

	for (j=0; j<numhd; j++) {
	    driveentry[j].state = fdiskThisDriveSetIsActive(&p->drive,
							     hdarr[j]->num);
	    driveentry[j].cb = newtCheckbox(22, row+j, hdarr[j]->name+5,
					    (driveentry[j].state) ? '*' : ' ',
					    NULL,
					    &driveentry[j].state);
	    newtFormAddComponent(driveform, driveentry[j].cb);
	}
	if (j > 4) {
	    newtFormSetHeight(driveform, 4);
	    newtFormAddComponent(driveform, sb);
	} else {
	    newtFormSetWidth(driveform, 10);
	}
    }
    
    /* setup type box callback */
    if (typeentry) {
	cb1.entry = &mntptentry;
	cb1.val   = mntpt;

	/* yuck but it works */
	typebox_mp[0] = -1;
	inswapentry = (p->type.current == LINUX_SWAP_PARTITION);
	newtComponentAddCallback(typeentry,(newtCallback) typebox_scroll,&cb1);
    }

    /* setup mount point callback */
    if (!p->immutable) {
	cb2.form      = &form;
	cb2.val       = mntpt;
	cb2.val2      = &boot;
	memset(&cb2.curds, 0, sizeof(DriveSet));
	memcpy(&cb2.origds, &p->drive, sizeof(DriveSet));
	cb2.numhd     = numhd;
	cb2.hdarr     = hdarr;
	cb2.de        = driveentry;
	cb2.dobootable = (fdiskIndexPartitionSpec(spec, "/boot", &j) !=
			  FDISK_SUCCESS);
	
	newtComponentAddCallback(mntptentry,(newtCallback) entrybox_cb,&cb2);
    }
				 
    row = 9+cval;
    ok = newtButton( 20, row, _("Ok"));
    cancel  = newtButton( 40, row, _("Cancel"));
    if (mntptentry)
	newtFormAddComponents( form,  mntptentry, NULL );
    if (sizeentry)
	newtFormAddComponents( form, sizeentry, NULL);
    if (growentry)
	newtFormAddComponents( form, growentry, NULL);
    if (typeentry)
	newtFormAddComponents( form, typeentry, NULL );
    if (driveform)
	newtFormAddComponents( form, driveform, NULL );
    newtFormAddComponents( form, ok, cancel, NULL);

    done = 0;
    while (!done) {
	answer = newtRunForm(form);

	if (answer != cancel) {
	    /* modify partition request based on the entry boxes */
	    if (typeentry) {
		tmpuint = (long) newtListboxGetCurrent( typeentry );
		fdiskSetConstraint(&p->type, tmpuint, tmpuint, tmpuint, 1);
	    }
	    
	    /* make sure mount point is valid */
	    if (p->type.current != LINUX_SWAP_PARTITION) {
		int valid=1;
		int skiprest=0;

		TrimWhitespace(mntpt);
		
		/* see if they even gave the partition a name  */
		/* we will ask them if they really want to not */
		/* assign the partition a name at this time if */
		/* they have just created a non-ext2 partition */
		if (!*mntpt && p->type.current != LINUX_NATIVE_PARTITION) {
		    if (newtWinChoice(_("No Mount Point"), _("Yes"), _("No"),
				      _("You have not selected a mount point "
				        "for this partition. Are you sure you "
				        "want to do this?")) == 2)
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

		if (!valid) {
		    newtWinMessage(_("Mount Point Error"), _("Ok"),
			   _("The mount point requested is either an illegal "
			     "path or is already in use. Please select a "
			     "valid mount point."));
		    
		    continue;
		}
	    }

	    if (sizeentry) {
		tmpint=strtol(size, &eptr, 10);
		if (eptr != size && *eptr == 0 && tmpint > 0) {
		    tmpint *= SECPERMEG;
		    if (growentry && grow != ' ')
			fdiskSetConstraint(&p->size,0,tmpint,FDISK_SIZE_MAX,1);
		    else
			fdiskSetConstraint(&p->size,0,tmpint,tmpint,1);
		} else {
		    newtWinMessage(_("Size Error"), _("Ok"),
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
		    newtWinMessage(_("Swap Size Error"), _("Ok"),
		       _("You have created a swap partition which is too "
			"large. The maximum size of a swap partition is "
			"%ld Megabytes."), (long)(maxswap / 1024 / 1024));
			continue;
		}
	    }

	    if (driveform) {
		fdiskDeactivateAllDriveSet( &p->drive );
		for (j=0; j<numhd; j++)
		    if (driveentry[j].state == '*')
			fdiskActivateDriveSet( &p->drive, hdarr[j]->num );
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
	    }
	    
	    /* first see if they changed the mount point    */
	    /* we only worry about ext2 and dos partitions  */
	    /* which have a valid mntpt                     */
	    /* LOGIC is not the word for how all this works */
	    if (p->type.current != LINUX_SWAP_PARTITION &&
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

	    fdiskHandleSpecialPartitions( hdarr, numhd, spec );
	    status = FDISK_SUCCESS;
	    done = 1;
	} else {
	    status = FDISK_ERR_USERABORT;
	    done = 1;
	}
    }
    
    newtPopWindow();
    newtFormDestroy(form);

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

static int DeletePartitionSpec( HardDrive **hdarr, unsigned int numhd,
			 PartitionSpec *spec, PartitionSpecEntry *entry,
				int force) {
    if (!force && newtWinChoice(_("Delete Partition"), _("Yes"), _("No"),
			        _("Are you sure you want to delete "
			          "this partition?")) == 2)
	return FDISK_ERR_USERABORT;

    return doDeletePartitionSpec(hdarr, numhd, spec, entry);
}
    

/* used for each line in partbox - tells us what is on that line */
enum partbox_types {PARTBOX_COMMENT, PARTBOX_NFS, PARTBOX_PART};
struct partbox_entry {
    enum partbox_types type;    /* what is on this line */
    int                index;   /* index in nfs or partition arrays */
    int               hilite;   /* element in drive window to hilight */
};

/* simple callback for scrollbox skipping non-entries */
struct partbox_struct {
    unsigned int  len;       /* total entries allocated */
    unsigned int  num;       /* number in use           */
    newtComponent *dbox;     /* drive box */
    struct partbox_entry *entry; /* describes use of this line */
};

/* this is some ugly sh*t, don't try this at home kids */
static void partbox_scroll(newtComponent list, struct partbox_struct *status) {

    static int last=-1;
    static int dontforce=0;
    int sel;
    int i;
    int odir, dir;
    int done;
    int lasttry;

    /* get the index into the partbox_struct array */
    sel = (long) newtListboxGetCurrent(list);

    /* see if this callback occurred because we were forcing */
    /* listbox to scroll                                     */
    if (dontforce) {
	dontforce = 0;
	return;
    }

    /* if the element is ok then just return */
    if (status->entry[sel].type != PARTBOX_COMMENT) {
	if (status->entry[sel].type == PARTBOX_PART &&
	    status->entry[sel].hilite >= 0 && status->dbox != NULL)
	    newtListboxSetCurrent(*status->dbox, status->entry[sel].hilite);
	return;
    }

    /* see which direction we're heading , >0 means down, < 0 means up */
    if (last == -1)
	dir = 1;
    else {
	if (sel > last)
	    dir = 1;
	else
	    dir = -1;
    }

    odir    = dir;
    done    = 0;
    lasttry = 0;
    while (!done) {
	if (dir > 0) {
	    for (i=sel; i < status->num; i++)
		if (status->entry[i].type != PARTBOX_COMMENT)
		    break;

	    if (i!=status->num) {
		dontforce = 1;
		newtListboxSetCurrent(list, i);
		last = i;
		done = 1;

		if (lasttry) {
		    /* scroll to top, since this is last try so original */
		    /* direction was going up                            */
		    dontforce = 1;
		    newtListboxSetCurrent(list, 0);
		    dontforce = 1;
		    newtListboxSetCurrent(list,last);
		}
	    } else {
		if (!lasttry) {
		    dir = -1;
		    lasttry = 1;
		} else {
		    done = 1;
		}
	    }
	} else {
	    for (i=sel; i >= 0; i--)
		if (status->entry[i].type != PARTBOX_COMMENT)
		    break;

	    if (i >= 0) {
		dontforce = 1;
		newtListboxSetCurrent(list, i);
		last = i;
		done = 1;

		if (lasttry) {
		    /* scroll to bottom, since this is last try so original */
		    /* direction was going up                               */
		    dontforce = 1;
		    newtListboxSetCurrent(list, status->num-1);
		    dontforce = 1;
		    newtListboxSetCurrent(list,last);
		}
	    } else {
		if (!lasttry) {
		    dir = 1;
		    lasttry = 1;
		} else {
		    done = 1;
		}
	    }
	}
    }

    /* if we found a valid line then move drive box selection too */
    sel = (long) newtListboxGetCurrent(list);
    if (status->entry[sel].type == PARTBOX_PART &&
	status->entry[sel].hilite >= 0 && status->dbox != NULL)
	newtListboxSetCurrent(*status->dbox, status->entry[sel].hilite);
}


static int MakeDriveBox( HardDrive **hdarr, unsigned int numhd,
			 unsigned int *drvused, int dheight,
			 newtComponent *dbox ) {
    int hdidx, i, per;
    char tmpstr[80];
    
    *dbox = newtListbox( -1, -1, dheight, NEWT_FLAG_SCROLL );
    newtComponentTakesFocus( *dbox, 0 );
    for (hdidx=0; hdidx < numhd; hdidx++) {
	snprintf(tmpstr,sizeof(tmpstr),
		 "   %s      [%5d/%3d/%2d]   "
		 "%6dM %6dM %6dM"
		 "                       ",
		 hdarr[hdidx]->name+5,
		 hdarr[hdidx]->geom.cylinders,
		 hdarr[hdidx]->geom.heads,
		 hdarr[hdidx]->geom.sectors,
		 hdarr[hdidx]->totalsectors/SECPERMEG,
		 drvused[hdidx]/SECPERMEG,
		 hdarr[hdidx]->totalsectors/SECPERMEG-drvused[hdidx]/SECPERMEG
		 );
	
	tmpstr[58]='[';
	tmpstr[69]=']';
	per = (100*drvused[hdidx])/hdarr[hdidx]->totalsectors;
	if (per >= 99)
	    per = 10;
	else
	    per = per/10;
	
	for (i=0; i < per; i++)
	    tmpstr[59+i] = '#';

	tmpstr[74]=0;
	newtListboxAddEntry(*dbox, tmpstr, (void *) 0);
    }

    return FDISK_SUCCESS;
}			 

/* fill in the                                          */
/* position in the status line, up to length characters */
/* if cen=1, center it                                    */
static void BuildTableField( char *line, char *val,
			     int pos, int length, int cen ) {
    char *p, *q;
    int i;
    int c;

    /* lets center field value in the field */
    if (cen) {
	c = strlen(val);
	c = (length-c)/2;
	if (c < 0)
	    c = 0;
    } else
	c=0;

    /* first setup device name */
    for (p=val, q=line+pos+c, i=c; *p && i<length; p++, q++, i++)
	*q = *p;
}

/* given partitoins/hard drives, returns a listbox for use */
/* includes the callback function to skip over headings    */
static int MakePartBox( HardDrive **hdarr, unsigned int numhd,
			PartitionSpec *spec, struct fstab *remotefs,
			int x, int y, int pheight, int dheight,
			newtComponent *list, struct partbox_struct *status,
			newtComponent *dbox) {
    
    newtComponent partbox;

    unsigned int drivenum;
    unsigned int totalused;
    int col;
    int i, k, l, hdidx;
    
    unsigned int listlen;
    unsigned int foundpart;
    char         tmpstr[80];
    int		 num;

    unsigned int *drvused=alloca(numhd*sizeof(unsigned int));

    memset(drvused, 0, numhd*sizeof(unsigned int));
    
    /* check if there are *any* partitions to display */
    for (i = 0, num = 0; i < spec->num; i++) {
	if (fdiskIsExtended(spec->entry[i].partition.type.current))
	    continue;
	num++;
    }
    num += remotefs->numEntries;
    if (!num) {
	*list = NULL;
	MakeDriveBox( hdarr, numhd, drvused, dheight, dbox );
	return FDISK_ERR_BADNUM;
    }

    partbox = newtListbox(-1, -1, pheight, 
			  NEWT_FLAG_RETURNEXIT  | NEWT_FLAG_SCROLL);
    
    listlen = 0;
    status->entry=(struct partbox_entry *)malloc(100*
						 sizeof(struct partbox_entry));
    status->len   = 100;
    memset(status->entry, 0, status->len*sizeof(struct partbox_entry));

    status->dbox = NULL;
    for (hdidx=0; hdidx < numhd; hdidx++) {
	drivenum = hdarr[hdidx]->num;

	/* display all spec'd partitions for this drive */
	foundpart = 0;
	totalused = 0;
	for (i=0; i<spec->num; i++) {
	    unsigned int num, minsize, actsize, drive, totsize;
	    char         statstr[80];
	    char         *pname, *devname;
	    Partition    *p;

	    if (spec->entry[i].status == REQUEST_DENIED)
		continue;
	    
	    p = &spec->entry[i].partition;
	    if ((drive = p->drive.current) != drivenum)
		continue;

	    if (fdiskIsExtended(p->type.current))
		continue;
	    
	    num     = p->num.current;
	    actsize = p->size.current;
	    minsize = p->size.min;
	    pname   = spec->entry[i].name;
	    devname = hdarr[hdidx]->prefix;
	    totsize = hdarr[hdidx]->totalsectors;
	    
	    if (!foundpart)
		foundpart = 1;

	    /* increment amount of space used */
	    totalused += actsize;
	    
	    /* mount point  */
	    col = 3;
	    memset(statstr, ' ', sizeof(statstr));
	    if (strncmp("Exist", pname, 5) && strncmp("Swap", pname, 4) &&
		strncmp("Dos", pname, 3))
		BuildTableField( statstr, pname, col, 16, 0 );
	    
	    /* Block device */
	    snprintf(tmpstr, sizeof(tmpstr), "%s%d", devname, num );
	    col += 22;
	    BuildTableField( statstr, tmpstr, col, 10, 0 );
	    
	    /* Size */
	    snprintf(tmpstr, sizeof(tmpstr), "%5dM", minsize/SECPERMEG);
	    col += 10;
	    BuildTableField( statstr, tmpstr, col, 9, 1 );

	    snprintf(tmpstr, sizeof(tmpstr), "%5dM", actsize/SECPERMEG);
	    col += 10;
	    BuildTableField( statstr, tmpstr, col, 9, 1 );

	    /* we dont want to see all that stuff, just English label */
	    /* for the type                                           */
	    /* Type */
	    col += 13;
	    if (hdarr[hdidx]->part_type == FDISK_PART_TYPE_SUN)
		l = p->type.current | SUNPARTTYPE;
	    else
		l = p->type.current | NONSUNPARTTYPE;
	    
	    for (k=0; k<nparttypes; k++)
		if (allparttypes[k].index == p->type.current ||
		    allparttypes[k].index == l)
		    break;

	    if (k != nparttypes)
		snprintf(tmpstr, sizeof(tmpstr), "%s", allparttypes[k].name);
	    else
		snprintf(tmpstr, sizeof(tmpstr), "0x%02x", p->type.current);
	    
	    BuildTableField( statstr, tmpstr, col, 18, 0);

	    /* now stick it in listbox */
	    statstr[73]=0;
	    status->entry[listlen].type = PARTBOX_PART;
	    status->entry[listlen].index = i;
	    status->entry[listlen].hilite = hdidx;
	    newtListboxAddEntry(partbox, statstr,(void *) (long) listlen);
	    listlen++;
	}

	drvused[hdidx] = totalused;
    }

    for (i=0; i<remotefs->numEntries; i++) {
	char         statstr[80];

	/* mount point  */
	col = 2;
	memset(statstr, ' ', sizeof(statstr));
	BuildTableField( statstr, remotefs->entries[i].mntpoint, col, 16, 0 );
	    
	/* Block device */
	col += 17;
	snprintf(tmpstr, sizeof(tmpstr), "%s:%s",
		 remotefs->entries[i].netHost, remotefs->entries[i].netPath);
	BuildTableField( statstr, tmpstr, col, 40, 0 );
	    
	/* Size */
/* snprintf(tmpstr, sizeof(tmpstr), "%5dM/NA           ", minsize/SECPERMEG);*/
	*tmpstr=0;
	col += 12;
	BuildTableField( statstr, "", col, 15, 0 );

	/* Type */
	col += 27;
	BuildTableField( statstr, "NFS", col, 18, 0);
	
	/* now stick it in listbox */
	statstr[70]=0;
	status->entry[listlen].type  = PARTBOX_NFS;
	status->entry[listlen].index = i;
	newtListboxAddEntry(partbox, statstr,(void *) (long) (listlen));
	listlen++;
    }
	
    
    /* now display any partition specs which WERE NOT allocated */
    foundpart = 0;
    for (i=0; i<spec->num && !foundpart; i++)
	if (spec->entry[i].status == REQUEST_DENIED)
	    foundpart = 1;

    if (foundpart) {
	status->entry[listlen].type = PARTBOX_COMMENT;
	newtListboxAddEntry(partbox,"", (void *)(long)listlen);
	listlen++;
	status->entry[listlen].type = PARTBOX_COMMENT;
	newtListboxAddEntry(partbox,"Unallocated requested partitions",
			    (void *)(long)listlen);
	listlen++;
	status->entry[listlen].type = PARTBOX_COMMENT;
	newtListboxAddEntry(partbox,"--------------------------------",
			    (void *)(long)listlen);
	listlen++;

	for (i=0; i<spec->num; i++) {
	    if (spec->entry[i].status == REQUEST_DENIED) {
		unsigned int num, minsize, actsize;
		char         statstr[80];
		char         *pname;
		Partition    *p;
		
		foundpart = 1;
		p = &spec->entry[i].partition;
		if (fdiskIsExtended(p->type.current))
		    continue;
		
		num     = p->num.current;
		minsize = p->size.min;
		actsize = p->size.current;  
		pname   = spec->entry[i].name;
		
		/* mount point  */
		col = 3;
		memset(statstr, ' ', sizeof(statstr));
		if (strncmp("Exist", pname, 5) && strncmp("Swap", pname, 4) &&
		    strncmp("Dos", pname, 3))
		    BuildTableField( statstr, pname, col, 16, 0 );
	    
		/* Reasons */
		col += 17;
		BuildTableField( statstr, 
				GetReasonString(spec->entry[i].reason),
				col, 25, 0 );
		
		/* Size */
		snprintf(tmpstr, sizeof(tmpstr), "%5dM/NA           ",
			 minsize/SECPERMEG);
		col += 23;
		BuildTableField( statstr, tmpstr, col, 15, 0 );
		
		/* we dont want to see all that stuff, just English label */
		/* for the type                                           */
		/* Type */
		col += 15;

               l = p->type.current | NONSUNPARTTYPE;
               for (hdidx = 0; hdidx < numhd; hdidx++)
                   if (hdarr[hdidx]->num == p->drive.current &&
                       hdarr[hdidx]->part_type == FDISK_PART_TYPE_SUN)
                       l = p->type.current | SUNPARTTYPE;
		for (k=0; k<nparttypes; k++)
		    if (allparttypes[k].index == p->type.current ||
			allparttypes[k].index == l)
			break;
		
		if (k != nparttypes)
		    snprintf(tmpstr, sizeof(tmpstr),"%s",allparttypes[k].name);
		else
		    snprintf(tmpstr, sizeof(tmpstr),"0x%02x", p->type.current);
	    
		BuildTableField( statstr, tmpstr, col, 18, 0);
		
		/* now stick it in listbox */
		statstr[70]=0;
		status->entry[listlen].type  = PARTBOX_PART;
		status->entry[listlen].index = i;
		status->entry[listlen].hilite = -1;
		newtListboxAddEntry(partbox, statstr,(void *)(long)(listlen));
		listlen++;
	    }
	}
    }
    
    /* mark the VERY end of listbox */
    status->num = listlen;
    
    /* setup the callback for the listbox */
    newtComponentAddCallback(partbox, (newtCallback) partbox_scroll, status);
    for (i=0; i<status->num-1;i++)
	if (status->entry[i].type != PARTBOX_COMMENT)
	    break;

    if (i!=status->num)
	newtListboxSetCurrent(partbox,i);

    *list = partbox;

    /* now make the drive box IF desired */
    MakeDriveBox( hdarr, numhd, drvused, dheight, dbox );
    
    if (partbox)
	status->dbox = dbox;

    
    return 0;
}



/* do a operation on a partition */
static int  DoMenuFunction( char *function,
			    HardDrive **orighd, unsigned int numhd,
			    HardDrive **curhd,
			    newtComponent partbox,
			    struct partbox_struct *partbox_status,
			    PartitionSpec *spec ) {
    
    unsigned int sel;
    int          num=0;
    int          status;
    int          i;
    HardDrive    *tmphdarr[MAX_HARDDRIVES];
    
    if (partbox) {
	sel = (long) newtListboxGetCurrent(partbox);
	if (partbox_status->entry[sel].type != PARTBOX_COMMENT)
	    num = partbox_status->entry[sel].index;
	else
	    num = -1;
    } else {
	num = -1;
    }
    
    for (i=0; i<numhd; i++) {
	tmphdarr[i] = (HardDrive *) alloca(sizeof(HardDrive));
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
			    
	return FDISK_SUCCESS;
    } else {
	return FDISK_ERR_BADNUM;
    }
	
}

void
setupGlobalContext(
		FseditContext *state)  {
    /* newt code knows better */
}

/* main loop of the program, builds the display of all drives/partitions */
int StartMaster( FseditContext *state, 
		 struct fstab *remotefs,
		 int dontPartition,
		 int *writeChanges) {
    newtComponent form, add, addnfs, del, edit, reset, ok, cancel, answer;
    newtComponent partbox, curcomponent;
    newtGrid buttons, grid;
    newtComponent dbox, partlabel, drivelabel, driveinfolabel;
    int formdone, totallydone, i, status = 0;
    int currentselection;
    struct newtExitStruct event;
    char tmpstr[80];
    unsigned int width, height;
    int          changesmade=0;
    int retcode = FDISK_SUCCESS; /* i'm optimistic */
    struct partbox_struct partbox_status;
    enum mywidgets {PARTBOX, ADD, ADDNFS, EDIT, DELETE,
		    RESET, OK, CANCEL, NONE};
    enum mywidgets curwidget; 
    HardDrive **hdarr = state->hdarr;
    HardDrive **newhdarr = state->newhdarr;
    HardDrive **prestinehdarr = state->prestinehdarr;
    unsigned int numhd = state->numhd;
    PartitionSpec *spec = &state->spec;

    width  = 78;
    height = 20;

    /* FIXME: hack! */
    addnfs = (void *) 1;
    reset = (void *) 2;

    /* build the main list of installed/probed devices */
    totallydone = 0;
    currentselection = -1;
    curwidget = NONE;

    newtCenteredWindow(width, height, _("Current Disk Partitions"));

    while (!totallydone) {

	/* are there ANY defined partitions ? */
	partlabel = newtLabel(-1, -1,
         "   Mount Point          Device     Requested   Actual         Type");
	MakePartBox(newhdarr, numhd, spec, remotefs,
		    1, 2, height-12, 4,
		    &partbox, &partbox_status, &dbox);

	form = newtForm(NULL, NULL, 0);

	if (dontPartition) {
/*  	    _("Delete"), &del,  */
	    buttons = newtButtonBar(
			_("Edit"), &edit, 
			_("Ok"), &ok, _("Back"), &cancel, NULL);
	    add = reset = NULL;
	    snprintf(tmpstr, sizeof(tmpstr),
		     "                  F3-Edit             "
		     "    F12-Ok   v%5s", VERSION_STR);
	} else {
	    buttons = newtButtonBar(_("Add"), &add, 
			_("Edit"), &edit, _("Delete"), &del,
			_("Ok"), &ok, _("Back"), &cancel, NULL);

	    newtFormAddHotKey(form, NEWT_KEY_F1);
	    newtFormAddHotKey(form, NEWT_KEY_F5);

	    snprintf(tmpstr, sizeof(tmpstr),
		     "    F1-Add                  F3-Edit   "
		     "F4-Delete    F5-Reset    F12-Ok   v%5s", VERSION_STR);
	}

	drivelabel = newtLabel(-1, -1, _("Drive Summaries"));
	driveinfolabel = newtLabel(-1, -1,
	       "  Drive      Geom [C/H/S]      Total    Used    Free");


	grid = newtCreateGrid(1, 6);
	newtGridSetField(grid, 0, 0, NEWT_GRID_COMPONENT, partlabel,
			 0, 0, 0, 0, NEWT_ANCHOR_LEFT, 0);
	if (partbox)
	    newtGridSetField(grid, 0, 1, NEWT_GRID_COMPONENT, partbox,
			     0, 0, 0, 0, NEWT_ANCHOR_LEFT, 0);
	newtGridSetField(grid, 0, 2, NEWT_GRID_COMPONENT, drivelabel,
			 0, partbox ? 0 : 8, 0, 0, NEWT_ANCHOR_LEFT, 0);
	newtGridSetField(grid, 0, 3, NEWT_GRID_COMPONENT, driveinfolabel,
			 0, 0, 0, 0, NEWT_ANCHOR_LEFT, 0);
	newtGridSetField(grid, 0, 4, NEWT_GRID_COMPONENT, dbox,
			 0, 0, 0, 0, 0, 0);
	newtGridSetField(grid, 0, 5, NEWT_GRID_SUBGRID, buttons,
			 0, 1, 0, 0, 0, NEWT_GRID_FLAG_GROWX);

	newtGridPlace(grid, 1, 0);
	newtGridAddComponentsToForm(grid, form, 1);
	newtGridFree(grid, 1);

	/* and the hotkeys */
/*  	newtFormAddHotKey(form, NEWT_KEY_F2); */
	newtFormAddHotKey(form, NEWT_KEY_F3);
	if (!dontPartition) {
	    newtFormAddHotKey(form, NEWT_KEY_F4);
	}

	/* and how to use this plus info on version */
	newtPushHelpLine(tmpstr);
	
	if (partbox) {
	    if (currentselection < 0)
		currentselection = (long) newtListboxGetCurrent(partbox);
	    else
		newtListboxSetCurrent(partbox, currentselection);
	}

	/* see if no partitions are present */
	/* we can't Add if we are in Read Only mode! */
	if (spec->num == 0 && !dontPartition)
	    curwidget = ADD;
	
	/* set current widget if necessary */

	if (partbox != NULL && curwidget == PARTBOX)
	    newtFormSetCurrent(form, partbox);
	else if (curwidget == ADD)
	    newtFormSetCurrent(form, add);
	else if (curwidget == EDIT)
	    newtFormSetCurrent(form, edit);
	else if (curwidget == DELETE)
	    newtFormSetCurrent(form, del);
	
	/* setup main screen */
	formdone = 0;
	answer = NULL;
	while (!formdone) {
	    newtFormRun(form, &event);

	    /* store the current widget so we can reset it */
	    /* if we have to destroy and recreate form     */
	    /* GetCurrent(), SetCurrent() use a ptr to     */
	    /* newtcomponent, so when we recreate form it  */
	    /* will be meaningless!                        */
	    curcomponent = newtFormGetCurrent( form );
	    curwidget = NONE;
	    if (partbox != NULL && curcomponent == partbox)
		curwidget = PARTBOX;
	    else if (curcomponent == add)
		curwidget = ADD;
	    else if (curcomponent == addnfs)
		curwidget = ADDNFS;
	    else if (curcomponent == edit)
		curwidget = EDIT;
	    else if (curcomponent == del)
		curwidget = DELETE;
	    else if (curcomponent == reset)
		curwidget = RESET;
	    else
		curwidget = NONE;
	    
	    if (event.reason == NEWT_EXIT_HOTKEY) {
		event.reason = NEWT_EXIT_COMPONENT;
		if (event.u.key == NEWT_KEY_F12)
		    event.u.co = ok;
		else if (event.u.key == NEWT_KEY_F1)
		    event.u.co = add;
		else if (event.u.key == NEWT_KEY_F2)
		    event.u.co = addnfs;
		else if (event.u.key == NEWT_KEY_F3)
		    event.u.co = edit;
		else if (event.u.key == NEWT_KEY_F4)
		    event.u.co = del;
		else if (event.u.key == NEWT_KEY_F5)
		    event.u.co = reset;
		else
		    continue;
	    }

	    if (event.reason == NEWT_EXIT_COMPONENT) {
		if (event.u.co != partbox) {
		    if (event.u.co == ok || event.u.co == cancel) {
			if (event.u.co == ok) {
			    *writeChanges = 1;
			    /* make sure '/' is defined */
			    status = fdiskIndexPartitionSpec(spec, "/", &i);
			    if (!status)
				if ((spec->entry[i].partition.type.current !=
				    LINUX_NATIVE_PARTITION ||
				    spec->entry[i].status == REQUEST_DENIED) &&
				    !validLoopbackSetup(spec))
				    status = 1;
			    
			    if (status) {
				newtWinMessage(_("No Root Partition"), _("Ok"),
				      _("You must assign a root (/) "
					"partition "
					"to a Linux native partition (ext2) "
					"for the install to proceed."));
				continue;
			    }

			    /* make sure some swapspace is defined */
			    status = 1;
			    for (i=0; i < spec->num; i++)
				if (spec->entry[i].partition.type.current ==
				    LINUX_SWAP_PARTITION &&
				    spec->entry[i].status != REQUEST_DENIED) {
				    status = 0;
				    break;
				}

#if 0
XXX
			    if (status && !expert) {
				newtWinMessage(_("No Swap Partition"), _("Ok"),
					  _("You must assign a swap "
					    "partition "
					    "for the install to proceed."));
				continue;
			    }
#endif
			}

			/* Check to see if there is an active partition */
			status = 0;
			for (i=0; i<spec->num; i++)
			    if (spec->entry[i].partition.active.current)
				status = 1;

			if (!status) {
			    status = fdiskIndexPartitionSpec(spec,"/boot", &i);
			    if (status)
				status = fdiskIndexPartitionSpec(spec,"/", &i);
			    fdiskSetFixedConstraint(&spec->entry[i].partition.active, 1);
			}			    
			      
			/* make sure there are no unallocated partitions */
			status = 0;
			for (i=0; i < spec->num; i++)
			    if (spec->entry[i].status == REQUEST_DENIED) {
				status = 1;
				break;
			    }
			
			if (status) {
			    if (newtWinChoice(_("Unallocated Partitions"),
					      _("Yes"), _("No"),
				   _("There are unallocated partitions "
				     "left. If you quit now they will "
				     "not be written to the disk.\n\n"
				     "Are you sure you want to exit?")) == 2)
			    continue;
			}

			/* dont trust the changes made I keep up with */
			/* above, compute it straight from the hd's   */
			changesmade = DisksChanged( prestinehdarr,
						   newhdarr,
						   numhd );
			/* the dontPartition here isn't necessary, but it
			   does make a reassuring sanity check */
			if (changesmade && !dontPartition) {
			    int rc;
			    rc =  newtWinTernary(_("Save Changes"),
						 _("Yes"), _("No"), _("Cancel"),
						 _("Save changes to "
						   "partition table(s)?"));
			    if (rc == 1) {
				retcode = FDISK_SUCCESS;
				*writeChanges = 1;

				/* copy changes */
				if (changesmade) {
/*				    for (i=0; i<numhd; i++) {
					memcpy(hdarr[i],
					       newhdarr[i],
					       sizeof(HardDrive));
				    }
*/				}
			    } else if (rc == 2) {
				retcode = FDISK_ERR_USERABORT;
				*writeChanges = 0;
			    } else {
				continue;
			    }
			} else {
			    retcode = FDISK_SUCCESS; /* no changes made */
			}

			/* make sure we indicate they canceled */
			if (event.u.co == cancel)
			    retcode = FDISK_ERR_USERABORT;

			formdone = 1;
			totallydone = 1;
		    } else if (event.u.co == add) {
			status=DoMenuFunction("ADD", hdarr, numhd, newhdarr,
				       partbox, &partbox_status, spec );
			if (status == FDISK_SUCCESS) {
			    formdone = 1;
			    changesmade = 1;
			}
		    } else if (event.u.co == del && partbox != NULL) {
			/* see if its NFS or local mount */
			int sel;
			sel = (long) newtListboxGetCurrent(partbox);
			if (partbox_status.entry[sel].type == PARTBOX_PART)
			    if (dontPartition) 
				newtWinMessage(_("Error"), _("Ok"),
					_("You may only delete NFS mounts."));
			    else 
				status=DoMenuFunction("DEL", hdarr, numhd,
						      newhdarr,
						      partbox, &partbox_status,
						      spec );
			else
			/* XXX 
			    status=DeleteNFSMount(remotefs, 
					partbox_status.entry[sel].index);
			*/
			    status = FDISK_ERR_USERABORT;
		    
			if (status == FDISK_SUCCESS) {
			    formdone = 1;
			    changesmade = 1;
			}
		    } else if (event.u.co == edit && partbox != NULL) {
			/* see if its NFS or local mount */
			int sel;
			sel = (long) newtListboxGetCurrent(partbox);
			if (partbox_status.entry[sel].type == PARTBOX_PART)
			    status=DoMenuFunction("EDIT", hdarr, numhd,
						  newhdarr,
						  partbox, &partbox_status,
						  spec );
			else 
			    status = FDISK_ERR_USERABORT;
			    /* XXX
			    status=EditNFSMount(remotefs,
					partbox_status.entry[sel].index,
						intf, netc, dl); */
			
			    
			if (status == FDISK_SUCCESS) {
			    formdone = 1;
			    changesmade = 1;
			}
		    } else if (event.u.co == reset) {
			if (newtWinChoice(_("Reset Partition Table"),
					  _("Yes"), _("No"),
					_("Reset partition table to original "
					  "contents? ")) != 2) {
			    for (i=0; i<numhd; i++)
				memcpy(hdarr[i],prestinehdarr[i],
				       sizeof(HardDrive));

			    fdiskWipePartitionSpec(spec);
			    fdiskSetupPartitionSpec( hdarr, numhd, spec );

			    changesmade = 0;
			    formdone = 1;
			}
		    } else if (event.u.co == addnfs) {
		    /* XXX
			status=EditNFSMount(remotefs, -1, intf, netc, dl);
			*/
			status = FDISK_ERR_USERABORT;

			if (status == FDISK_SUCCESS) {
			    formdone = 1;
			    changesmade = 1;
			}
		    }
		} else {
		    /* see if its NFS or local mount */
		    int sel;
		    sel = (long) newtListboxGetCurrent(partbox);
		    if (partbox_status.entry[sel].type == PARTBOX_PART)
			status=DoMenuFunction("EDIT", hdarr, numhd, newhdarr,
					      partbox, &partbox_status, spec );
		    else
		    	/* XXX 
			status=EditNFSMount(remotefs,
					    partbox_status.entry[sel].index,
					    intf, netc, dl); */
			status = FDISK_ERR_USERABORT;
		    
		    if (status == FDISK_SUCCESS) {
			formdone = 1;
			changesmade = 1;
		    }
		}
	    }

	    /* save current line if leaving */
	    if (partbox)
		currentselection = (long) newtListboxGetCurrent(partbox);
	}

	newtPopHelpLine();
	if (partbox)
	    free(partbox_status.entry);
	newtFormDestroy(form);
    }

    newtPopWindow();

    exitFsedit (state, NULL);
    
    return retcode;
}

void ddruidShowMessage(char * title, char * button, char * text, ...) {
    va_list args;

    va_start(args, text);
    newtWinMessagev(title, button, text, args);
    va_end(args);
}

