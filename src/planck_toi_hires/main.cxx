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
      std::cerr << "\t1: mode (hires, minimap, both)\n";
      std::cerr << "\t2: output file prefix\n";
      std::cerr << "\t3: input TOI file\n";
      std::cerr << "\t4: query type\n";
      std::cerr << "\t5: query parameters\n";
      std::cerr << "\t6: parameter definitions\n";
      std::cerr << "\tAt least one parameter definition (PARAM=VAL) required!\n";
      exit(1);
    }
  try
    {
// htm_file shape vertex_string
      tinyhtm::Query query(argv[3],argv[4],argv[5]);
      const char *hires_mode(argv[1]);

      std::vector<std::string> param_str(&(argv[6]), &(argv[argc]));

      hires::Hires_Parameters hp("planck", hires_mode, param_str);
      hires::Hires hires(hp);

      hires::Gnomonic projection(hires.crval1,hires.crval2);
      std::vector<hires::Sample> samples;
      samples.emplace_back(fill_samples(query,projection));

      arma::mat wgt_image;
      std::map<int,arma::mat> flux_images;
      std::map<int,arma::mat> cfv_images;
      std::map<int,arma::mat> beam_images;
      std::string outfile_prefix(argv[2]);

      for (int iter=0; iter<=hires.iter_max; ++iter) {
          hires.iterate(wgt_image, flux_images, cfv_images, beam_images, 
                  samples, iter);
   
          if (iter == 0 || find(hires.iter_list.begin(),hires.iter_list.end(),iter) != hires.iter_list.end()) {
              hires.write_output(wgt_image, flux_images, cfv_images, beam_images, 
                           iter, outfile_prefix);
          }
      } 
    }
  catch(std::runtime_error &e)
    {
      std::cerr << e.what() << "\n";
      exit(1);
    }
}
