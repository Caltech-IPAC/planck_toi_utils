#include "tinyhtm/Query.hxx"
#include "Hires.hxx"
#include "Gnomonic.hxx"
#include <CCfits/CCfits>
#include <boost/filesystem.hpp>

hires::Sample fill_samples (const tinyhtm::Query &query,
                            const hires::Gnomonic &Projection);

hires::Sample fill_samples_from_fits(std::map<std::string, CCfits::Column *> &columns,
                            const hires::Gnomonic &Projection);

void add_keywords_from_fits(string filename_toi, std::vector<string> filename_list);

int main (int argc, char *argv[])
{
  if (argc < 7)
    {
      std::cerr << "Need at least 6 arguments.  Only got " << argc - 1 << "\n";
      std::cerr << "\t1: output file prefix\n";
      std::cerr << "\t2: input TOI file\n";
      std::cerr << "\t3: query type\n";
      std::cerr << "\t4: query parameters\n";
      std::cerr << "\t5: hires iteration output list\n";
      std::cerr << "\t6: parameter definitions\n";
      std::cerr
          << "\tAt least one parameter definition (PARAM=VAL) required!\n";
      exit (1);
    }
  try
  {
    // Process command line arguments
    std::string outfile_prefix (argv[1]);
    boost::filesystem::path path(argv[2]);
    
    // Iterations for which to generate output
    size_t iter_max;
    std::vector<size_t> iter_list;
    std::vector<std::string> iter_str;
    boost::split (iter_str, argv[5], boost::is_any_of (",\t "));
    for (auto &n_str : iter_str)
      {
        if (!n_str.empty ())
          {
            std::stringstream ns (n_str);
            size_t n;
            ns >> n;
            iter_list.push_back (n);
          }
      }
    std::sort (iter_list.begin (), iter_list.end ());
    iter_list.resize (std::unique (iter_list.begin (), iter_list.end ())
                      - iter_list.begin ());
    iter_max = iter_list.back ();

    // HIRES parameters
    std::vector<std::string> param_str (&(argv[6]), &(argv[argc]));
    hires::Hires hires (param_str);
    hires::Gnomonic projection (hires.crval1, hires.crval2);
    // and the HIRES samples
    std::vector<hires::Sample> samples;

    if (boost::iequals(path.extension().string(), ".hdf") ||
       boost::iequals(path.extension().string(), ".h5") ||
       boost::iequals(path.extension().string(), ".hdf5")) {
    // htm_file shape vertex_string
        tinyhtm::Query query(argv[2], argv[3], argv[4]);
        samples.emplace_back(fill_samples(query, projection));
    } else if (boost::iequals(path.extension().string(), ".fits")) {
    // extracted FITS finary table - no need to query.
        CCfits::FITS fits_file(argv[2], CCfits::Read, false);
        CCfits::ExtHDU &img = fits_file.extension(1);

        CCfits::BinTable *image(dynamic_cast<CCfits::BinTable *>(&img));
        std::map<std::string, CCfits::Column *> &columns(image->column());
        samples.emplace_back(fill_samples_from_fits(columns, projection));
    } else {
        std::cerr << "Invalid file type\n";
        exit (1);
    }

    hires.init (samples);
    std::vector<std::string> outfile_list;
    outfile_list = hires.write_output_worker (hires::Hires::Image_Type::all, outfile_prefix);
    if (boost::iequals(path.extension().string(), ".fits") && outfile_list.size() > 0) {
        add_keywords_from_fits(path.string(), outfile_list);
    } 

    while (hires.iteration <= iter_max)
      {
        hires.iterate (samples);
        if (find (iter_list.begin (), iter_list.end (), hires.iteration)
            != iter_list.end ()) {
          outfile_list = hires.write_output_worker (hires::Hires::Image_Type::all, outfile_prefix);
          if (boost::iequals(path.extension().string(), ".hdf") ||
              boost::iequals(path.extension().string(), ".h5") ||
              boost::iequals(path.extension().string(), ".hdf5")) {
//
// Pending implementation of add_keywords_from_hdf()
//
          } else if (boost::iequals(path.extension().string(), ".fits") && outfile_list.size() > 0) {
              add_keywords_from_fits(path.string(), outfile_list);
          } 
      }
  } 
  }
  catch (std::runtime_error &e)
  {
    std::cerr << e.what () << "\n";
    exit (1);
  }
}
