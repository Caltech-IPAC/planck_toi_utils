#include <stdexcept>
#include <CCfits/CCfits>

#include "Query.hxx"
#include "tinyhtm.h"

bool Query::callback (const char *entry)
{
  double mjd = *(reinterpret_cast<const double *>(entry + mjd_offset));

  bool in_interval (time_intervals.empty ());
  for (auto &t : time_intervals)
    {
      if ((mjd >= t.first && mjd <= t.second))
        {
          in_interval = true;
          break;
        }
    }
  if (in_interval)
    {
      float x (*reinterpret_cast<const float *>(entry + x_offset)),
          y (*reinterpret_cast<const float *>(entry + y_offset)),
          z (*reinterpret_cast<const float *>(entry + z_offset)),
          psi (*reinterpret_cast<const float *>(entry + psi_offset)),
          signal (*reinterpret_cast<const float *>(entry + signal_offset)),
          sso (*(entry + sso_offset));
      results.data.push_back (std::make_tuple (x, y, z, psi, mjd, signal, sso));
    }
  return in_interval;
}
