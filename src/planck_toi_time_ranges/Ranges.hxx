#ifndef PLANCK_TOI_TIME_RANGES_RANGE_HXX
#define PLANCK_TOI_TIME_RANGES_RANGE_HXX

#include <cstdint>
#include <vector>
#include <cstddef>
#include "tinyhtm/Query.hxx"

class Ranges
{
public:
  static const double begin_time, end_time;
  int mjd_index;
  const double dt;
  std::vector<double> r;
  Ranges(const size_t &s): mjd_index(-1), dt((end_time-begin_time)/s), r(s) {}
  bool callback(void *entry, int num_elements, hid_t *, char **names);
};

inline std::ostream & operator<<(std::ostream &os, const Ranges &r)
{
  os << "t_begin" << "\t" << "t_end" << "\t" << "count" << "\n";
  for(size_t i=0; i<r.r.size(); ++i)
    {
      if(r.r[i]!=0)
        os << (r.begin_time + i*r.dt) << "\t"
           << (r.begin_time + (i+1)*r.dt) << "\t"
           << r.r[i] << "\n";
    }
  return os;
}

#endif
