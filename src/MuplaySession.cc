/* Combination of diversion and replay session */

#include "MuplaySession.h"
#include "DiversionSession.h"
#include "ReplaySession.h"

using namespace std;

namespace rr {

  MuplaySession::MuplaySession() : emu_fs(EmuFs::create()) {}

  MuplaySession::MuplaySession(ReplaySession& replaySession) :
    emu_fs(EmuFs::create()),
    replay_session(replaySession.clone())
    // new_trace_reader(new TraceReader(new_trace_dir))
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

  MuplaySession::shr_ptr MuplaySession::create(const string& trace_dir)
  {
    ReplaySession::shr_ptr replay_session = (ReplaySession::create(trace_dir));

    shr_ptr muplay_session(new MuplaySession(*replay_session));

    return muplay_session;
  }


  /**
   * This function will use a replay session from the new trace directory,
   * but will replace the frames with frames from the old trace for as long
   * as that is possible. When the two logs become divergent. The replayer
   * will create a diversion session and will go live.
   * TODO: There needs to be some sort of convergence with the old recorded log at
   * some point. Not sure how to detect when that should be or how to make that happen
   * TODO: Figure out when appropriate to swap frames. There are some things that are
   * specific to the new replay event.
   */
  MuplaySession::MuplayResult MuplaySession::muplay_step(RunCommand command)
  {
    bool LIVE = false;
    MuplaySession::MuplayResult res;
    res.status = MuplaySession::MuplayStatus::MUPLAY_CONTINUE;
    /* Replaying from old log trying to sub in new executable */
    if (!LIVE)
    {
      printf("going to replay frame num: %lu\n", replay_session->current_trace_frame().time());
      auto result = replay_session->replay_step(command);
      if (result.status == REPLAY_EXITED) {
        res.status = MuplaySession::MuplayStatus::MUPLAY_EXITED;
        printf("REPLAY_EXITED\n");
      }


    } else {
      /* Create diversion session here and go live */
    }

    return res;
  }

} // namespace rr
