#include <cstring>

#include <boost/math/constants/constants.hpp>
#include <boost/filesystem.hpp>

#include "tinyhtm/Query.hxx"
#include "tinyhtm/Shape.hxx"
#include "tinyhtm/Spherical.hxx"
#include "tinyhtm/Cartesian.hxx"
#include "Hires.hxx"
#include "Gnomonic.hxx"
#include "Sample.hxx"

hires::Sample sample;
std::vector<double> lon, lat, flux;
hires::Gnomonic projection (0, 0);

int callback (void *void_entry, int num_elements,
              const std::vector<H5::DataType> &,
              const std::vector<std::string> &names)

{
  char *entry = static_cast<char *>(void_entry);
  double x, y, z, flux_entry;
  const int offsets[] = { 0, 4, 8, 12, 16, 24, 28 };
  void *p;

  for (int i = 0; i < num_elements; ++i)
    {
      p = entry + offsets[i];

      if (names[i]=="x")
        {
          x = *((float *)p);
        }
      else if (names[i]=="y")
        {
          y = *((float *)p);
        }
      else if (names[i]=="z")
        {
          z = *((float *)p);
        }
      else if (names[i]=="TSKY")
        {
          flux_entry = *((float *)p);
        }
      else if (names[i]=="MJD")
        {
        }
      else if (names[i]=="PSI")
        {
        }
      else if (names[i]=="SSO")
        {
        }
      else
        throw std::runtime_error ("Unknown type: " + names[i]);
    }
  /* FIXME: Do selection based on utc */
  tinyhtm::Spherical coord (tinyhtm::Cartesian (x, y, z));
  std::tie (x, y) = projection.degrees_to_xy (coord.lon (), coord.lat ());

  lon.push_back (x);
  lat.push_back (y);
  flux.push_back (flux_entry);
  return 1;
}

hires::Sample get_sample_from_query
(const std::string &data_file,
 const std::unique_ptr<tinyhtm::Shape> &shape,
 const hires::Gnomonic &Projection)
{
  projection = Projection;
  tinyhtm::Tree tree(data_file);
  shape->search (tree, callback);
  if (lon.size () != lat.size () || lon.size () != flux.size ())
    throw std::runtime_error ("INTERNAL ERROR: sizes do not match");

  sample.x = std::valarray<double>(lon.data (), lon.size ());
  sample.y = std::valarray<double>(lat.data (), lat.size ());
  sample.signal = std::valarray<double>(flux.data (), flux.size ());
  sample.angle = std::valarray<double>(0.0, flux.size ());
  // FIXME: We should really get rid of this
  sample.id = 1;
  return sample;
}
