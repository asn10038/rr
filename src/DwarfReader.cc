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
#include <string.h> /*for memset*/


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

    //for line number operations
    Dwarf_Signed line_cnt;
    Dwarf_Line *linebuf;
    Dwarf_Addr line_addr;
    Dwarf_Unsigned line_no;


    /* opening the file and initializing dwarf info */
    int fd = open(elf_file_path, O_RDONLY);
    /* TODO add error handline */
    res = dwarf_init(fd, DW_DLC_READ, errhand, errarg, &dbg, errp);
    if(check_libdwarf_error(res))
      return "no debug info not found";

    //TODO see how this needs to be built to run more than one source file
    // probably need to include some sort of for/while loop to get each cu

    res = dwarf_next_cu_header(dbg, &cu_header_length,
                               &version_stamp,
                               &abbrev_offset,
                               &address_size,
                               &next_cu_header,
                               errp);
    if(check_libdwarf_error(res))
      return "No cu header to parse...no debug info";

    res = dwarf_siblingof(dbg, no_die, &cu_die, errp);
    if(check_libdwarf_error(res))
      FATAL() << "ERROR getting sibling of";

    res = dwarf_diename(cu_die, &cu_name, errp);
    if(check_libdwarf_error(res))
      FATAL()<< "ERROR getting cu_name";


    if ((res = dwarf_srclines(cu_die, &linebuf,&line_cnt, errp)) == DW_DLV_OK) {
      for (int i = 0; i < line_cnt; ++i) {
        /* use linebuf[i] */
        /* Get the address of the line */
        res = dwarf_lineaddr(linebuf[i], &line_addr, errp);
        if(check_libdwarf_error(res))
        {
          LOG(debug) << "ERROR getting lineaddr";
        }
        /* get the line number of the line */
        res = dwarf_lineno(linebuf[i], &line_no, errp);
        if(check_libdwarf_error(res))
        {
          LOG(debug) << "ERROR getting line number";
        }

        // LOG(debug) << cu_name << ":" << line_no << " corresponds to : 0x" << std::hex << line_addr;

        /* If the address matches the address we're looking for you have
           you have found the line number */
        if(line_addr == mem_address)
        {
          // std::string res(cu_name + ":" + line_no + " line_addr");
          std::string res_string = std::to_string(line_no);
          return res_string.c_str();
        }
        /* Print the source and line number where the line is found */

        /* de allocate line_buf where no longer in use */
        dwarf_dealloc(dbg, linebuf[i], DW_DLA_LINE);
      }
      dwarf_dealloc(dbg, linebuf, DW_DLA_LIST);
    }

    /* De allocating memory from Dwarf_Init */
    dwarf_finish(dbg, errp);
    /* closing the opened file */
    close(fd);
    return "src line not found";
  }

  bool DwarfReader::check_libdwarf_error(int return_status) {
    // TODO change this to something different for not found
    /* returns error for both not found and error */
    if(return_status == DW_DLV_ERROR ||
       return_status == DW_DLV_NO_ENTRY)
       return true;
    return false;
  }

  /* returns true if has debug info and false otherwise */
  bool DwarfReader::check_for_dwarf_info(const char* elf_file_path) {
    /* opening the file and initializing dwarf info */
    int fd = open(elf_file_path, O_RDONLY);
    Dwarf_Debug dbg;
    /* TODO add error handline */
    int res = dwarf_init(fd, DW_DLC_READ, 0, 0, &dbg, 0);
    close(fd);
    if(res == DW_DLV_NO_ENTRY || res == DW_DLV_ERROR){
      return false;
    }
    return true;
  }
} // namespace rr
