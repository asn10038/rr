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
     mod_exe(mod_exe),
     /* TODO remove this hardcoded diversion point */
     diversion_point("/home/ant/asn10038_rr/traces/hello_world-0/mmap_hardlink_3_hello_world : 6")
    // new_trace_reader(new TraceReader(new_trace_dir))
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
        LOG(debug) << "safe_src_line_return: " << file_name << " : " << src_line;
        std::string location = file_name + " : " + src_line;
        /* checking for diversion */
        if(location == diversion_point)
        {
          LOG(debug) << "DIVERSION POINT DETECTED";
          scratch_clone();
          res.status = MuplaySession::MuplayStatus::MUPLAY_EXITED;
          return res;
        }
      }
      LOG(debug) << "-------------";

    } else {
        // auto result = replay_session->replay_step(command);
        //
        //
        // if (result.status == REPLAY_EXITED) {
        //   res.status = MuplaySession::MuplayStatus::MUPLAY_EXITED;
        //   LOG(debug) << "REPLAY_EXITED\n";
        // }
        // else {
        //
        //   auto div_res = ds->diversion_step(t, RUN_SINGLESTEP, 0);
        //   if (div_res.status == DiversionSession::DiversionStatus::DIVERSION_EXITED)
        //   {
        //     res.status = MUPLAY_EXITED;
        //   }
        // }
        FATAL() << "Entered LIVE mode. Shouldn't be here yet";
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

  /* DEBUG this function is just for checking what might happen when you hit diversion point*/
  void MuplaySession::scratch_clone(){
    // virtual Task* clone(CloneReason reason, int flags, remote_ptr<void> stack,
    //                     remote_ptr<void> tls, remote_ptr<int> cleartid_addr,
    //                     pid_t new_tid, pid_t new_rec_tid, uint32_t new_serial,
    //                     Session* other_session = nullptr);
    //
    // Task* cloned_task;
    // cloned_task = clone(Task::CloneReason::)

    DiversionSession::shr_ptr ds = replay_session->clone_diversion();

    Task* t = replay_session->current_task();
    int count = 0;
    /* GOING TO USE PTRACE TO CHANGE THE VALUE OF THE STRING IN MEMORY */
    // long addr = 0x4005e0;
    // long addr2 = 0x4005e1;
    // long addr3 = 0x4005e2;
    // char b = ' ';
    // char c = '2';
    // char d = '!';
    // ptrace(PTRACE_POKEDATA, pid, addr, b);
    // ptrace(PTRACE_POKEDATA, pid, addr2, c);
    // ptrace(PTRACE_POKEDATA, pid, addr3, d);
    /*  ---------- end of ptrace mods -------- */

    /* Look for addresses that need to be loaded from modified executable */
    DwarfReader::get_lineno_addrs(mod_exe.c_str(), 6);
    /*------------------------------------------------------------------- */

    /* calling the MuplayLoader to load the modified code into memory */
    MuplayLoader mu_loader(mod_exe, t);
    mu_loader.load();
    /* --- loaded modified code --- */
    while(1) {
        ds->diversion_step(t, RUN_CONTINUE, 0);
        count++;
        if (count > 10)
          break;
    }

    // LOG(debug) << "In the diversion session the current task is: " << ds->current_task();
    return;
  }
} // namespace rr
