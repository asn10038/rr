/**
* Ant edit lets see if you can even get this code to execute
*/

/* IDK what imports are actually needed */
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>


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
    " rr muplay [old_tracedir] [old_executable] [new_executable] \n"
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

  // std::string exec(const char* cmd) {
  //   std::array<char, 128> buffer;
  //   std::string result;
  //   std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
  //   if (!pipe) throw std::runtime_error("popen() failed!");
  //   while (!feof(pipe.get())) {
  //       if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
  //           result += buffer.data();
  //   }
  //   return result;
  // }
  //
  // static string remove_filename_from_path(const string& path)
  // {
  //   string tmp_string(path);
  //   size_t last_slash_idx = tmp_string.find_last_of("/");
  //   /* remove trailing slash if there is one */
  //   if (last_slash_idx == tmp_string.length()-1)
  //   {
  //     tmp_string.erase(tmp_string.length()-1);
  //     last_slash_idx = tmp_string.find_last_of("/");
  //   }
  //   if (std::string::npos != last_slash_idx)
  //   {
  //     tmp_string.erase(0, last_slash_idx + 1);
  //   }
  //   return tmp_string;
  // }

  /* Replaces the old executable with the new executable in the log */
  // static string make_new_log(const string& old_trace_dir, const string& old_executable,
  //                            const string& new_executable)
  // {
  //   string tmp_string = old_trace_dir;
  //   const char* temp_dir = tmp_dir();
  //   string trace_name = remove_filename_from_path(old_trace_dir);
  //   string new_trace_dir = temp_dir + string("/") + trace_name;
  //   string cmd = "cp -r " + old_trace_dir + " " + temp_dir;
  //   /* TODO figure out how to check for errors */
  //   /* copy the old trace to the tmp directory */
  //   exec(cmd.c_str());
  //
  //   /* Find and remove old hardlink executable in the old log */
  //   printf("the new trace dir: %s\n", new_trace_dir.c_str());
  //   DIR *dir = opendir(new_trace_dir.c_str());
  //   if(dir)
  //   {
  //     struct dirent *ent;
  //     string old_executable_name = remove_filename_from_path(old_executable);
  //     while((ent = readdir(dir)) != NULL)
  //     {
  //       string name = string(ent->d_name);
  //       /* assuming the new executable has the same name as the old one */
  //       if (name.find(old_executable_name) != std::string::npos)
  //       {
  //         /* removed old hardlinked executable */
  //         string file_to_replace = new_trace_dir + string("/") + name;
  //         cmd = "rm " + file_to_replace;
  //         exec(cmd.c_str());
  //         /* copy new executable to log with correct name */
  //         cmd = "cp " + new_executable + " " + file_to_replace;
  //         exec(cmd.c_str());
  //       }
  //     }
  //   } else {
  //     printf("Error opening new_trace_dir: %s\n", new_trace_dir.c_str());
  //     exit(1);
  //   }
  //
  //   return new_trace_dir;
  // }

  static void serve_muplay_no_debugger(const string& old_trace_dir,
                                       const string& old_executable,
                                       const string& new_executable,
                                       const MuplayFlags& flags)
  {
    /* Commenting this out for exeperimenting with libunwind Make the new trace with the modified executable */
    // string new_trace_dir = make_new_log(old_trace_dir, old_executable, new_executable);
    if(old_executable.size() > new_executable.size()) {}

    /* TODO check and actual flags here */
    if (flags.dont_launch_debugger)
      {}


    // MuplaySession::shr_ptr muplay_session = MuplaySession::create(new_trace_dir);
    // if(new_trace_dir.size()){}


    MuplaySession::shr_ptr muplay_session = MuplaySession::create(old_trace_dir,
                                                                  old_executable,
                                                                  new_executable);

    printf("Doing muplay replay\n");
    while(true) {
      RunCommand cmd = RUN_CONTINUE;
      auto res = muplay_session->muplay_step(cmd);
      if (res.status == MuplaySession::MuplayStatus::MUPLAY_EXITED)
        break;
    }
  }


  static void muplay(const string& old_trace_dir, const string& old_executable,
                     const string& new_executable, const MuplayFlags& flags,
                     const vector<string>& args, FILE* out) {
      if (!args.empty())
        fprintf(out, "Args are not empty\n");

      serve_muplay_no_debugger(old_trace_dir, old_executable,
                               new_executable, flags);

  }

  int MuplayCommand::run(vector<string>& args) {
    MuplayFlags flags;
    // // TODO CODE TO PARSE OPTIONS

    string old_trace_dir, old_executable, new_executable;
    if (!parse_optional_trace_dir(args, &old_trace_dir))
    {
      printf("Problem parsing the old_trace_dir\n");
      return 1;
    }
    if (!parse_optional_trace_dir(args, &old_executable))
    {
      printf("Problem parsing the old_executable\n");
      return 1;
    }
    if (!parse_optional_trace_dir(args, &new_executable))
    {
      printf("Problem parsing the new_executable\n");
      return 1;
    }

    /* TODO add checks that the args are the right number */
    if(args.size() > 0) {};

    muplay(old_trace_dir, old_executable, new_executable, flags, args, stdout);
    return 0;
  }
} // namespace rr
