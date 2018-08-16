#ifndef RR_DwarfReader_H_
#define RR_DwarfReader_H_


#include </usr/local/include/dwarf.h>
#include </usr/local/include/libdwarf.h>

#include <libunwind.h>
#include <string>
namespace rr {
  class DwarfReader {
  public:
    /* Functions */

    /* Safe call to get_src_line that checks for special cases and errors thrown during dwarf_init */
    static std::string safe_get_src_line(const char* elf_file_path, unw_word_t mem_address);
    /* Resolves the src line associated with the given memory addres */
    static std::string get_src_line(const char* elf_file_path, unw_word_t mem_address);

    /* Checks if result of libdwarf call resulted in not found or error */
    static bool check_libdwarf_error(int return_status);

    /* Determines if the file at elf_file_path has debugging information */
    static bool check_for_dwarf_info(const char* elf_file_path);

    /* error handler for the dwarf_init called in check_for_dwarf_info */
    static void check_dwarf_info_init_handler(Dwarf_Error error, Dwarf_Ptr errarg);
  };


} // namespace rr
#endif
