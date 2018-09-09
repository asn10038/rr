#ifndef RR_MUPLAY_ELF_READER_H_
#define RR_MUPLAY_ELF_READER_H_

#include <string>
#include <elf.h>
#include <vector>

namespace rr {
  /* Uses libelf to read the elf file and extract information that we need */
  class MuplayElfReader {
  public:
    /* Constructors */
    MuplayElfReader(std::string elf_pat);

    /* attributes */
    std::string elf_path;

    /* Functions */

    /* Fills the elf64_hdr field -- elf header defined in /usr/include/elf.h */
    Elf64_Ehdr read_elf_header();

    /* Fills the elf64_\ field -- elf header defined in /usr/include/elf.h */
    std::vector<Elf64_Phdr> read_loadable_segments(Elf64_Off phoff, Elf64_Half e_phnum);




  };
}
#endif
