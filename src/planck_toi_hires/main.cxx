#include <set>
#include <array>

#include <CCfits/CCfits>
#include <boost/filesystem.hpp>

#include "tinyhtm/Query.hxx"
#include "tinyhtm/Shape.hxx"
#include "tinyhtm/Spherical.hxx"
#include "tinyhtm/Shape.hxx"
#include "Hires.hxx"
#include "Gnomonic.hxx"
#include "json5_parser.h"
#include "tablator/Format.hxx"

#include "Coordinate_Frame.hxx"

void usage(std::ostream &os)
{
  os << "Usage: planck_toi_hires FILE\n"
     << "       planck_toi_hires --query=QUERY\n"
     << "FILE must be a valid json5 file.  QUERY must be a valid\n"
     << "json5 object\n"
     << "To read from standard input, set FILE to '-'.\n";
}

hires::Sample get_sample_from_query
(const std::string &data_file,
 const tinyhtm::Shape &shape,
 const hires::Gnomonic &Projection);

std::vector<hires::Sample> get_sample_from_table
(const boost::filesystem::path &path,
 const std::map<std::string,std::string> &columns,
 const bool &shape_valid,
 const Coordinate_Frame &coordinate_frame,
 std::vector<std::pair<std::string, std::pair<std::string,std::string> > >
 &keywords,
 tinyhtm::Spherical &center, tinyhtm::Spherical &size);

void read_input(json5_parser::mValue &json5, const std::string &arg,
                std::string &output_prefix, std::string &input_file,
                bool &compute_minimap, bool &compute_mcm,
                bool &compute_elastic_net, double &sigma_drf, int &mcm_iterations,
                std::map<std::string,std::string> &columns,
                Coordinate_Frame &coordinate_frame,
                std::unique_ptr<tinyhtm::Shape> &shape,
                std::vector<std::pair<std::string, std::pair<std::string,
                                                             std::string> > >
                &keywords,
                double &angResolution);

int main (int argc, char *argv[])
{
  if(argc!=2)
    {
      std::cerr << "Wrong number of arguments.  Got " << argc-1 << "\n";
      usage(std::cerr);
      exit(1);
    }

  std::string argument(argv[1]);
  if(argument=="-h" || argument=="--help")
    {
      usage(std::cout);
      exit(0);
    }
      
  json5_parser::mValue json5;

  try
    {
      std::string output_prefix, input_file;
      std::map<std::string,std::string> columns={{"signal","signal"},
                                                 {"psi","psi"}};
      Coordinate_Frame coordinate_frame=Coordinate_Frame::ICRS;
      double angResolution, sigma_drf(0);
      int mcm_iterations=0;
      std::unique_ptr<tinyhtm::Shape> shape;
      std::vector<std::pair<std::string, std::pair<std::string,
                                                   std::string> > > keywords;
      bool compute_minimap(false), compute_mcm(false),
        compute_elastic_net(false);
      read_input(json5,argument,output_prefix,input_file,compute_minimap,
                 compute_mcm,compute_elastic_net,sigma_drf,mcm_iterations,
                 columns,coordinate_frame,shape,keywords,angResolution);

      /// Load the samples
      std::vector<hires::Sample> samples;

      tinyhtm::Spherical center, size;
      if(shape)
        std::tie(center,size)=shape->bounding_box();

      boost::filesystem::path path(input_file);
      if(Tablator::Format(path).is_hdf5() &&
         H5::H5File(input_file, H5F_ACC_RDONLY).getNumObjs()==2)
        {
          if(!shape)
            throw std::runtime_error("Shape required for this input file: "
                                     + input_file);
          hires::Gnomonic projection (center.lon(),center.lat());
          samples.emplace_back(get_sample_from_query(input_file, *shape,
                                                     projection));
        }
      else
        {
          samples=get_sample_from_table(path, columns, shape.operator bool(),
                                        coordinate_frame, keywords, center,
                                        size);
        }

      std::array<size_t,2> nxy{{static_cast<size_t>(size.lon()/angResolution),
            static_cast<size_t>(size.lat()/angResolution)}};
      std::array<double,2> crval{{center.lon(),center.lat()}};

      constexpr double pi=boost::math::constants::pi<double>();
      hires::Hires hires (nxy,crval,angResolution*pi/180,keywords,samples);
      if (compute_minimap)
        {
          hires.compute_minimap ();
          hires.write_minimap (output_prefix);
        }
      if (compute_mcm)
        {
          hires.compute_mcm (sigma_drf, mcm_iterations);
          hires.write_mcm (output_prefix);
        }
      if (compute_elastic_net)
        {
          hires.compute_elastic_net (sigma_drf);
          hires.write_elastic_net (output_prefix);
        }
    }
  catch (CCfits::FitsException &e)
    {
      std::cerr << e.message() << "\n";
    }
  catch (std::runtime_error &e)
    {
      std::cerr << "ERROR: " << e.what () << "\n";
      exit (1);
    }
}
