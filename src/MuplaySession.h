/* Combination of diversion and replay session */
#ifndef RR_MUPLAY_SESSION_H_
#define RR_MUPLAY_SESSION_H_


#include "EmuFs.h"
#include "Session.h"
#include "ReplaySession.h"


namespace rr {

  class ReplaySession;
  class DiversionSession;

  /* A muplay session is a combination of a replay session and a diversion
   * session. It goes live and diverges when appropriate, but will converge and
   * read from the log as well just as a replay session would
   */

  class MuplaySession {
  public:
    typedef std::shared_ptr<MuplaySession> shr_ptr;

    ~MuplaySession();

    // EmuFs& emufs() const { return *emu_fs; }

    // virtual MuplaySession* as_muplay() override { return this; }


    enum MuplayStatus {
      // continue replay muplay_step() can be called again
      MUPLAY_CONTINUE,
      /* Tracees are dead don't call muplay_step() again */
      MUPLAY_EXITED,
      /* Some code was executed.Reached Divergent state */
      MUPLAY_LIVE
    };

    struct MuplayResult {
      MuplayStatus status;
      BreakStatus break_status;
    };

    /**
     * Create a muplay session from the modified log
     * essentially a replay session with a swapped executable in the log
     */
    static shr_ptr create(const std::string& trace_dir);
    /* TODO only accomodates RUN_CONTINUE */
    MuplayResult muplay_step(RunCommand command);

  private:
    friend class ReplaySession;
    friend class DiversionSession;

    MuplaySession();
    MuplaySession(const ReplaySession::shr_ptr& replaySession);

    /* TODO Figure out what these args should be */
    MuplayResult muplay_step();

    ReplaySession::shr_ptr replay_session;
    DiversionSession::shr_ptr diversion_session;

    bool LIVE;
    /* TODO remove this variable */
    int count;

    /* pid of the current task under observation */
    pid_t pid;
  };

} // namespace rr

#endif
