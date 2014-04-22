#include "Ranges.hxx"
#include <cstring>

//FIXME: Are these good numbers?
/* Begin and end times for Planck mission */
const double Ranges::begin_time=55044.0;
const double Ranges::end_time=56700.0;

bool Ranges::callback(void *entry, int num_elements, hid_t *, char **names)
{
  if(mjd_index==-1)
    for(int i=0;i<num_elements;++i)
      {
        if(0==std::strcmp(names[i],"MJD"))
          {
            mjd_index=i;
          }
      }

  double mjd=*((double *)(entry)+mjd_index);
          
  size_t j((mjd-begin_time)/dt);
  if(j>=r.size())
    {
      std::stringstream ss;
      ss << "Bad time in database: " << mjd
         << "Allowed range: "
         << begin_time << " "
         << end_time;
      throw std::runtime_error(ss.str());
    }
  r[j]+=1;

  return true;
}
