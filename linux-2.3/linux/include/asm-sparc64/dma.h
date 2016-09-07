/* $Id: dma.h,v 1.13 1998/12/16 04:33:55 davem Exp $
 * include/asm-sparc64/dma.h
 *
 * Copyright 1996 (C) David S. Miller (davem@caip.rutgers.edu)
 */

#ifndef _ASM_SPARC64_DMA_H
#define _ASM_SPARC64_DMA_H

#include <linux/kernel.h>
#include <linux/types.h>

#include <asm/sbus.h>
#include <asm/delay.h>
#include <asm/oplib.h>
#include <asm/spinlock.h>

extern spinlock_t  dma_spin_lock;

#define claim_dma_lock() \
({	unsigned long flags; \
	spin_lock_irqsave(&dma_spin_lock, flags); \
	flags; \
})

#define release_dma_lock(__flags) \
	spin_unlock_irqrestore(&dma_spin_lock, __flags);

/* These are irrelevant for Sparc DMA, but we leave it in so that
 * things can compile.
 */
#define MAX_DMA_CHANNELS 8
#define DMA_MODE_READ    1
#define DMA_MODE_WRITE   2

/* This is actually used. */
extern unsigned long phys_base;
#define MAX_DMA_ADDRESS  (phys_base + (0xfe000000UL) + PAGE_OFFSET)

/* Useful constants */
#define SIZE_16MB      (16*1024*1024)
#define SIZE_64K       (64*1024)

/* Structure to describe the current status of DMA registers on the Sparc */
struct sparc_dma_registers {
  __volatile__ __u32 cond_reg;	/* DMA condition register */
  __volatile__ __u32 st_addr;	/* Start address of this transfer */
  __volatile__ __u32 cnt;	/* How many bytes to transfer */
  __volatile__ __u32 dma_test;	/* DMA test register */
};

/* DVMA chip revisions */
enum dvma_rev {
	dvmarev0,
	dvmaesc1,
	dvmarev1,
	dvmarev2,
	dvmarev3,
	dvmarevplus,
	dvmahme
};

#define DMA_HASCOUNT(rev)  ((rev)==dvmaesc1)

/* Linux DMA information structure, filled during probe. */
struct Linux_SBus_DMA {
	struct Linux_SBus_DMA *next;
	struct linux_sbus_device *SBus_dev;
	struct sparc_dma_registers *regs;

	/* Status, misc info */
	int node;                /* Prom node for this DMA device */
	int running;             /* Are we doing DMA now? */
	int allocated;           /* Are we "owned" by anyone yet? */

	/* Transfer information. */
	unsigned int addr;       /* Start address of current transfer */
	int nbytes;              /* Size of current transfer */
	int realbytes;           /* For splitting up large transfers, etc. */

	/* DMA revision */
	enum dvma_rev revision;
};

extern struct Linux_SBus_DMA *dma_chain;

/* Broken hardware... */
#define DMA_ISBROKEN(dma)    ((dma)->revision == dvmarev1)
#define DMA_ISESC1(dma)      ((dma)->revision == dvmaesc1)

/* Main routines in dma.c */
extern void dvma_init(struct linux_sbus *);

/* Fields in the cond_reg register */
/* First, the version identification bits */
#define DMA_DEVICE_ID    0xf0000000        /* Device identification bits */
#define DMA_VERS0        0x00000000        /* Sunray DMA version */
#define DMA_ESCV1        0x40000000        /* DMA ESC Version 1 */
#define DMA_VERS1        0x80000000        /* DMA rev 1 */
#define DMA_VERS2        0xa0000000        /* DMA rev 2 */
#define DMA_VERHME       0xb0000000        /* DMA hme gate array */
#define DMA_VERSPLUS     0x90000000        /* DMA rev 1 PLUS */

