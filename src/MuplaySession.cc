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
    DiversionSession::shr_ptr ds;
    Task* t = nullptr;

    if (!LIVE)
    {
      // LOG(debug) << "going to replay frame num:" << replay_session->current_trace_frame().time() << "\n";
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

      /* Call libdwarf stuff */

      /* End of libdwarf stuff */
      /* doing stack unwinding */

      /* holds value of program counter stack trace */
      std::vector<unw_word_t> pc_st = Unwinder::unwind_pc(pid);


      for (auto pc_val : pc_st)
      {
        LOG(debug) << "0x" << pc_val;
        std::string src_line = DwarfReader::get_src_line(old_exe.c_str(), pc_val);
      }
      LOG(debug) << "_-------------";

      /* The unwinding has happened */

    } else {
        auto result = replay_session->replay_step(command);
        /* using libunwind to take a look at the stack at the place
           of the event */

        if (result.status == REPLAY_EXITED) {
          res.status = MuplaySession::MuplayStatus::MUPLAY_EXITED;
          LOG(debug) << "REPLAY_EXITED\n";
        }
        else {

          auto div_res = ds->diversion_step(t, RUN_SINGLESTEP, 0);
          if (div_res.status == DiversionSession::DiversionStatus::DIVERSION_EXITED)
          {
            res.status = MUPLAY_EXITED;
          }
        }



    }



    return res;
  }

} // namespace rr
