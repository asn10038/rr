/**
* Ant edit lets see if you can even get this code to execute
*/

/* IDK what imports are actually needed */
#include <inttypes.h>

#include <limits>
#include <unordered_map>

#include "preload/preload_interface.h"

#include "AddressSpace.h"
#include "Command.h"
#include "TraceStream.h"
#include "core.h"
#include "kernel_metadata.h"
#include "log.h"
#include "main.h"
#include "util.h"

#include "MuplayEventMatcher.h"
#include "ReplaySession.h"
#include "MuplaySession.h"
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

  struct MuplayFlags {
    bool some_flag;

    MuplayFlags()
    : some_flag(false) {}
  };


  static void muplay(const string& old_trace_dir, const string& new_trace_dir,
                     const MuplayFlags& flags, const vector<string>& args,
                     FILE* out) {
      if (flags.some_flag)
        fprintf(out, "some_flag is true");
      while (!args.empty())
        fprintf(out, "Args are not empty");

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

      MuplayEventMatcher muMatcher(oldFrames, newFrames);
      vector<TraceFrame> muTrace = muMatcher.combineTraces();
      printf("You combined the traces to %lu events\n", muTrace.size());
      printf("Old trace was %lu events\n", muMatcher.oldFrames.size());
      printf("New trace was %lu events\n", muMatcher.newFrames.size());
      /* Trigger this modified replay */
      ReplaySession::shr_ptr replay_session = ReplaySession::create(new_trace_dir);
      MuplaySession::shr_ptr muplay_session = MuplaySession::create(replay_session);

  }

  int MuplayCommand::run(vector<string>& args) {
    MuplayFlags flags;
    // TODO CODE TO PARSE OPTIONS

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

    muplay(old_trace_dir, new_trace_dir, flags, args, stdout);
    return 0;
  }
} // namespace rr
