/* Combination of diversion and replay session */
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <libunwind.h>
#include "log.h"
#include "MuplaySession.h"
#include "DiversionSession.h"
#include "ReplaySession.h"
#include "ReplayTask.h"
#include "Unwinder.h"
#include "DwarfReader.h"
#include "MuplayLoader.h"
#include "MuplayElfReader.h"
#include "MuplayElf.h"

using namespace std;

namespace rr {

  // MuplaySession::MuplaySession() : emu_fs(EmuFs::create()) {}

  MuplaySession::MuplaySession(const ReplaySession::shr_ptr& replaySession,
                               const string& old_exe,
                               const string& mod_exe)
   : replay_session(replaySession),
     LIVE(false),
     pid(-1),
     old_exe(old_exe),
     mod_exe(mod_exe)
     {
      /* always redirect the stdio of the replay session */
      ReplaySession::Flags flags;
      flags.redirect_stdio = true;
      flags.muplay_enabled = true;
      replay_session->set_flags(flags);
     }

  MuplaySession::~MuplaySession() {
    // We won't permanently leak any OS resources by not ensuring
    // we've cleaned up here, but sessions can be created and
    // destroyed many times, and we don't want to temporarily hog
    // resources.
    // kill_all_tasks();
    // DEBUG_ASSERT(tasks().size() == 0 && vms().size() == 0);
    // DEBUG_ASSERT(emu_fs->size() == 0);
  }

  MuplaySession::shr_ptr MuplaySession::create(const string& trace_dir,
                                               const string& old_exe,
                                               const string& mod_exe)
  {
    ReplaySession::shr_ptr rs(ReplaySession::create(trace_dir));
    shr_ptr session(new MuplaySession(rs, old_exe, mod_exe));

    /* NOTE at some point may need to make the reader
     * the elf file, and the loader part of the class rather
     * than just tools used at startup
     * depends on if we use them again once the diversion is
     * running
     */

    /* Read the elf file and extract the information */
    MuplayElfReader reader(mod_exe);
    MuplayElf mu_elf = reader.read_muplay_elf();
    /* ----------------------- */
    /* Map the patched executable into memory */
    /* ----------------------- */



    return session;
  }


  /**
   * This function will use a replay session from the new trace directory,
   * but will replace the frames with frames from the old trace for as long
   * as that is possible. When the two logs become divergent. The replayer
   * will create a diversion session and will go live.
   * TODO: There needs to be some sort of convergence with the old recorded log at
   * some point. Not sure how to detect when that should be or how to make that happen
   */
  MuplaySession::MuplayResult MuplaySession::muplay_step(RunCommand command)
  {
    MuplaySession::MuplayResult res;
    res.status = MuplaySession::MuplayStatus::MUPLAY_CONTINUE;
    // DiversionSession::shr_ptr ds;
    // Task* t = nullptr;

    if (!LIVE)
    {
      auto result = replay_session->replay_step(command);

      // get the pid of the process under replay
      if (pid == -1)
      {
        /* TODO figure out a way to track if the current process changes
         * this code only checks the pid once
         */
        pid = replay_session->current_task()->tid;
      }
      // check for exit
      if (result.status == REPLAY_EXITED) {
        res.status = MuplaySession::MuplayStatus::MUPLAY_EXITED;
        LOG(debug) << "REPLAY_EXITED\n";
        return res;
      }

      /* doing stack unwinding */
      /* holds value of program counter stack trace */
      std::vector<unw_word_t> pc_st = Unwinder::unwind_pc(pid);

      for (auto pc_val : pc_st)
      {
        //TODO decide if I should use the pc_val or pc_val-1 to get
        // memory address of instruction that triggered the event rather than just
        // the return value
        std::string file_name = get_elf_file(pc_val);
        std::string src_line = DwarfReader::safe_get_src_line(file_name.c_str(), pc_val);
        // LOG(debug) << "safe_src_line_return: " << file_name << " : " << src_line;
        std::string location = file_name + " : " + src_line;
      }
      // LOG(debug) << "-------------";

    } else {
    }
    return res;
  }

  /* Finds the file that is mapped to this virtual address based on what's in
     /proc/id/maps of the tracee process */
  std::string MuplaySession::get_elf_file(unw_word_t mem_address)
  {
    /* Returning empty string on process exit */
    if(replay_session->current_task() == NULL)
    {
      LOG(debug) << "Process exited...get_elf_file returns empty string";
      return "";
    }

    /* Reading the current proc mapping from the AddressSpace */
    /* Need to re init this on each call because mapping changes during replay */
    for (KernelMapIterator it(replay_session->current_task()); !it.at_end(); ++it)
    {
      KernelMapping km = it.current();
      if (km.contains(mem_address))
      {
        // LOG(debug) << "The mem address 0x" << std::hex << mem_address << " is in file: " << km.fsname();
        return km.fsname();
      }
    }

    LOG(debug) << "Couldn't find /proc entry for mem address: " << std::hex << mem_address;
    /* Returns empty string on not found because the Dwarf Reader checks this */
    return "";

  }
} // namespace rr
