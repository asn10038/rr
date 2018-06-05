/* Combination of diversion and replay session */

#include "MuplaySession.h"
#include "DiversionSession.h"
#include "ReplaySession.h"

using namespace std;

namespace rr {

  MuplaySession::MuplaySession() : emu_fs(EmuFs::create()) {}

  MuplaySession::MuplaySession(ReplaySession& replaySession) :
    emu_fs(EmuFs::create()),
    replay_session(replaySession.clone()) {}

MuplaySession::~MuplaySession() {
    // We won't permanently leak any OS resources by not ensuring
    // we've cleaned up here, but sessions can be created and
    // destroyed many times, and we don't want to temporarily hog
    // resources.
    kill_all_tasks();
    DEBUG_ASSERT(tasks().size() == 0 && vms().size() == 0);
    DEBUG_ASSERT(emu_fs->size() == 0);
  }

} // namespace rr
