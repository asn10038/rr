#ifndef RR_MUPLAY_LINKER_H_
#define RR_MUPLAY_LINKER_H_

#include <memory>
#include "MuplayElf.h"
#include "MuplaySession.h"

namespace rr {



  /* Once the new code has been loaded need to link it in the
     context of the original executable. This means both
     the relocation entries and the global variable references */

  class MuplayLinker{
  public:
    /* Constructors */
    MuplayLinker(rr::MuplayElf old_elf,
                 rr::MuplayElf mod_elf,
                 std::shared_ptr<MuplaySession> mu_session);

    /* Fields */
    MuplayElf old_elf; /* The exe that is recorded */
    MuplayElf mod_elf; /* exe with the modifications */
    std::shared_ptr<MuplaySession> mu_session; /* the current session with the load information*/

    /* Functions */

  };

} // namespace rr
#endif
