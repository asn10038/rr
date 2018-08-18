#include <sys/types.h> /* For open() */
#include <sys/stat.h>  /* For open() */
#include <fcntl.h>     /* For open() */
#include <stdlib.h>     /* For exit() */
#include <unistd.h>     /* For close() */


#include "log.h"
#include "ElfReader.h"
#include "MuplayLoader.h"



using namespace std;

namespace rr {
  MuplayLoader::MuplayLoader(std::string mod_exe_path, Task* t)
  : mod_exe_path(mod_exe_path),
    t(t)
  { }

  void MuplayLoader::load()
  {
    AutoRemoteSyscalls remote(t);

    /* HOW IN THE HELL DO I DO THIS *???*/
    /* open the modified exe */
    ScopedFd mod_fd = open(mod_exe_path.c_str(), O_RDONLY);
    if (mod_fd < 0) { FATAL() << "Unable to open mod_exe at: " << mod_exe_path; }

    /* Using the elf reader to determine what to map into the address space */
    ElfFileReader elf_reader(mod_fd);
    LOG(debug) << "READING THE MODIFIED EXECUTABLE";
    SectionOffsets text_offset = elf_reader.find_section_file_offsets(".text");
    
    LOG(debug) << "The offet of the .text section is: 0x" << std::hex  << text_offset.start;

    /* Mapping the executable into the address space of the target process */

    /* Need to map the code section in as executable and readable */
    /* Need to map the global data section as Readable and Writable */
    /* OTHER SECTIONS HERE? */
  }
}
