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

double extract_number(const std::map<std::string, Tablator::Property>
                      &properties, const std::string &key)
{
  auto iter=properties.find(key);
  if(iter==properties.end())
    throw std::runtime_error("Expected " + key + " in the table");
  return std::stod(iter->second.value);
}


void get_sample_from_table(const boost::filesystem::path &path,
                           const std::map<std::string,std::string> &columns,
                           const bool &shape_valid,
                           const Coordinate_Frame &coordinate_frame,
                           std::vector<hires::Sample> &samples,
                           std::vector<std::pair<std::string,
                                                 std::pair<std::string,
                                                           std::string> > >
                           &keywords,
                           tinyhtm::Spherical &center, tinyhtm::Spherical &size)
{
  Tablator::Table table(path);

  /// Get BUNIT from the signal column
  keywords.push_back({"BUNIT",{table.fields_properties.at
          (table.compound_type.getMemberIndex(columns.at("signal")))
          .attributes["unit"],""}});
  auto keyword_mapping=Tablator::fits_keyword_mapping(true);
  for(auto &p: table.properties)
    {
      std::string name=p.first;
      auto it=keyword_mapping.find(name);
      if(it!=keyword_mapping.end())
        name=it->second;
      keywords.push_back({name,{p.second.value,""}});
    }
  if(!shape_valid)
    {
      switch(coordinate_frame)
        {
        case Coordinate_Frame::ICRS:
        case Coordinate_Frame::J2000:
          center.lon()=extract_number(table.properties,"pos.eq.ra");
          center.lat()=extract_number(table.properties,"pos.eq.dec");
          break;
        case Coordinate_Frame::Galactic:
          center.lon()=extract_number(table.properties,"pos.galactic.lon");
          center.lat()=extract_number(table.properties,"pos.galactic.lat");
          break;
        }
      
      auto iter=table.properties.find("pos.radius");
      if(iter==table.properties.end())
        {
          iter=table.properties.find("pos.width");
          if(iter==table.properties.end())
            throw std::runtime_error("Either pos.radius or pos.width must "
                                     "exist in the table if "
                                     "a shape is not specified");
          size.lon()=std::stod(iter->second.value);

          size.lat()=extract_number(table.properties,"pos.height");
        }
      else
        {
          size.lon()=size.lat()=2*std::stod(iter->second.value);
        }
    }
  hires::Gnomonic projection (center.lon (), center.lat ());

  samples.push_back(hires::Sample());
  auto &x=samples[0].x;
  auto &y=samples[0].y;
  auto &signal=samples[0].signal;
  auto &psi=samples[0].angle;

  x.resize(table.size());
  y.resize(table.size());
  signal.resize(table.size());
  psi.resize(table.size());

  size_t ra_offset(table.compound_type.getMemberOffset
                   (table.compound_type.getMemberIndex(columns.at("ra")))),
    dec_offset(table.compound_type.getMemberOffset
               (table.compound_type.getMemberIndex(columns.at("dec")))),
    signal_offset(table.compound_type.getMemberOffset
                  (table.compound_type.getMemberIndex(columns.at("signal")))),
    psi_offset(table.compound_type.getMemberOffset
               (table.compound_type.getMemberIndex(columns.at("psi"))));

  size_t num_rows=table.size();
  for(size_t row=0; row<num_rows; ++row)
    {
      double ra=*reinterpret_cast<double*>(table.data.data()+row*table.row_size
                                           +ra_offset),
        dec=*reinterpret_cast<double*>(table.data.data()+row*table.row_size
                                       +dec_offset);
      std::tie(x[row],y[row])=projection.degrees_to_xy(ra,dec);
      // FIXME: Figure out whether float or double automatically
      signal[row]=*reinterpret_cast<float*>(table.data.data()+row*table.row_size
                                             +signal_offset);
      psi[row]=*reinterpret_cast<float*>(table.data.data()+row*table.row_size
                                          +psi_offset);
    }
  // FIXME: We should really get rid of this
  samples[0].id=1;
}
