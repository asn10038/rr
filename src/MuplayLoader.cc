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

  /* TODO add arguments as appropriate */
  long MuplayLoader::load()
  {
    AutoRemoteSyscalls remote(t);
    SupportedArch arch = t->arch();
    /* Mmapping the instructions from the new executable */
    /* in this case I know they range from 0x400534-0x40053e in the new executable */
    MemoryRange code_result;
    remote_ptr<void> addr;
    // size_t length;
    // int prot;
    // int flags;
    string path = mod_exe_path;
    AutoRestoreMem child_path(remote, path.c_str());
    int child_fd = remote.syscall(syscall_number_for_open(arch),
                     child_path.get(), O_RDONLY);
    if(child_fd < 0)
      FATAL() << "Open failed with errno " << errno_name(-child_fd);

    //TODO need to modify the mmap call to specific addresses
    code_result = MemoryRange(remote.infallible_mmap_syscall(remote_ptr<void>(),
                                                             734, //TODO assuming one page
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

    /* Moving the ip...this shouldn't be done here */
    auto regs = t->regs();
    LOG(debug) << "current ip: " << regs.ip();
    long ip_val = ptrace(PTRACE_PEEKTEXT, t->tid, regs.ip());
    LOG(debug) << "value at current ip: " << ip_val;
    regs.set_ip(addr1);
    t->set_regs(regs);
    regs = t->regs();

    long new_ip_val = ptrace(PTRACE_PEEKTEXT, t->tid, regs.ip());
    LOG(debug) << "new ip { " << std::hex << regs.ip() << " : " << new_ip_val << " }";


    /* TODO figure out how to close file after reading in target process */
    // close(mod_fd);
    /* */

    return code_result.start().as_int();
  }
}
