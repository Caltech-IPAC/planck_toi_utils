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
      std::cerr << "\t1: output file prefix\n";
      std::cerr << "\t2: input TOI file\n";
      std::cerr << "\t3: query type\n";
      std::cerr << "\t4: query parameters\n";
      std::cerr << "\t5: hires iteration output list\n";
      std::cerr << "\t6: parameter definitions\n";
      std::cerr << "\tAt least one parameter definition (PARAM=VAL) required!\n";
      exit(1);
    }
  try
    {
// Process command line arguments
      std::string outfile_prefix(argv[1]);
// htm_file shape vertex_string
      tinyhtm::Query query(argv[2],argv[3],argv[4]);

// Iterations for which to generate output
      int iter_max;   
      std::vector<int> iter_list;
      std::vector<std::string> iter_str;
      boost::split(iter_str,argv[5],boost::is_any_of(",\t "));
      for (auto &n_str: iter_str) {
          if (!n_str.empty()) {
              std::stringstream ns(n_str);
              int n;
              ns >> n;
              iter_list.push_back(n);
          }
      }
      std::sort(iter_list.begin(),iter_list.end());
      iter_list.resize(std::unique(iter_list.begin(),
                                   iter_list.end())-iter_list.begin());
      iter_max = iter_list.back();

// HIRES parameters
      std::vector<std::string> param_str(&(argv[6]), &(argv[argc]));
      hires::Hires_Parameters hp(param_str);
      hires::Hires hires(hp);

// Let's do it!
      hires::Gnomonic projection(hires.crval1,hires.crval2);
      std::vector<hires::Sample> samples;
      samples.emplace_back(fill_samples(query,projection));


      for (int iter=0; iter<=iter_max; ++iter) {
          hires.iterate(iter, samples);
   
          if (iter == 0 || find(iter_list.begin(),iter_list.end(),iter) != iter_list.end()) {
// generate file name here.
              hires.write_output(iter, hires::Image_Type::all, outfile_prefix);
          }
      } 
    }
  catch(std::runtime_error &e)
    {
      std::cerr << e.what() << "\n";
      exit(1);
    }
}
