#include <cstring>
#include <boost/math/constants/constants.hpp>
#include "tinyhtm/Query.hxx"
#include "tinyhtm/Spherical.hxx"
#include "tinyhtm/Cartesian.hxx"
#include "Hires.hxx"
#include "Gnomonic.hxx"
#include "Sample.hxx"
#include <CCfits/CCfits>

void add_keywords_from_fits(std::string filename_toi, std::vector<std::string> filename_list)
{
   
  hires::Sample sample;
  std::vector<double> lon, lat, flux;
  std::vector<double> ximg, yimg;

  std::cout << "opening " << filename_toi.c_str() << "\n";
  CCfits::FITS fits_in(filename_toi, CCfits::Read, false);
  CCfits::ExtHDU &exthdu_in = fits_in.extension(1);
  exthdu_in.readAllKeys();
  std::map<string, CCfits::Keyword *> kmap = exthdu_in.keyWord();

  for (size_t i = 0; i < filename_list.size(); i++) {
      std::cout << "opening " << filename_list[i].c_str() << "\n";
      CCfits::FITS fits_out(filename_list[i], CCfits::Write, false);

      fits_out.pHDU().addKey(kmap["DETNAME"]);
      fits_out.pHDU().addKey(kmap["FREQ"]);
      fits_out.pHDU().addKey(kmap["INSTRUME"]);
      fits_out.pHDU().addKey(kmap["OBJECT"]);
      fits_out.pHDU().addKey(kmap["OBJ_TYPE"]);
      fits_out.pHDU().addKey(kmap["PROCVER"]);
      fits_out.pHDU().addKey(kmap["TELESCOP"]);
  }
}
