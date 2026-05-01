#ifndef ELF_H
#define ELF_H

#include <stdint.h>
#include <stdbool.h>
#include <os_setjmp.h>

// three structs from linux foundation
typedef struct
{
    unsigned char ident[16];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} elf_header_t;

typedef struct
{
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint32_t addr;
    uint32_t offset;
    uint32_t size;
    uint32_t link;
    uint32_t info;
    uint32_t addralign;
    uint32_t entsize;
} elf_sh_t;

typedef struct
{
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t filesz;
    uint32_t memsz;
    uint32_t flags;
    uint32_t align;
} elf_ph_t;

// important data for actually runnning it
typedef struct
{
    uint32_t entry_fileOff; // offset of entry within file. full offset
    uint32_t text_offset;
    uint32_t text_size;
    uint32_t data_offset;
    uint32_t data_size; // includes data, rodata, bss
    uint8_t *file;
    uint32_t file_size;
} elf_section_offsets_t;

enum
{
    SHT_NULL,
    SHT_PROGBITS,
    SHT_SYMTAB,
    SHT_STRTAB,
    SHT_RELA,
    SHT_HASH,
    SHT_DYNAMIC,
    SHT_NOTE,
    SHT_NOBITS,
    SHT_REL,
    SHT_SHLIB,
    SHT_DYNSYM,
    SHT_INIT_ARRAY,
    SHT_FINI_ARRAY,
    SHT_PREINIT_ARRAY,
    SHT_GROUP,
    SHT_SYMTAB_SHNDX,
};

#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MERGE 0x10
#define SHF_STRINGS 0x20
#define SHF_INFO_LINK 0x40
#define SHF_LINK_ORDER 0x80
#define SHF_OS_NONCONFORMING 0x100
#define SHF_GROUP 0x200
#define SHF_TLS 0x400
#define SHF_MASKOS 0x0ff00000
#define SHF_MASKPROC 0xf0000000

#define CHECK_TYPE(e, _type) (e->type == _type)
#define CHECK_FLAGS(e, _flags) ((e->flags ^ (_flags)) == 0)
/*
XOR places 1 in any bit where that flag is wrong.
If all right ->  0x0000000
if some wrong -> 0x0001000
If number = 0, then it was correct
*/
#define IS_BSS(e) ((e->size != 0) && (CHECK_TYPE(e, SHT_NOBITS) && (CHECK_FLAGS(e, SHF_ALLOC + SHF_WRITE))))
#define IS_DATA(e) ((e->size != 0) && (CHECK_TYPE(e, SHT_PROGBITS) && (CHECK_FLAGS(e, SHF_ALLOC + SHF_WRITE))))
#define IS_RODATA(e) ((e->size != 0) && (CHECK_TYPE(e, SHT_PROGBITS) && (CHECK_FLAGS(e, SHF_ALLOC))))
#define IS_TEXT(e) ((e->size != 0) && (CHECK_TYPE(e, SHT_PROGBITS) && (CHECK_FLAGS(e, SHF_ALLOC + SHF_EXECINSTR))))
// #define IS_RELTEXT(e) ((e->size != 0) && (CHECK_TYPE(e, SHT_REL)      && (CHECK_FLAGS(e, SHF_ALLOC))))

#define PT_NULL 0U
#define PT_LOAD 1U
#define PT_DYNAMIC 2U
#define PT_INTERP 3U

#define PF_X 0x1U // Execute
#define PF_W 0x2U // Write
#define PF_R 0x4U // Read

#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5

#define ELFMAG0 0x7FU
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'
#define ELFCLASS32 1U  // 32-bit
#define ELFDATA2LSB 2U // LE
#define ET_EXEC 2U     // Executable file
#define EM_386 3U      // x86

typedef enum
{
    ELF_OK = 1,
    ELF_ERR_MAGIC,
    ELF_ERR_CLASS,
    ELF_ERR_TYPE,
    ELF_ERR_MACHINE,
    ELF_ERR_NOPHDR,
} elf_error_t;

typedef struct
{
    uint8_t* buffer;
    uint32_t size;
} elf_loaded_file_t;

#define user_stack_size 4096
extern uint8_t user_stack_glob[user_stack_size] __attribute__((aligned(user_stack_size)));

elf_error_t elf_load(const uint8_t *file_bytes, uint32_t file_size, elf_section_offsets_t *out);
void elf_unload(const elf_section_offsets_t *info);
elf_error_t elf_exec(const uint8_t *file_bytes, uint32_t file_size, uint8_t *user_stack, uint32_t stack_size, uint32_t argc, char** argv);

extern uint8_t  exec_pending;
extern uint8_t *exec_pending_file;
extern uint32_t exec_pending_size;
extern uint32_t exec_pending_argc;
extern char   **exec_pending_argv;
extern bool exec_free_buffer; 

#endif