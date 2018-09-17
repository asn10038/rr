/* Combination of diversion and replay session */
#ifndef RR_MUPLAY_SESSION_H_
#define RR_MUPLAY_SESSION_H_


#include "EmuFs.h"
#include "Session.h"
#include "ReplaySession.h"
#include "Unwinder.h"
#include "MuplayElfReader.h"
#include "MuplayLoader.h"
#include "MuplayElf.h"

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
    static shr_ptr create(const std::string& trace_dir,
                          const std::string& old_exe,
                          const std::string& mod_exe);
    /* TODO only accomodates RUN_CONTINUE */
    MuplayResult muplay_step(RunCommand command);

    /* Find the file associated with a given virtual address in the tracee process */
    std::string get_elf_file(unw_word_t mem_address);

    /* DEBUG this is just to test what happens if you clone a new process */
    void scratch_clone();

    /* Returns if you've already loaded the modified executable or not */
    bool has_done_modified_load() { return done_modified_load; };

    /* loads the modified elf into the tracee address space */
    /* also fills in the actual_load_addresses field of this class */
    /* and modifies the done_modified_load field */
    /* NOTE this may not be the best design decision */
    void load_modified_elf();

    /* Links the modified elf into the context of the original
       elf after the modified elf has been loaded into the
       address space
        NOTE this may not be the best design decision */
    void link_modified_elf();


  private:
    friend class ReplaySession;
    friend class DiversionSession;

    MuplaySession();
    MuplaySession(const ReplaySession::shr_ptr& replaySession,
                  const std::string& old_exe,
                  const std::string& mod_exe);

    ReplaySession::shr_ptr replay_session;
    DiversionSession::shr_ptr diversion_session;

    bool LIVE;
    /* TODO remove this variable */
    int count;

    /* pid of the current task under observation */
    pid_t pid;

    /* Paths to the original and the changed binaries */
    const std::string old_exe;
    const std::string mod_exe;

    /* Where the loadable segments were loaded */
    /* TODO turn this into a map of Elf64_Phdr segments to memory return_addresses
       need to come up with a hash function for Elf64_Phdr to do this */
    std::vector<MemoryRange> actual_load_addresses;

    /* Know if you have already loaded the modified executable */
    bool done_modified_load;
  };

} // namespace rr

#endif
