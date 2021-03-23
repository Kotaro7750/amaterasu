#ifndef _ELF_H_
#define _ELF_H_

#define EI_NIDENT 16

#define EI_MAG0 0    /* File identification byte 0 index */
#define ELFMAG0 0x7f /* Magic number byte 0 */

#define EI_MAG1 1   /* File identification byte 1 index */
#define ELFMAG1 'E' /* Magic number byte 1 */

#define EI_MAG2 2   /* File identification byte 2 index */
#define ELFMAG2 'L' /* Magic number byte 2 */

#define EI_MAG3 3   /* File identification byte 3 index */
#define ELFMAG3 'F' /* Magic number byte 3 */

#define EI_CLASS 4     /* File class byte index */
#define ELFCLASSNONE 0 /* Invalid class */
#define ELFCLASS32 1   /* 32-bit objects */
#define ELFCLASS64 2   /* 64-bit objects */
#define ELFCLASSNUM 3

#define EI_DATA 5     /* Data encoding byte index */
#define ELFDATANONE 0 /* Invalid data encoding */
#define ELFDATA2LSB 1 /* 2's complement, little endian */
#define ELFDATA2MSB 2 /* 2's complement, big endian */
#define ELFDATANUM 3

#define EI_VERSION 6 /* File version byte index */
#define EV_NONE 0    /* Invalid ELF version */
#define EV_CURRENT 1 /* Current version */
#define EV_NUM 2

#define EI_OSABI 7                  /* OS ABI identification */
#define ELFOSABI_NONE 0             /* UNIX System V ABI */
#define ELFOSABI_SYSV 0             /* Alias.  */
#define ELFOSABI_HPUX 1             /* HP-UX */
#define ELFOSABI_NETBSD 2           /* NetBSD.  */
#define ELFOSABI_GNU 3              /* Object uses GNU ELF extensions.  */
#define ELFOSABI_LINUX ELFOSABI_GNU /* Compatibility alias.  */
#define ELFOSABI_SOLARIS 6          /* Sun Solaris.  */
#define ELFOSABI_AIX 7              /* IBM AIX.  */
#define ELFOSABI_IRIX 8             /* SGI Irix.  */
#define ELFOSABI_FREEBSD 9          /* FreeBSD.  */
#define ELFOSABI_TRU64 10           /* Compaq TRU64 UNIX.  */
#define ELFOSABI_MODESTO 11         /* Novell Modesto.  */
#define ELFOSABI_OPENBSD 12         /* OpenBSD.  */
#define ELFOSABI_ARM_AEABI 64       /* ARM EABI */
#define ELFOSABI_ARM 97             /* ARM */
#define ELFOSABI_STANDALONE 255     /* Standalone (embedded) application */

#define PT_NULL 0                  /* Program header table entry unused */
#define PT_LOAD 1                  /* Loadable program segment */
#define PT_DYNAMIC 2               /* Dynamic linking information */
#define PT_INTERP 3                /* Program interpreter */
#define PT_NOTE 4                  /* Auxiliary information */
#define PT_SHLIB 5                 /* Reserved */
#define PT_PHDR 6                  /* Entry for header table itself */
#define PT_TLS 7                   /* Thread-local storage segment */
#define PT_NUM 8                   /* Number of defined types */
#define PT_LOOS 0x60000000         /* Start of OS-specific */
#define PT_GNU_EH_FRAME 0x6474e550 /* GCC .eh_frame_hdr segment */
#define PT_GNU_STACK 0x6474e551    /* Indicates stack executability */
#define PT_GNU_RELRO 0x6474e552    /* Read-only after relocation */
#define PT_LOSUNW 0x6ffffffa
#define PT_SUNWBSS 0x6ffffffa   /* Sun Specific segment */
#define PT_SUNWSTACK 0x6ffffffb /* Stack segment */
#define PT_HISUNW 0x6fffffff
#define PT_HIOS 0x6fffffff   /* End of OS-specific */
#define PT_LOPROC 0x70000000 /* Start of processor-specific */
#define PT_HIPROC 0x7fffffff /* End of processor-specific */

typedef unsigned short Elf64_Half;
typedef unsigned int Elf64_Word;
typedef unsigned long long Elf64_Addr;
typedef unsigned long long Elf64_Off;
typedef unsigned long long Elf64_Xword;

struct Elf64_Ehdr {
  unsigned char e_ident[EI_NIDENT]; /* Magic number and other info */
  Elf64_Half e_type;                /* Object file type */
  Elf64_Half e_machine;             /* Architecture */
  Elf64_Word e_version;             /* Object file version */
  Elf64_Addr e_entry;               /* Entry point virtual address */
  Elf64_Off e_phoff;                /* Program header table file offset */
  Elf64_Off e_shoff;                /* Section header table file offset */
  Elf64_Word e_flags;               /* Processor-specific flags */
  Elf64_Half e_ehsize;              /* ELF header size in bytes */
  Elf64_Half e_phentsize;           /* Program header table entry size */
  Elf64_Half e_phnum;               /* Program header table entry count */
  Elf64_Half e_shentsize;           /* Section header table entry size */
  Elf64_Half e_shnum;               /* Section header table entry count */
  Elf64_Half e_shstrndx;            /* Section header string table index */
};

struct Elf64_Phdr {
  Elf64_Word p_type;    /* Segment type */
  Elf64_Word p_flags;   /* Segment flags */
  Elf64_Off p_offset;   /* Segment file offset */
  Elf64_Addr p_vaddr;   /* Segment virtual address */
  Elf64_Addr p_paddr;   /* Segment physical address */
  Elf64_Xword p_filesz; /* Segment size in file */
  Elf64_Xword p_memsz;  /* Segment size in memory */
  Elf64_Xword p_align;  /* Segment alignment */
};

struct Elf64_Shdr {
  Elf64_Word sh_name;       /* Section name (string tbl index) */
  Elf64_Word sh_type;       /* Section type */
  Elf64_Xword sh_flags;     /* Section flags */
  Elf64_Addr sh_addr;       /* Section virtual addr at execution */
  Elf64_Off sh_offset;      /* Section file offset */
  Elf64_Xword sh_size;      /* Section size in bytes */
  Elf64_Word sh_link;       /* Link to another section */
  Elf64_Word sh_info;       /* Additional section information */
  Elf64_Xword sh_addralign; /* Section alignment */
  Elf64_Xword sh_entsize;   /* Entry size if section holds table */
};

#endif
