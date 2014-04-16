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
  std::vector<std::tuple<float,float,float,float,double,float,char>> data;
  std::list<std::pair<float,float> > &time_intervals;
  size_t x_index, y_index, z_index, psi_index, mjd_index, tsky_index, sso_index;
  bool valid, count;

  bool callback(void *entry, int num_elements, hid_t *, char **names);

  Results(std::list<std::pair<float,float> > &t):
    time_intervals(t),
    x_index(-1), y_index(-1), z_index(-1), psi_index(-1), mjd_index(-1), tsky_index(-1),
    sso_index(-1),
    valid(true), count(false) {}

  void write_fits(char * fname);
};

inline std::ostream & operator<<(std::ostream &os, const Results &results)
{
  os << "x" << "\t" << "y" << "\t" << "z" << "\t"
     << "PSI" << "\t" << "MJD" << "\t" << "TSKY" << "\t" << "SSO\n";

  for(auto &r: results.data)
    {
      os << std::get<0>(r) << "\t"
         << std::get<1>(r) << "\t"
         << std::get<2>(r) << "\t"
         << std::get<3>(r) << "\t"
         << std::get<4>(r) << "\t"
         << std::get<5>(r) << "\t"
         << std::get<6>(r) << "\n";
    }
  return os;
}

#endif
