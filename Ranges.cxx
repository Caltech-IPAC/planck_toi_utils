#include "Ranges.hxx"
#include <cstring>

/* Begin and end times for Planck mission */
const uint64_t Ranges::begin_time=1705060000000000000;
const uint64_t Ranges::end_time=1705160000000000000;

int Ranges::callback(void *entry, int num_elements, hid_t *, char **names)
{
  for(int i=0;i<num_elements;++i)
    {
      if(0==std::strcmp(names[i],"UTC"))
        {
          int64_t utc=*((int64_t *)(entry)+i);
          
          size_t j((utc-begin_time)/dt);
          if(j>=r.size())
            {
              std::stringstream ss;
              ss << "Bad time in database: " << utc;
              throw std::runtime_error(ss.str());
            }
          r[j]+=1;
        }
    }
  return 1;
}
