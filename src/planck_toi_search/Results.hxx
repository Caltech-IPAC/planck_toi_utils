#ifndef PLANCK_TOI_SEARCH_RESULTS_HXX
#define PLANCK_TOI_SEARCH_RESULTS_HXX

#include <vector>
#include <list>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <tuple>

#include <hdf5.h>

#include "tinyhtm/Query.hxx"

class Results
{
public:
  std::vector<std::tuple<float, float, float, float, double, float, char> > data;
  void write_fits (char *fname);
};

inline std::ostream &operator<<(std::ostream &os, const Results &results)
{
  os << "x"
     << "\t"
     << "y"
     << "\t"
     << "z"
     << "\t"
     << "PSI"
     << "\t"
     << "MJD"
     << "\t"
     << "SIGNAL"
     << "\t"
     << "SSO\n";
  int sso;

  for (auto &r : results.data)
    {
      sso = std::get<6>(r);
      os << std::get<0>(r) << "\t" << std::get<1>(r) << "\t" << std::get<2>(r)
         << "\t" << std::get<3>(r) << "\t" << std::get<4>(r) << "\t"
         << std::get<5>(r) << "\t" << sso << "\n";
    }
  return os;
}

#endif
