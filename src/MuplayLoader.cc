#include <sys/types.h> /* For open() */
#include <sys/stat.h>  /* For open() */
#include <fcntl.h>     /* For open() */
#include <stdlib.h>     /* For exit() */
#include <unistd.h>     /* For close() */


#include "log.h"
#include "ElfReader.h"
#include "MuplayLoader.h"
#include "MemoryRange.h"


using namespace std;

namespace rr {
  MuplayLoader::MuplayLoader(std::string mod_exe_path, Task* t)
  : mod_exe_path(mod_exe_path),
    t(t)
  { }


  std::vector<MemoryRange> MuplayLoader::find_empty_pages()
  {
    /* read the address space */
    std::vector<MemoryRange> res;
    KernelMapping prev;
    bool started = false;
    for (KernelMapIterator it(t); !it.at_end(); ++it)
    {
      KernelMapping km = it.current();
      if(!started)
      {
        started = true;
        prev = km;
        continue;
      }
      int space = km.start().as_int() - prev.end().as_int();

      if(space < 0) {
        //TODO deal with why this is happening. Something with an
        // int long problem
      }
      if(space > 0)
      {
        MemoryRange mem_range(prev.end().as_int(), space);
        res.push_back(mem_range);
      }
      prev = km;
    }
    return res;
  }

  /* TODO add arguments as appropriate */
  long MuplayLoader::load()
  {
    std::vector<MemoryRange> open_pages = find_empty_pages();
    AutoRemoteSyscalls remote(t);
    SupportedArch arch = t->arch();

    MemoryRange code_result;
    remote_ptr<void> addr;

    string path = mod_exe_path;
    AutoRestoreMem child_path(remote, path.c_str());
    int child_fd = remote.syscall(syscall_number_for_open(arch),
                     child_path.get(), O_RDONLY);
    if(child_fd < 0)
      FATAL() << "Open failed with errno " << errno_name(-child_fd);

    //TODO need to modify the mmap call to specific addresses
    code_result = MemoryRange(remote.infallible_mmap_syscall(remote_ptr<void>(),
                                                             4096, //TODO assuming one page
                                                             PROT_READ | PROT_EXEC,
                                                             MAP_PRIVATE,
                                                             child_fd,
                                                             0
                                                             //4096*1024 //TODO find how many pages the offset is
                                                           ), 4096);
    LOG(debug) << "Mapped the new code into: " << std::hex << code_result.start() << " - " << code_result.end();
    /* Checking the values at the address */
    long addr1 = code_result.start().as_int()+1332;
    long result = ptrace(PTRACE_PEEKDATA, t->tid, addr1);
    LOG(debug) << "Mov instruction to jump to: " << std::hex << addr1 << " : " << result;
    /* Mmapping the new data from the executable */
    /* --assuming this is done */
    /* in this case I know they range from 0x4005ef-0x400609*/
    /* --assuming this is done */


    /* TODO figure out how to close file after reading in target process */
    // close(mod_fd);
    /* */

    return code_result.start().as_int();
  }
}
