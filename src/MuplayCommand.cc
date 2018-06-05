/**
* Ant edit lets see if you can even get this code to execute
*/

/* IDK what imports are actually needed */
#include <sys/time.h>

#include "Command.h"
#include "TraceStream.h"
#include "core.h"
#include "main.h"
#include "util.h"
#include "log.h"

#include "MuplayEventMatcher.h"
#include "ReplaySession.h"
#include "ReplayCommand.h"
#include "DiversionSession.h"
#include "ReplayTask.h"
// #include "MuplaySession.h"
/* ------------------------ */

using namespace std;

namespace rr {

  class MuplayCommand : public Command {
  public:
    virtual int run(vector<string>& args) override;

  protected:
    MuplayCommand(const char* name, const char* help) : Command(name, help) {}

    static MuplayCommand singleton;
  };

  MuplayCommand MuplayCommand::singleton(
    "muplay",
    " rr muplay [old_tracedir] [new_tracedir]\n"
  );

/* Copy of ReplayFlags */
  struct MuplayFlags {
    bool some_flag;
    bool dont_launch_debugger;
    MuplayFlags()
        : some_flag(false),
          dont_launch_debugger(true) {}
  };


  static ReplaySession::Flags get_session_flags() {
    ReplaySession::Flags result;
    result.redirect_stdio = true;
    return result;
  }
  static void serve_muplay_no_debugger(const string& trace_dir,
                                       const MuplayFlags& flags,
                                       vector<TraceFrame>& muTrace)
  {
    //Do some analysis with muTrace to detect divergence at some point
    if (muTrace.size() > 0)
      {}
    //signals switch to turn in to live execution
    bool LIVE = false;

    ReplaySession::shr_ptr replay_session = ReplaySession::create(trace_dir);
    replay_session->set_flags(get_session_flags());

    if (flags.dont_launch_debugger)
      printf("don't launch debugger\n");

    uint32_t step_count = 0;
    struct timeval last_dump_time;
    Session::Statistics last_stats;
    gettimeofday(&last_dump_time, NULL);

    printf("Doing personal replay\n");
    int LIVE_FRAME = 158;
    DiversionSession::shr_ptr diversion_session = replay_session->clone_diversion();
    Task* task = diversion_session->find_task(replay_session->current_task()->tuid());
    while (true) {
      RunCommand cmd = RUN_CONTINUE;

      /* TODO Figure out how to refactor this if statement with inheritance */
      if (!LIVE) {
        FrameTime before_time = replay_session->trace_reader().time();
        if (before_time == LIVE_FRAME)
        {
          LIVE = true;
          diversion_session = replay_session->clone_diversion();
          task = diversion_session->find_task(replay_session->current_task()->tuid());
          continue;
        }
        auto result = replay_session->replay_step(cmd);
        FrameTime after_time = replay_session->trace_reader().time();
        DEBUG_ASSERT(after_time >= before_time && after_time <= before_time + 1);
        ++step_count;
        if (result.status == REPLAY_EXITED)
          break;
      } else {
        printf("You have diverged \n");
        auto result = diversion_session->diversion_step(task, cmd, 0);
        if (result.status == DiversionSession::DIVERSION_EXITED)
          break;
      }
    }
  }


  static void muplay(const string& old_trace_dir, const string& new_trace_dir,
                     const MuplayFlags& flags, const vector<string>& args,
                     FILE* out) {
      if (!args.empty())
        fprintf(out, "Args are not empty\n");

      TraceReader oldTrace(old_trace_dir);
      TraceReader newTrace(new_trace_dir);

      // READ THE TRACES
      vector<TraceFrame> oldFrames, newFrames;
      while (!oldTrace.at_end())
      {
        // printf("You are advancing the reader\n");
        TraceFrame oldTraceFrame = oldTrace.read_frame();
        oldFrames.push_back(oldTraceFrame);
        // oldTraceFrame.dump(out);
      }
      while(!newTrace.at_end())
      {
        TraceFrame newTraceFrame = newTrace.read_frame();
        newFrames.push_back(newTraceFrame);
      }

      if (flags.some_flag) {}

      MuplayEventMatcher muMatcher(oldFrames, newFrames);
      vector<TraceFrame> muTrace = muMatcher.combineTraces();
      // printf("You combined the traces to %lu events\n", muTrace.size());
      // printf("Old trace was %lu events\n", muMatcher.oldFrames.size());
      // printf("New trace was %lu events\n", muMatcher.newFrames.size());
      /* Trigger this modified replay */
      serve_muplay_no_debugger(new_trace_dir, flags, muTrace);

  }

  int MuplayCommand::run(vector<string>& args) {
    MuplayFlags flags;
    // // TODO CODE TO PARSE OPTIONS

    string old_trace_dir, new_trace_dir;
    if (!parse_optional_trace_dir(args, &old_trace_dir))
    {
      printf("Problem parsing the old_trace_dir");
      return 1;
    }
    if (!parse_optional_trace_dir(args, &new_trace_dir))
    {
      printf("Problem parsing the new_trace_dir");
      return 1;
    }

    if(args.size() > 0) {};

    printf("about to go to muplay\n");
    muplay(old_trace_dir, new_trace_dir, flags, args, stdout);
    return 0;
  }
} // namespace rr
