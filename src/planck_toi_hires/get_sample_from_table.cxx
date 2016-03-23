#include <map>
#include <string>

#include "tablator/Table.hxx"
#include "tablator/Property.hxx"
#include "tablator/fits_keyword_mapping.hxx"
// FIXME: Hires.hxx should be hires/Hires.hxx
#include "Hires.hxx"
#include "Gnomonic.hxx"
#include "tinyhtm/Spherical.hxx"

#include "Coordinate_Frame.hxx"

double
extract_number (const std::vector<std::pair<std::string, tablator::Property> >
                &properties,
                const std::string &key)
{
  for (auto &property: properties)
    {
      if (property.first==key)
        return std::stod (property.second.value);
    }
  throw std::runtime_error ("Expected " + key + " in the table");
}

std::vector<hires::Sample> get_sample_from_table (
    const boost::filesystem::path &path,
    const std::map<std::string, std::string> &columns, const bool &shape_valid,
    const Coordinate_Frame &coordinate_frame,
    std::vector<std::pair<std::string,
                          std::pair<std::string, std::string> > > &keywords,
    tinyhtm::Spherical &center, tinyhtm::Spherical &size)
{
  tablator::Table table (path);

  /// Get BUNIT from the signal column
  auto column=std::find_if (table.columns.begin (), table.columns.end (),
                            [](const tablator::Column &c)
                            { return c.name == "signal"; });
  if (column == table.columns.end ())
    throw std::runtime_error ("Could not find 'signal' column in result.");

  keywords.push_back ({ "BUNIT", {column->field_properties.attributes["unit"],
          ""}});
  auto keyword_mapping = tablator::fits_keyword_mapping (true);
  for (auto &p : table.properties)
    {
      std::string name = p.first;
      auto it = keyword_mapping.find (name);
      if (it != keyword_mapping.end ())
        name = it->second;
      keywords.push_back ({ name, { p.second.value, "" } });
    }
  if (!shape_valid)
    {
      switch (coordinate_frame)
        {
        case Coordinate_Frame::ICRS:
        case Coordinate_Frame::J2000:
          center.lon () = extract_number (table.properties, "pos.eq.ra");
          center.lat () = extract_number (table.properties, "pos.eq.dec");
          break;
        case Coordinate_Frame::Galactic:
          center.lon ()
              = extract_number (table.properties, "pos.galactic.lon");
          center.lat ()
              = extract_number (table.properties, "pos.galactic.lat");
          break;
        }

      auto iter = std::find_if
        (table.properties.begin (), table.properties.end (),
         [&] (const std::pair<std::string,tablator::Property> &p)
         { return p.first == "pos.radius";});
      if (iter == table.properties.end ())
        {
          iter = std::find_if
            (table.properties.begin (), table.properties.end (),
             [&] (const std::pair<std::string,tablator::Property> &p)
             { return p.first == "pos.width";});
          if (iter == table.properties.end ())
            throw std::runtime_error ("Either pos.radius or pos.width must "
                                      "exist in the table if "
                                      "a shape is not specified");
          size.lon () = std::stod (iter->second.value);

          size.lat () = extract_number (table.properties, "pos.height");
        }
      else
        {
          size.lon () = size.lat () = std::sqrt (2.0)
                                      * std::stod (iter->second.value);
        }
    }
  hires::Gnomonic projection (center.lon (), center.lat ());

  std::vector<hires::Sample> samples;
  samples.reserve (table.num_rows ());

  size_t ra_offset (table.column_offset (columns.at ("ra"))),
    dec_offset (table.column_offset (columns.at ("dec"))),
    signal_offset (table.column_offset (columns.at ("signal"))),
    psi_offset (table.column_offset (columns.at ("psi")));

  size_t num_rows = table.num_rows ();
  for (size_t row = 0; row < num_rows; ++row)
    {
      double ra = *reinterpret_cast<double *>(table.data.data ()
                                              + row * table.row_size ()
                                              + ra_offset),
             dec = *reinterpret_cast<double *>(table.data.data ()
                                               + row * table.row_size ()
                                               + dec_offset);
      double x, y;
      std::tie (x, y) = projection.degrees_to_xy (ra, dec);
      // FIXME: Figure out whether float or double automatically
      float signal = *reinterpret_cast<float *>(table.data.data ()
                                                + row * table.row_size ()
                                                + signal_offset);
      float psi = *reinterpret_cast<float *>(table.data.data ()
                                             + row * table.row_size ()
                                             + psi_offset);
      samples.emplace_back (x, y, signal, psi);
    }
  return samples;
}
