/**
* Ant edit lets see if you can even get this code to execute
*/

/* IDK what imports are actually needed */
#include <sys/time.h>
#include <experimental/filesystem>


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
    " rr muplay [old_tracedir] [new_executable -- has same name as old executable]\n"
  );

/* Copy of ReplayFlags */
  struct MuplayFlags {
    bool some_flag;
    bool dont_launch_debugger;
    MuplayFlags()
        : some_flag(false),
          dont_launch_debugger(true) {}
  };

/** TODO decide if this should be commented or deleted
  static ReplaySession::Flags get_session_flags() {
    ReplaySession::Flags result;
    result.redirect_stdio = true;
    return result;
  }
  */

  /* for executing shell commands
   * copied from https://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
   */

  std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
  }

  /* Replaces the old executable with the new executable in the log */
  /* Assumes the new and the old executable have the same name */
  static string make_new_log(const string& old_trace_dir, const string& new_executable)
  {
    string tmp_string = old_trace_dir;
    printf("%s %s \n", old_trace_dir.c_str(), new_executable.c_str());
    const char* dir = tmp_dir();
    size_t last_slash_idx = tmp_string.find_last_of("/");
    if (std::string::npos != last_slash_idx)
    {
      tmp_string.erase(0, last_slash_idx + 1);
    }
    string new_trace_dir = dir + tmp_string;
    /* TODO figure out how to check for errors */
    string cmd = "cp -r " + old_trace_dir + " " + dir;
    /* copy the old trace to the tmp directory */
    exec(cmd.c_str());

    /* Replace the executable in the old log */
    printf("Executed copy command\n");



    return new_trace_dir;
  }
  static void serve_muplay_no_debugger(const string& old_trace_dir,
                                       const string& new_executable,
                                       const MuplayFlags& flags)
  {
    /* Make the new trace with the modified executable */
    make_new_log(old_trace_dir, new_executable);

    if (flags.dont_launch_debugger)
      printf("don't launch debugger\n");

    // uint32_t step_count = 0;
    struct timeval last_dump_time;
    Session::Statistics last_stats;
    gettimeofday(&last_dump_time, NULL);
    /* Beginning to moving things out of this class as appropriate */
    MuplaySession::shr_ptr muplay_session = MuplaySession::create(old_trace_dir,
                                                                  new_executable);
    printf("Doing muplay replay\n");
    while(true) {
      RunCommand cmd = RUN_CONTINUE;
      auto res = muplay_session->muplay_step(cmd);
      if (res.status == MuplaySession::MuplayStatus::MUPLAY_EXITED)
        break;
    }
  }


  static void muplay(const string& old_trace_dir, const string& new_trace_dir,
                     const MuplayFlags& flags, const vector<string>& args,
                     FILE* out) {
      if (!args.empty())
        fprintf(out, "Args are not empty\n");

      serve_muplay_no_debugger(old_trace_dir, new_trace_dir, flags);

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
