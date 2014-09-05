#include <cstring>
#include <boost/math/constants/constants.hpp>
#include "tinyhtm/Query.hxx"
#include "tinyhtm/Spherical.hxx"
#include "tinyhtm/Cartesian.hxx"
#include "Hires.hxx"
#include "Gnomonic.hxx"
#include "Sample.hxx"
#include <CCfits/CCfits>


hires::Sample fill_samples_from_fits(std::map<std::string, CCfits::Column *> &columns,
                            const hires::Gnomonic &Projection)
{
  double x, y;
  hires::Sample sample;
  std::vector<double> lon, lat, flux;
  std::vector<double> ximg, yimg;

  CCfits::Column *tsky_column =
            dynamic_cast<CCfits::Column *>(columns["tsky"]);
  if (tsky_column == nullptr)
     throw std::runtime_error("Can not open TSKY column");
  const size_t num_points_in_file(tsky_column->rows());
  tsky_column->read(flux, 0, num_points_in_file);

  CCfits::Column *glon_column =
            dynamic_cast<CCfits::Column *>(columns["ra"]);
  if (glon_column == nullptr)
     throw std::runtime_error("Can not open GLON column");
  glon_column->read(lon, 0, num_points_in_file);

  CCfits::Column *glat_column =
            dynamic_cast<CCfits::Column *>(columns["dec"]);
  if (glat_column == nullptr)
     throw std::runtime_error("Can not open GLAT column");
  glat_column->read(lat, 0, num_points_in_file);


  for (size_t i = 0; i < num_points_in_file; ++i) {
      std::tie (x, y) = Projection.lonlat2xy(Projection.radians(lon[i]),
                                             Projection.radians(lat[i]));
      ximg.push_back(x);
      yimg.push_back(y);
  }

  sample.x = std::valarray<double>(ximg.data (), ximg.size ());
  sample.y = std::valarray<double>(yimg.data (), yimg.size ());
  sample.flux = std::valarray<double>(flux.data (), flux.size ());
  sample.angle = std::valarray<double>(0.0, flux.size ());
  sample.id = 1;
  return sample;
}
