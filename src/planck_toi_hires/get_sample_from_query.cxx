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

class Query_Samples
{
public:
  std::vector<hires::Sample> &samples;
  const hires::Gnomonic &projection;

  size_t x_offset, y_offset, z_offset, signal_offset, angle_offset;

  Query_Samples (std::vector<hires::Sample> &Samples,
                 const hires::Gnomonic &Projection,
                 const std::vector<std::string> &names)
      : samples (Samples), projection (Projection)
  {
    // FIXME: If the offsets are hard coded, why bother checking the
    // names?
    const size_t offsets[] = { 0, 4, 8, 12, 16, 24, 28 };
    size_t index = 0;
    for (auto &name : names)
      {
        if (name == "x")
          {
            x_offset = offsets[index];
          }
        else if (name == "y")
          {
            y_offset = offsets[index];
          }
        else if (name == "z")
          {
            z_offset = offsets[index];
          }
        else if (name == "SIGNAL")
          {
            signal_offset = offsets[index];
          }
        else if (name == "PSI")
          {
            angle_offset = offsets[index];
          }
        else if (name == "MJD")
          {
          }
        else if (name == "SSO")
          {
          }
        else
          throw std::runtime_error ("Unknown type: " + name);
        ++index;
      }
  }

  bool add_entry (const char *entry)
  {
    double x (*reinterpret_cast<const float *>(entry + x_offset)),
        y (*reinterpret_cast<const float *>(entry + y_offset)),
        z (*reinterpret_cast<const float *>(entry + z_offset)),
        signal (*reinterpret_cast<const float *>(entry + signal_offset)),
        angle (*reinterpret_cast<const float *>(entry + angle_offset));

    /* FIXME: Do selection based on utc */
    tinyhtm::Spherical coord (tinyhtm::Cartesian (x, y, z));
    double x_projected, y_projected;
    std::tie (x_projected, y_projected)
        = projection.degrees_to_xy (coord.lon (), coord.lat ());

    samples.emplace_back (x_projected, y_projected, signal, angle);
    return true;
  }
};

std::vector<hires::Sample>
get_sample_from_query (const std::string &data_file,
                       const tinyhtm::Shape &shape,
                       const hires::Gnomonic &projection)
{
  tinyhtm::Tree tree (data_file);
  std::vector<hires::Sample> samples;
  Query_Samples query_samples (samples, projection, tree.tree.element_names);
  shape.search (tree, std::bind (&Query_Samples::add_entry, &query_samples,
                                 std::placeholders::_1));
  return query_samples.samples;
}
