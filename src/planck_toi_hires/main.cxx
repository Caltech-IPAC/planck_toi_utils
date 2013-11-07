#include "tinyhtm/Query.hxx"
#include "Hires.hxx"
#include "Gnomonic.hxx"

hires::Sample fill_samples(const tinyhtm::Query &query,
                           const hires::Gnomonic &Projection);

int main(int argc, char* argv[])
{
  if(argc<6)
    {
      std::cerr << "Need at least 5 arguments.  Only got " << argc-1 << "\n";
      exit(1);
    }
  try
    {
      tinyhtm::Query query(argv[3],argv[4],argv[5]);

      std::vector<std::string> input_files;
      input_files.push_back(argv[1]);
      hires::Hires hires("planck",input_files);

      hires::Gnomonic projection(hires.crval1,hires.crval2);
      std::vector<hires::Sample> samples;
      samples.emplace_back(fill_samples(query,projection));

      hires.compute_images(samples,argv[2]);
    }
  catch(std::runtime_error &e)
    {
      std::cerr << e.what() << "\n";
      exit(1);
    }
}
