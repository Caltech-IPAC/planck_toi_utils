#include <iostream>
#include <string>
#include <stdexcept>
#include <cmath>
#include <vector>

#include <H5Cpp.h>

#include "../lzf/lzf_filter.h"
#include "indexed_entry.hxx"
#include "raw_entry.hxx"

void read_raw_hdf5(const std::string &filename,
                   std::vector<raw_entry> &raw_entries);
void read_indexed_hdf5(const std::string &filename,
                       std::vector<indexed_entry> &indexed_entries);

/// Compares the glon and glat of the original hdf5 files received
/// from the Planck project with the search results obtained through
/// the web api.  The files should have the same data, so the best way
/// to do this is to look at a single raw file at a time
/// (e.g. 100-1a_091.hdf5).  Run planck_to_htm to index it for the web
/// api.  Then point the web api to that indexed file and do a full
/// sky search.  
///
/// curl -v -o 100-1a_091_result.hdf5 "http://hades.ipac.caltech.edu/TAP/sync?LANG=ADQL&REQUEST=doQuery&QUERY=SELECT+*+FROM+planck_toi_100_1a+WHERE+CONTAINS%28POINT%28%27J2000%27,ra,dec%29,CIRCLE%28%27J2000%27,0,0,179%29%29=1&format=hdf5"
///
/// Then run compare against the two files
///
/// ./build/compare 100-1a_091.hdf5 100-1a_091_result.hdf5
///
/// Both the raw file (100-1a_091.hdf5) and the search result
/// (100-1a_091_result.hdf5) must have the same number of entries.

int main(int argc, char *argv[])
{
  if(argc!=3)
    {
      std::cout << "compare <raw_hdf5> <indexed_hdf5>\n";
      exit(1);
    }
  try
    {
      register_lzf();
      std::vector<raw_entry> raw_entries;
      read_raw_hdf5(argv[1],raw_entries);

      std::vector<indexed_entry> indexed_entries;
      read_indexed_hdf5(argv[2],indexed_entries);

      if(indexed_entries.size()!=raw_entries.size())
        throw std::runtime_error("Sizes are different\n\traw: "
                                 + std::to_string(raw_entries.size())
                                 + "\n\tindexed: "
                                 + std::to_string(indexed_entries.size()));
      double max_error=0;
      float min_glon=1e7, max_glon=-1e7, min_glat=1e7, max_glat=-1e7;
      for(size_t i=0; i<indexed_entries.size(); ++i)
        {
          min_glon=std::min(min_glon,raw_entries[i].glon);
          min_glat=std::min(min_glat,raw_entries[i].glat);
          max_glon=std::max(max_glon,raw_entries[i].glon);
          max_glat=std::max(max_glat,raw_entries[i].glat);

          double glon_error=
            3600*std::abs(raw_entries[i].glon-indexed_entries[i].glon);
          if(glon_error>3600*359 && glon_error<3600*361)
            glon_error-=3600*360;
          double glat_error=
            3600*std::abs(raw_entries[i].glat-indexed_entries[i].glat);

          max_error=std::max(max_error,glon_error);
          max_error=std::max(max_error,glat_error);
        }
      std::cout << "max error in arcseconds: " << max_error << "\n";
      std::cout << "glon: " << min_glon << " " << max_glon << "\n";
      std::cout << "glat: " << min_glat << " " << max_glat << "\n";
    }
  catch (H5::Exception &e)
    {
      std::cerr << "HDF5 Exception: "
                << e.getDetailMsg() << "\n";
      exit(1);
    }
  catch (std::runtime_error& e) 
    {
      std::cerr << "Error: "
                << e.what() << "\n";
      exit(1);
    }
}
