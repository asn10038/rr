/* Reads the dwarf information to find location of source files */

// TODO figure out how to not make this a full path
#include </usr/local/include/dwarf.h>
#include </usr/local/include/libdwarf.h>

/* Windows specific header files */
#if defined(_WIN32) && defined(HAVE_STDAFX_H)
#include "stdafx.h"
#endif /* HAVE_STDAFX_H */

#include <sys/types.h> /* For open() */
#include <sys/stat.h>  /* For open() */
#include <fcntl.h>     /* For open() */
#include <stdlib.h>     /* For exit() */
#include <unistd.h>     /* For close() */


#include "DwarfReader.h"
#include "log.h"


using namespace std;

namespace rr {
  std::string DwarfReader::get_src_line(const char* elf_file_path,
                                        unw_word_t mem_address)
  {
    /* For now doing based on what is in simple reader */
    Dwarf_Debug dbg = 0;
    Dwarf_Die cu_die = 0;
    Dwarf_Die no_die = 0;
    // Dwarf_Die sibling_die = 0;
    // int fd = -1;
    int res = DW_DLV_ERROR;
    // Dwarf_Error error;
    Dwarf_Handler errhand = 0;
    Dwarf_Ptr errarg = 0;
    // Dwarf_Sig8 hash8;
    Dwarf_Error *errp = 0;
    // int simpleerrhand = 0;

    // for next_cu_header
    Dwarf_Unsigned cu_header_length = 0;
    Dwarf_Half version_stamp = 0;
    Dwarf_Unsigned abbrev_offset = 0;
    Dwarf_Half address_size = 0;
    Dwarf_Unsigned next_cu_header = 0;

    // to get cu name
    char* cu_name;


    /* opening the file and initializing dwarf info */
    int fd = open(elf_file_path, O_RDONLY);
    /* TODO add error handline */
    res = dwarf_init(fd, DW_DLC_READ, errhand, errarg, &dbg, errp);
    if(check_libdwarf_error(res))
      FATAL() << "Couldn't init dwarf info";
    if (res == DW_DLV_OK)
      LOG(debug) << "successfully called dwarf_init";
    if(mem_address) {}

    res = dwarf_next_cu_header(dbg, &cu_header_length,
                               &version_stamp,
                               &abbrev_offset,
                               &address_size,
                               &next_cu_header,
                               errp);
    if(check_libdwarf_error(res))
      LOG(debug) << "ERROR getting next cu header";

    res = dwarf_siblingof(dbg, no_die, &cu_die, errp);
    if(check_libdwarf_error(res))
      LOG(debug) << "ERROR getting sibling of";

    res = dwarf_diename(cu_die, &cu_name, errp);
    if(check_libdwarf_error(res))
      LOG(debug) << "ERROR getting cu_name";
    LOG(debug) << "cu_name is " << cu_name;


    Dwarf_Signed line_cnt;
    Dwarf_Line *linebuf;

    if ((res = dwarf_srclines(cu_die, &linebuf,&line_cnt, errp)) == DW_DLV_OK) {
      for (int i = 0; i < line_cnt; ++i) {
        /* use linebuf[i] */
        dwarf_dealloc(dbg, linebuf[i], DW_DLA_LINE);
      }
        LOG(debug)<<"Line_count is: " << line_cnt;
      dwarf_dealloc(dbg, linebuf, DW_DLA_LIST);
    }

    /* De allocating memory from Dwarf_Init */
    dwarf_finish(dbg, errp);
    /* closing the opened file */
    close(fd);
    return "this";
  }

  bool check_libdwarf_error(int return_status) {
    // TODO change this to something different for not found
    /* returns error for both not found and error */
    if(return_status == DW_DLV_ERROR ||
       return_status == DW_DLV_NO_ENTRY)
       return true;
    return false;
  }
} // namespace rr
