#ifndef RR_MUPLAY_LOADER_H
#define RR_MUPLAY_LOADER_H

#include <string>
#include <elf.h>

#include "ReplaySession.h"
#include "AutoRemoteSyscalls.h"
#include "MuplayElf.h"

namespace rr {
  /* Loads the modified executable into the target process address space
   * in the future will do some other things like relocating etc. but
   * for now just trying to load the modified executable and maybe change the
   * ip to point to a spot in the new  code based on DWARF info
   */
   class MuplayLoader {
   public:
     /* MuplayElf object holds all information for loading */
     MuplayElf mu_elf;

     /* The target task that the loader puts the code into */
     Task* t;

     /* Constructor with object holding all information from elf
        and the target task */
     MuplayLoader(MuplayElf mu_elf, Task* t);

     /* load the modified exe into the target task address space */
     /* returns the load address in the target process */
     long load();

     /* Finds empty pages in the address space (reads /proc/maps)
        to load the patched code needs to make sure that it loads
        the page within 32 bits of the space where the original code
        is loaded. For now assuming the lowest free pages will do the
        trick */
     std::vector<MemoryRange> find_empty_pages();

     /*
        Given a program header struct load the segment into the nearest
        available page range. Only load into pages that are greater than
        intended virtual address so that relocations can be done simply by adding
        a base address
        TODO base addresses need to be stored somewhere. This probably needs
        to return this information back to the Muplay Session to manage this
       */
     void load_into_nearest_page_range(Elf64_Phdr phdr);

     /* Finds the closest memory page range for a given program segment */
     MemoryRange find_closest_page_range(Elf64_Phdr phdr);

   };

} //namespace rr

#endif