#define DMA_HNDL_INTR    0x00000001        /* An IRQ needs to be handled */
#define DMA_HNDL_ERROR   0x00000002        /* We need to take an error */
#define DMA_FIFO_ISDRAIN 0x0000000c        /* The DMA FIFO is draining */
#define DMA_INT_ENAB     0x00000010        /* Turn on interrupts */
#define DMA_FIFO_INV     0x00000020        /* Invalidate the FIFO */
#define DMA_ACC_SZ_ERR   0x00000040        /* The access size was bad */
#define DMA_FIFO_STDRAIN 0x00000040        /* DMA_VERS1 Drain the FIFO */
#define DMA_RST_SCSI     0x00000080        /* Reset the SCSI controller */
#define DMA_RST_ENET     DMA_RST_SCSI      /* Reset the ENET controller */
#define DMA_ST_WRITE     0x00000100        /* write from device to memory */
#define DMA_ENABLE       0x00000200        /* Fire up DMA, handle requests */
#define DMA_PEND_READ    0x00000400        /* DMA_VERS1/0/PLUS Pending Read */
#define DMA_ESC_BURST    0x00000800        /* 1=16byte 0=32byte */
#define DMA_READ_AHEAD   0x00001800        /* DMA read ahead partial longword */
#define DMA_DSBL_RD_DRN  0x00001000        /* No EC drain on slave reads */
#define DMA_BCNT_ENAB    0x00002000        /* If on, use the byte counter */
#define DMA_TERM_CNTR    0x00004000        /* Terminal counter */
#define DMA_SCSI_SBUS64  0x00008000        /* HME: Enable 64-bit SBUS mode. */
#define DMA_CSR_DISAB    0x00010000        /* No FIFO drains during csr */
#define DMA_SCSI_DISAB   0x00020000        /* No FIFO drains during reg */
#define DMA_DSBL_WR_INV  0x00020000        /* No EC inval. on slave writes */
#define DMA_ADD_ENABLE   0x00040000        /* Special ESC DVMA optimization */
#define DMA_E_BURST8	 0x00040000	   /* ENET: SBUS r/w burst size */
#define DMA_BRST_SZ      0x000c0000        /* SCSI: SBUS r/w burst size */
#define DMA_BRST64       0x000c0000        /* SCSI: 64byte bursts (HME on UltraSparc only) */
#define DMA_BRST32       0x00040000        /* SCSI: 32byte bursts */
#define DMA_BRST16       0x00000000        /* SCSI: 16byte bursts */
#define DMA_BRST0        0x00080000        /* SCSI: no bursts (non-HME gate arrays) */
#define DMA_ADDR_DISAB   0x00100000        /* No FIFO drains during addr */
#define DMA_2CLKS        0x00200000        /* Each transfer = 2 clock ticks */
#define DMA_3CLKS        0x00400000        /* Each transfer = 3 clock ticks */
#define DMA_EN_ENETAUI   DMA_3CLKS         /* Put lance into AUI-cable mode */
#define DMA_CNTR_DISAB   0x00800000        /* No IRQ when DMA_TERM_CNTR set */
#define DMA_AUTO_NADDR   0x01000000        /* Use "auto nxt addr" feature */
#define DMA_SCSI_ON      0x02000000        /* Enable SCSI dma */
#define DMA_PARITY_OFF   0x02000000        /* HME: disable parity checking */
#define DMA_LOADED_ADDR  0x04000000        /* Address has been loaded */
#define DMA_LOADED_NADDR 0x08000000        /* Next address has been loaded */
#define DMA_RESET_FAS366 0x08000000        /* HME: Assert RESET to FAS366 */

/* Values describing the burst-size property from the PROM */
#define DMA_BURST1       0x01
#define DMA_BURST2       0x02
#define DMA_BURST4       0x04
#define DMA_BURST8       0x08
#define DMA_BURST16      0x10
#define DMA_BURST32      0x20
#define DMA_BURST64      0x40
#define DMA_BURSTBITS    0x7f

/* Determine highest possible final transfer address given a base */
#define DMA_MAXEND(addr) (0x01000000UL-(((unsigned long)(addr))&0x00ffffffUL))

/* Yes, I hack a lot of elisp in my spare time... */
#define DMA_ERROR_P(regs)  ((((regs)->cond_reg) & DMA_HNDL_ERROR))
#define DMA_IRQ_P(regs)    ((((regs)->cond_reg) & (DMA_HNDL_INTR | DMA_HNDL_ERROR)))
#define DMA_WRITE_P(regs)  ((((regs)->cond_reg) & DMA_ST_WRITE))
#define DMA_OFF(regs)      ((((regs)->cond_reg) &= (~DMA_ENABLE)))
#define DMA_INTSOFF(regs)  ((((regs)->cond_reg) &= (~DMA_INT_ENAB)))
#define DMA_INTSON(regs)   ((((regs)->cond_reg) |= (DMA_INT_ENAB)))
#define DMA_PUNTFIFO(regs) ((((regs)->cond_reg) |= DMA_FIFO_INV))
#define DMA_SETSTART(regs, addr)  ((((regs)->st_addr) = (char *) addr))
#define DMA_BEGINDMA_W(regs) \
        ((((regs)->cond_reg |= (DMA_ST_WRITE|DMA_ENABLE|DMA_INT_ENAB))))
#define DMA_BEGINDMA_R(regs) \
        ((((regs)->cond_reg |= ((DMA_ENABLE|DMA_INT_ENAB)&(~DMA_ST_WRITE)))))

/* For certain DMA chips, we need to disable ints upon irq entry
 * and turn them back on when we are done.  So in any ESP interrupt
 * handler you *must* call DMA_IRQ_ENTRY upon entry and DMA_IRQ_EXIT
 * when leaving the handler.  You have been warned...
 */
#define DMA_IRQ_ENTRY(dma, dregs) do { \
        if(DMA_ISBROKEN(dma)) DMA_INTSOFF(dregs); \
   } while (0)

#define DMA_IRQ_EXIT(dma, dregs) do { \
	if(DMA_ISBROKEN(dma)) DMA_INTSON(dregs); \
   } while(0)

#define for_each_dvma(dma) \
        for((dma) = dma_chain; (dma); (dma) = (dma)->next)

extern int get_dma_list(char *);
extern int request_dma(unsigned int, __const__ char *);
extern void free_dma(unsigned int);

#endif /* !(_ASM_SPARC64_DMA_H) */