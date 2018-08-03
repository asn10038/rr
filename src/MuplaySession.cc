/* Combination of diversion and replay session */
#include <sys/ptrace.h>
#include <sys/wait.h>

#include "log.h"
#include "MuplaySession.h"
#include "DiversionSession.h"
#include "ReplaySession.h"
#include "ReplayTask.h"
#include "libunwind-ptrace.h"
using namespace std;

namespace rr {

  // MuplaySession::MuplaySession() : emu_fs(EmuFs::create()) {}

  MuplaySession::MuplaySession(const ReplaySession::shr_ptr& replaySession)
   : replay_session(replaySession),
     LIVE(false),
     pid(-1)
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

  MuplaySession::shr_ptr MuplaySession::create(const string& trace_dir)
  {
    ReplaySession::shr_ptr rs(ReplaySession::create(trace_dir));
    shr_ptr session(new MuplaySession(rs));
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
        /* TODO figure out a way to track if the current process changes */
        pid = replay_session->current_task()->tid;
      }
      // check for exit
      if (result.status == REPLAY_EXITED) {
        res.status = MuplaySession::MuplayStatus::MUPLAY_EXITED;
        LOG(debug) << "REPLAY_EXITED\n";
        return res;
      }


      /* doing stack unwinding */
      unw_addr_space_t as = unw_create_addr_space(&_UPT_accessors, 0);


      void *context = _UPT_create(pid);
      unw_cursor_t cursor;
      if (unw_init_remote(&cursor, as, context) != 0)
      {
        LOG(debug) << "Couldn't initialize for remote unwinding -- process probably exited";
      }

      do {
        unw_word_t offset, pc;
        char sym[4096];
        if (unw_get_reg(&cursor, UNW_REG_IP, &pc))
          LOG(debug) << "ERROR: cannot read program counter\n";

        printf("0x%lx: ", pc);

        if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0)
          printf("(%s+0x%lx)\n", sym, offset);
        else
          printf("\n");
      } while (unw_step(&cursor) > 0);
      printf("-----------------\n");
_UPT_destroy(context);

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
