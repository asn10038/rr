/* Combination of diversion and replay session */

#include "MuplaySession.h"
#include "DiversionSession.h"
#include "ReplaySession.h"

using namespace std;

namespace rr {

  MuplaySession::MuplaySession() : emu_fs(EmuFs::create()) {}

  MuplaySession::MuplaySession(ReplaySession& replaySession,
                               const string old_trace_dir) :
    emu_fs(EmuFs::create()),
    replay_session(replaySession.clone()),
    old_trace_reader(new TraceReader(old_trace_dir))
    {
      /* always redirect the stdio of the replay session */
      ReplaySession::Flags flags;
      flags.redirect_stdio = true;
      replay_session->set_flags(flags);
    }

  MuplaySession::~MuplaySession() {
    // We won't permanently leak any OS resources by not ensuring
    // we've cleaned up here, but sessions can be created and
    // destroyed many times, and we don't want to temporarily hog
    // resources.
    kill_all_tasks();
    DEBUG_ASSERT(tasks().size() == 0 && vms().size() == 0);
    DEBUG_ASSERT(emu_fs->size() == 0);
  }

  MuplaySession::shr_ptr MuplaySession::create(const string& old_trace_dir,
                               const string& new_trace_dir)
  {
    ReplaySession::shr_ptr replay_session = (ReplaySession::create(new_trace_dir));

    shr_ptr muplay_session(new MuplaySession(*replay_session, old_trace_dir));

    return muplay_session;
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
    bool LIVE = false;
    MuplaySession::MuplayResult res;
    /* Replaying from old log with new code */
    if (!LIVE)
    {

      // 2) Replay from the old frame rather than the current frame


    } else {
      /* Create diversion session here and go live */
    }
    if(command == RUN_SINGLESTEP)
      printf("command");

    return res;
  }

} // namespace rr
