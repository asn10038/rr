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


  static void muplay(const string& old_tracedir, const string& new_tracedir,
                     const MuplayFlags& flags, const vector<string>& args,
                     FILE* out) {
      if (flags.some_flag)
        printf("some_flag is true");
      while (!args.empty())
        printf("Args are not empty");
      TraceReader oldTrace(old_tracedir);
      TraceReader newTrace(new_tracedir);
      fprintf(out,"Going to read the old trace dir log");
      int oldCount, newCount = 0;
      while (!oldTrace.at_end())
      {
        // printf("You are advancing the reader\n");
        oldCount++;
        TraceFrame oldTraceFrame = oldTrace.read_frame();
        oldTraceFrame.dump(out);
      }
      while(!newTrace.at_end())
      {
        newCount++;
        TraceFrame newTraceFrame = newTrace.read_frame();
      }
      printf("Old trace count: %i \t : \t New Trace count: %i\n", oldCount, newCount);
  }

  int MuplayCommand::run(vector<string>& args) {
    MuplayFlags flags;
    // TODO CODE TO PARSE OPTIONS

    string old_trace_dir, new_trace_dir;
    if (!parse_optional_trace_dir(args, &new_trace_dir))
    {
      printf("Problem parsing the old_trace_dir");
      return 1;
    }
    if (!parse_optional_trace_dir(args, &old_trace_dir))
    {
      printf("Problem parsing the old_trace_dir");
      return 1;
    }

    muplay(old_trace_dir, new_trace_dir, flags, args, stdout);
    return 0;
  }
} // namespace rr
