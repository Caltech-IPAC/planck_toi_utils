#include <vector>
#include <map>

#include "Hires.hxx"
#include "Exception.hxx"
#include "Gnomonic.hxx"

namespace hires
{
  std::vector<Sample> read_all_IN_files(const Hires::Data_Type &dt,
                                        const std::string &prefix,
                                        const Gnomonic &projection);
}

int main(int argc, char* argv[])
{
  if(argc<6)
    {
      std::cerr << "Usage: hires data_type hires_mode IN_prefix OUT_prefix"
        " param_file1 param_file2 ...\n";
      exit(1);
    }

  std::vector<std::string> args;
  for(int i=5;i<argc;++i)
    args.emplace_back(argv[i]);

  try
    {
      hires::Hires hires(argv[1],argv[2],args);
      hires::Gnomonic projection(hires.crval1,hires.crval2);
      std::vector<hires::Sample> samples(read_all_IN_files(hires.data_type,
                                                           argv[3],
                                                           projection));
      hires.compute_images(samples,argv[4]);
    }
  catch(hires::Exception &e)
    {
      std::cerr << e.what() << "\n";
      exit(1);
    }
}
