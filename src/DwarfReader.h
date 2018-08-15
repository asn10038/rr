#ifndef RR_DwarfReader_H_
#define RR_DwarfReader_H_

#include <libunwind.h>
#include <string>
namespace rr {
  class DwarfReader {
  public:
    /* Resolves the src line associated with the given memory addres */
    static std::string get_src_line(const char* elf_file_path, unw_word_t mem_address);
  };
    bool check_libdwarf_error(int return_status);
} // namespace rr
#endif
