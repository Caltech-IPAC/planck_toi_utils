#include <stdexcept>

#include "Results.hxx"

bool Results::callback(void *entry, int num_elements, hid_t *, char **names)
{
  if(utc_index==static_cast<size_t>(-1))
    {
      for(int i=0;i<num_elements;++i)
        {
          if(names[i]==std::string("UTC"))
            {
              utc_index=i;
            }
          else if(names[i]==std::string("TSKY"))
            {
              tsky_index=i;
            }
          else if(names[i]==std::string("RING"))
            {
              ring_index=i;
            }
          else if(names[i]==std::string("x"))
            {
              x_index=i;
            }
          else if(names[i]==std::string("y"))
            {
              y_index=i;
            }
          else if(names[i]==std::string("z"))
            {
              z_index=i;
            }
          else
            {
              // FIXME: This will certainly be fatal, since we can not
              // catch exceptions called from threads.
              throw std::runtime_error("Bad column name: "
                                       + std::string(names[i]));
            }
        }
      if(x_index==static_cast<size_t>(-1))
        throw std::runtime_error("Missing column 'x' in HTM file");
      if(y_index==static_cast<size_t>(-1))
        throw std::runtime_error("Missing column 'y' in HTM file");
      if(z_index==static_cast<size_t>(-1))
        throw std::runtime_error("Missing column 'z' in HTM file");
      if(utc_index==static_cast<size_t>(-1))
        throw std::runtime_error("Missing column 'UTC' in HTM file");
      if(tsky_index==static_cast<size_t>(-1))
        throw std::runtime_error("Missing column 'TSKY' in HTM file");
      if(ring_index==static_cast<size_t>(-1))
        throw std::runtime_error("Missing column 'RING' in HTM file");
    }

  uint64_t utc=*((uint64_t *)(entry)+utc_index);

  bool in_interval(time_intervals.empty());
  for(auto &t: time_intervals)
    {
      if(utc>=t.first && utc<t.second)
        {
          in_interval=true;
          break;
        }
    }
  if(in_interval && !count)
    {
      double x(((double *)(entry))[x_index]),
        y(((double *)(entry))[y_index]),
        z(((double *)(entry))[z_index]),
        tsky(((double *)(entry))[tsky_index]),
        ring(((uint64_t *)(entry))[ring_index]);
      data.push_back(std::make_tuple(x,y,z,utc,tsky,ring));
    }
  return in_interval;
}
