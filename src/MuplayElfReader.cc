#include <fstream>
#include "log.h"
#include "MuplayElfReader.h"

using namespace std;

namespace rr {

  MuplayElfReader::MuplayElfReader(std::string elf_path) :
  elf_path(elf_path) { }


  Elf64_Ehdr MuplayElfReader::read_elf_header()
  {
    Elf64_Ehdr elf64_ehdr;
    std::ifstream ifs(elf_path,std::ifstream::in | std::ifstream::binary);
    if(ifs.good())
    {
        ifs.read((char*)&elf64_ehdr, sizeof(Elf64_Ehdr));
        return elf64_ehdr;
    } else {
      ifs.close();
      FATAL() << "Couldn't open up the elf file at " << elf_path;
    }
    return elf64_ehdr;
  }

  std::vector<Elf64_Phdr> MuplayElfReader::read_loadable_segments(Elf64_Off e_phoff, Elf64_Half e_phnum) {
    std::vector<Elf64_Phdr> res;
    std::ifstream ifs(elf_path, std::ifstream::in | std::ifstream::binary);
    // go to the offset of the program header
    ifs.seekg(e_phoff);

    // for loop through all the program headers
    Elf64_Phdr elf64_phdr;
    for(int i=0; i<e_phnum; i++)
    {
      ifs.read((char*)&elf64_phdr, sizeof(Elf64_Phdr));
      // check if the segment is of type LOAD
      if(elf64_phdr.p_type == PT_LOAD)
      {
        LOG(debug) << " You found some loadable segments";
      }
    }

    // add the loadable segments to the list of loadable segments
    return res;
  }

}
