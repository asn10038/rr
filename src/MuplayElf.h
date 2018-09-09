#ifndef RR_MUPLAY_ELF_READER_H_
#define RR_MUPLAY_ELF_READER_H_

#include <elf.h>
#include <string>
#include <vector>

namespace rr {

  /*
     Definition that holds all the information necessary from the elf
     for the purposes of loading the code and doing the relocations
   */
  class MuplayElf {
    /*
       For all of these assuming 64 bit operating system
       TODO make this tolerant of 32 bit executables as well
     */

     /* path associated with this elf file -- not needed but might be useful */
     std::string path;

     /* Elf header */
     Elf64_Ehdr elf64_ehdr;

     /* Program headers that are loadable */
     std::vector<Elf64_Phdr> elf64_loadable_segments;

     /* This will need the relocation information */

     /* This will need the symbol information */

  };
}
#endif
