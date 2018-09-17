#include "MuplayLinker.h"
#include "MuplaySession.h"
using namespace std;

namespace rr {

  MuplayLinker::MuplayLinker(MuplayElf old_elf,
                             MuplayElf mod_elf,
                             std::shared_ptr<MuplaySession> mu_session)
  : old_elf(old_elf),
    mod_elf(mod_elf),
    mu_session(mu_session)
    { }


} // namespace rr
