/* Combination of diversion and replay session */
#ifndef RR_MUPLAY_SESSION_H_
#define RR_MUPLAY_SESSION_H_


#include "EmuFs.h"
#include "Session.h"


namespace rr {

  class ReplaySession;
  class DiversionSession;

  /* A muplay session is a combination of a replay session and a diversion
   * session. It goes live and diverges when appropriate, but will converge and
   * read from the log as well just as a replay session would
   */

  class MuplaySession : public Session {
  public:
    typedef std::shared_ptr<MuplaySession> shr_ptr;

    ~MuplaySession();

    EmuFs& emufs() const { return *emu_fs; }

    virtual MuplaySession* as_muplay() override { return this; }

  private:
    friend class ReplaySession;
    friend class DiversionSession;

    MuplaySession();
    MuplaySession(ReplaySession& replaySession);

    std::shared_ptr<EmuFs> emu_fs;
    std::shared_ptr<ReplaySession> replay_session;
    std::shared_ptr<DiversionSession> diversion_session;
  };

} // namespace rr

#endif
