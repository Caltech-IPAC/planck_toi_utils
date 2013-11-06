#ifndef PLANCK_TIME_RANGES_RANGE_HXX
#define PLANCK_TIME_RANGES_RANGE_HXX

#include <cstdint>
#include <vector>
#include <cstddef>
#include "tinyhtm/Query.hxx"

class Ranges
{
public:
  static const uint64_t begin_time, end_time;
  const uint64_t dt;
  std::vector<uint64_t> r;
  Ranges(const size_t &s): dt((end_time-begin_time)/s), r(s) {}
  int callback(void *entry, int num_elements, hid_t *, char **names);
};

inline std::ostream & operator<<(std::ostream &os, const Ranges &r)
{
  for(size_t i=0; i<r.r.size(); ++i)
    {
      if(r.r[i]!=0)
        os << (r.begin_time + i*r.dt) << " "
           << (r.begin_time + (i+1)*r.dt) << " "
           << r.r[i] << "\n";
    }
  return os;
}

#endif
