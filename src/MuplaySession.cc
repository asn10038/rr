/* Combination of diversion and replay session */
#include <sys/ptrace.h>
#include <sys/wait.h>

#include "log.h"
#include "MuplaySession.h"
#include "DiversionSession.h"
#include "ReplaySession.h"
#include "ReplayTask.h"
using namespace std;

namespace rr {

  MuplaySession::MuplaySession() : emu_fs(EmuFs::create()) {}

  MuplaySession::MuplaySession(ReplaySession& replaySession) :
    emu_fs(EmuFs::create()),
    replay_session(replaySession.clone()),
    LIVE(false)
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
   */
  MuplaySession::MuplayResult MuplaySession::muplay_step(RunCommand command)
  {
    MuplaySession::MuplayResult res;
    res.status = MuplaySession::MuplayStatus::MUPLAY_CONTINUE;
    Task* task;

    if (!LIVE)
    {
      // LOG(debug) << "going to replay frame num:" << replay_session->current_trace_frame().time() << "\n";
      // auto result = replay_session->replay_step(command);
      // if (result.status == REPLAY_EXITED) {
      //   res.status = MuplaySession::MuplayStatus::MUPLAY_EXITED;
      //   LOG(debug) << "REPLAY_EXITED\n";
      // } else if (result.status == GOING_LIVE)
      // {
      //   LOG(debug) << "REALIZED YOU HAVE TO GO LIVE!!!\n";
      //   res.status = MUPLAY_LIVE;
      //   LIVE = true;
      //   diversion_session = replay_session->clone_diversion();
      //   task = diversion_session -> find_task(replay_session->current_task()->tuid());
      //   count = 0;
      // }
      task = replay_session->current_task();

      int ws;
      int count = 0;
      bool start = true;
      while (start || WIFSTOPPED(ws) || WIFSIGNALED(ws)) {

        if(start)
          start = false;
        // struct user_regs_struct tregs;
        // task->ptrace_if_alive(PTRACE_GETREGS, 0, (void*)(uintptr_t)&tregs);
        if(task->status().stop_sig())
          printf("PROCESS IS STOPPED\n");
        else
          printf("PROCESS IS NOT STOPPED\n");
        // ptrace(PTRACE_GETREGS, task->tid, 0 ,&tregs);
        printf("The task being traced is: %i\n", task->tid);
        printf("The recorded tid is: %i\n", task->rec_tid);
        // unsigned instr = task->ptrace_if_alive(PTRACE_PEEKTEXT, tregs.rip, 0);

        // printf("icount = %i, RIP=0x%llx, instr=0x%08x\n", count, tregs.rip, instr);

        if(task->ptrace_if_alive(PTRACE_SINGLESTEP, nullptr, (void*)(uintptr_t) 0) < 0)
        {
          printf("ERROR in PTRACE SINGLESTEP\n");
          break;
        }
        wait(&ws);
        // printf("ws is: %i\n", ws);

        if(WIFEXITED(ws))
          printf("WIFEXITED\n");
        if(WIFSIGNALED(ws))
          printf("WIFSIGNALED\n");
        if(WCOREDUMP(ws))
          printf("WCOREDUMP\n");
        if(WIFSTOPPED(ws)) {
          count++;
          printf("WIFSTOPPED by: %i\n", WSTOPSIG(ws));
        }

        if(WIFCONTINUED(ws))
          printf("WCONTINUED\n");
        // count ++;
        if(count == 150)
          start = false;
        if (WSTOPSIG(ws) == 11)
          break;
      }
      printf("COUNT IS: %i", count);

      // for(int i=0; i<20; i++) {
      //   printf("calling ptrace_if_alive\n");
      //   bool err = task->ptrace_if_alive(RESUME_CONT, nullptr, (void*)(uintptr_t) 0);
      //
      //   if (!err)
      //     printf("Error returned from ptrace_if_alive\n");
      // }
      res.status = MUPLAY_EXITED;
    }
    if(LIVE)
    {
      LOG(debug) << "Entered the live if statement\n";
      /* TODO set task to class variable so you don't reset it each time */

      task = diversion_session -> find_task(replay_session->current_task()->tuid());
      auto result = diversion_session->diversion_step(task, command);
      LOG(debug) << "Successfully called diversion_step()\n";
      if (result.status == DiversionSession::DIVERSION_EXITED)
      {
        res.status = MuplaySession::MuplayStatus::MUPLAY_EXITED;
        LOG(debug)<<"GOING LIVE EXITED";
      }
      count++;
      if (count > 3) {
        LOG(debug) << "count is breaking the loop\n";
        res.status = MuplaySession::MuplayStatus::MUPLAY_EXITED;
      }
    }

    return res;
  }

} // namespace rr
