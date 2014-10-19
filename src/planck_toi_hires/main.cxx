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
 const std::unique_ptr<tinyhtm::Shape> &shape,
 const hires::Gnomonic &Projection);

void get_sample_from_table(const boost::filesystem::path &path,
                           const std::map<std::string,std::string> &columns,
                           const bool &shape_valid,
                           std::vector<hires::Sample> &samples,
                           tinyhtm::Spherical &center, tinyhtm::Spherical &size);

void read_input(json5_parser::mValue &json5, const std::string &arg,
                std::string &output_prefix, std::string &input_file,
                boost::filesystem::path &drf_file,
                std::map<std::string,std::string> &columns,
                Coordinate_Frame &coordinate_frame,
                std::set<int> &iterations,
                bool &generate_beams,
                std::string &boost_function_string,
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
      boost::filesystem::path drf_file("share/hires/beams");
      std::map<std::string,std::string> columns={{"ra","ra"},{"dec","dec"},
                                                 {"signal","tsky"},
                                                 {"psi","psi"}};
      Coordinate_Frame coordinate_frame=Coordinate_Frame::ICRS;
      bool generate_beams=false;
      double angResolution;
      std::string boost_function_string;
      std::set<int> iterations;
      std::unique_ptr<tinyhtm::Shape> shape;
      std::vector<std::pair<std::string, std::pair<std::string,
                                                   std::string> > > keywords;
      read_input(json5,argument,output_prefix,input_file,drf_file,
                 columns,coordinate_frame,iterations,generate_beams,
                 boost_function_string,shape,keywords,
                 angResolution);

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
          samples.emplace_back(get_sample_from_query(input_file, shape,
                                                     projection));
        }
      else
        {
          get_sample_from_table(path, columns, shape.operator bool(),
                                samples, center, size);
        }

      std::array<int,2> nxy{{static_cast<int>(size.lon()/angResolution),
            static_cast<int>(size.lat()/angResolution)}};
      std::array<double,2> crval{{center.lon(),center.lat()}};

      constexpr double pi=atan(1.0)*4;
      hires::Hires hires (nxy,crval,angResolution*pi/180,generate_beams,
                          drf_file,boost_function_string,keywords,samples);
      hires.init ();
      hires.write_output (hires::Hires::Image_Type::all, output_prefix);
      const size_t iter_max=*iterations.rbegin();
      while (hires.iteration <= iter_max)
        {
          hires.iterate (false);
          if (iterations.find (hires.iteration) != iterations.end ())
            hires.write_output (hires::Hires::Image_Type::all, output_prefix);
        }
    }
  catch (std::runtime_error &e)
    {
      std::cerr << "ERROR: " << e.what () << "\n";
      exit (1);
    }
}
