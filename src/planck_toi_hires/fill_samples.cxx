#include <cstring>
#include <boost/math/constants/constants.hpp>
#include "tinyhtm/Query.hxx"
#include "tinyhtm/Spherical.hxx"
#include "tinyhtm/Cartesian.hxx"
#include "Hires.hxx"
#include "Gnomonic.hxx"
#include "Sample.hxx"

hires::Sample sample;
std::vector<double> lon, lat, flux;
hires::Gnomonic projection(0,0);

bool callback(void *entry, int num_elements, hid_t *, char **names)
{
  double x,y,z,flux_entry;
  // int64_t utc,ring;
  for(int i=0;i<num_elements;++i)
    {
      if(0==std::strcmp(names[i],"x"))
        {
          x=*((double *)(entry)+i);
        }
      else if(0==std::strcmp(names[i],"y"))
        {
          y=*((double *)(entry)+i);
        }
      else if(0==std::strcmp(names[i],"z"))
        {
          z=*((double *)(entry)+i);
        }
      else if(0==std::strcmp(names[i],"TSKY"))
        {
          flux_entry=*((double *)(entry)+i);
        }
      else if(0==std::strcmp(names[i],"UTC"))
        {
          // utc=*((int64_t *)(entry)+i);               
        }
      else if(0==std::strcmp(names[i],"RING"))
        {
          // ring=*((int64_t *)(entry)+i);               
        }
      else
        throw std::runtime_error("Unknown type: " + std::string(names[i]));
    }
  /* FIXME: Do selection based on utc or ring */
  tinyhtm::Spherical coord(tinyhtm::Cartesian(x,y,z));
  std::tie(x,y)=projection.lonlat2xy(projection.radians(coord.lon()),
                                     projection.radians(coord.lat()));
  lon.push_back(x);
  lat.push_back(y);
  flux.push_back(flux_entry);
  return true;
}

hires::Sample fill_samples(const tinyhtm::Query &query,
                           const hires::Gnomonic &Projection)
{
  projection=Projection;
  query.callback(callback);
  if(lon.size()!=lat.size() || lon.size()!=flux.size())
    throw std::runtime_error("INTERNAL ERROR: sizes do not match");

  sample.x=std::valarray<double>(lon.data(),lon.size());
  sample.y=std::valarray<double>(lat.data(),lat.size());
  sample.flux=std::valarray<double>(flux.data(),flux.size());
  sample.angle=std::valarray<double>(0.0,flux.size());
  sample.id=1;
  return sample;
}