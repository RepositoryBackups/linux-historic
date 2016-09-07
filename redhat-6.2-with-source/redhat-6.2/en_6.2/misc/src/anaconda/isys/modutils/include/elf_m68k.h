/* Machine-specific elf macros for m68k.  */
#ident "$Id: elf_m68k.h,v 1.1.1.1 1998/01/06 20:51:07 ewt Exp $"

#define ELFCLASSM	ELFCLASS32
#define ELFDATAM	ELFDATA2MSB

#define MATCH_MACHINE(x)  (x == EM_68K)

#define SHT_RELM	SHT_RELA
#define Elf32_RelM	Elf32_Rela
