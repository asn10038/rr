#ifndef RR_MUPLAY_LOADER_H
#define RR_MUPLAY_LOADER_H

#include <string>

#include "ReplaySession.h"
#include "AutoRemoteSyscalls.h"

namespace rr {
  /* Loads the modified executable into the target process address space
   * in the future will do some other things like relocating etc. but
   * for now just trying to load the modified executable and maybe change the
   * ip to point to a spot in the new  code based on DWARF info
   */
   class MuplayLoader {
   public:
     /* file path to the modified executable */
     std::string mod_exe_path;

     /* The target task that the loader puts the code into */
     Task* t;

     /* Constructor with path to mod exe and the target task */
     MuplayLoader(std::string mod_exe_path, Task* t);

     /* load the modified exe into the target task address space */
     /* returns the load address in the target process */
     long load();

     /* Finds empty pages in the address space (reads /proc/maps)
        to load the patched code needs to make sure that it loads
        the page within 32 bits of the space where the original code
        is loaded. For now assuming the lowest free pages will do the
        trick */
     std::vector<MemoryRange> find_empty_pages();

   };

} //namespace rr

#endif
