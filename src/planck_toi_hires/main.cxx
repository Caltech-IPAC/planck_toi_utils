#include "tinyhtm/Query.hxx"
#include "Hires.hxx"
#include "Gnomonic.hxx"

hires::Sample fill_samples(const tinyhtm::Query &query,
                           const hires::Gnomonic &Projection);

int main(int argc, char* argv[])
{
  if(argc<7)
    {
      std::cerr << "Need at least 6 arguments.  Only got " << argc-1 << "\n";
      exit(1);
    }
  try
    {
// htm_file shape vertex_string
      tinyhtm::Query query(argv[4],argv[5],argv[6]);
      const char *hires_mode(argv[2]);

      std::vector<std::string> input_files;
      input_files.push_back(argv[1]);
 //     hires::Hires hires("planck",hires_mode,input_files);
      hires::Hires hires("planck",hires_mode,argv[1]);

      hires::Gnomonic projection(hires.crval1,hires.crval2);
      std::vector<hires::Sample> samples;
      samples.emplace_back(fill_samples(query,projection));

      hires.compute_images(samples,argv[3]);
    }
  catch(std::runtime_error &e)
    {
      std::cerr << e.what() << "\n";
      exit(1);
    }
}
