#include <memory>
#include <string>
#include <map>
#include <stdexcept>
#include <math.h>
#include <array>

#include <H5Cpp.h>

#include <CCfits/CCfits>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "wcs.h"

#include "lzf/lzf_filter.h"
#include "tinyhtm/sort_and_index.hxx"

struct planck_tod_entry
{
  int64_t htmid;
  float psi;
  double mjd;
  float tsky;
  unsigned char sso;
  htm_sc sc;

  typedef float vector_type;
  static constexpr size_t num_elements = 4;
  static constexpr size_t data_size = 17;

  static std::array<std::string, num_elements> names;
  static std::array<H5::DataType, num_elements> types;

  const void *data (const size_t i) const
  {
    assert (i < num_elements);
    switch (i)
      {
      case 0:
        return reinterpret_cast<const void *>(&psi);
      case 1:
        return reinterpret_cast<const void *>(&mjd);
      case 2:
        return reinterpret_cast<const void *>(&tsky);
      case 3:
        return reinterpret_cast<const void *>(&sso);
      }
    /// Keep the compiler happy so that it does not warn about no
    /// return statement.
    return nullptr;
  }

  bool operator<(const planck_tod_entry &p) const
  {
    return (htmid < p.htmid || (htmid == p.htmid && tsky < p.tsky));
  }
} HTM_ALIGNED (16);

struct planck_hdf5_entry
{
  float glon, glat, psi;
  int32_t healpix_2048;
  float tsky;
  double utc;
  unsigned char sso;
};

std::array<std::string, planck_tod_entry::num_elements>
planck_tod_entry::names{ { "PSI", "MJD", "TSKY", "SSO" } };

std::array<H5::DataType, planck_tod_entry::num_elements>
planck_tod_entry::types{
  { H5::PredType::NATIVE_FLOAT, H5::PredType::NATIVE_DOUBLE,
    H5::PredType::NATIVE_FLOAT, H5::PredType::NATIVE_UCHAR }
};

int main (int argc, char *argv[])
{
   const size_t memsz = 210*1024*1024*1024L;
  // const size_t memsz = sizeof(planck_tod_entry)*2*16*64*1024;
  // const size_t memsz = sizeof(planck_tod_entry)*2*16*64*1024*1024;
  const size_t ioblksz(sizeof(planck_tod_entry)*32*1024);
  mem_params mem(memsz, ioblksz);
  const int htm_depth(20);
// attributes
  int nattr(0);
  std::vector<H5::Attribute> avec;

  if (argc < 3)
    {
      std::cout << "planck_to_htm <out_path> <in_file_1> <in_file_2> ...\n";
      exit (1);
    }
  std::string out_path (argv[1]);
  std::string data_file (out_path + ".h5"), scratch_file (out_path + ".scr"),
      htm_file (out_path + ".htm");

  try
  {
    register_lzf ();
    size_t npoints (0);
    auto start_time = std::chrono::high_resolution_clock::now ();
    {
      blk_writer<planck_tod_entry> out (data_file, mem.sortsz);
      for (int arg = 2; arg < argc; ++arg)
        {
          boost::filesystem::path path (argv[arg]);
          std::cout << "Reading " << path.string () << std::endl;
          if (boost::iequals (path.extension ().string (), ".fits"))
            {
              CCfits::FITS fits_file (argv[arg], CCfits::Read, false);

              CCfits::ExtHDU &img = fits_file.extension (1);

              CCfits::BinTable *image (dynamic_cast<CCfits::BinTable *>(&img));

              std::map<std::string, CCfits::Column *> &columns (
                  image->column ());

              CCfits::ColumnData<double> *tsky_column
                  = dynamic_cast<CCfits::ColumnData<double> *>(
                      columns["TSKY"]);
              CCfits::ColumnData<double> *glon_column
                  = dynamic_cast<CCfits::ColumnData<double> *>(
                      columns["GLON"]);
              CCfits::ColumnData<double> *glat_column
                  = dynamic_cast<CCfits::ColumnData<double> *>(
                      columns["GLAT"]);
              CCfits::ColumnData<long long> *utc_column
                  = dynamic_cast<CCfits::ColumnData<long long> *>(
                      columns["UTC"]);
              CCfits::ColumnData<short> *ring_column
                  = dynamic_cast<CCfits::ColumnData<short> *>(columns["RING"]);

              if (tsky_column == nullptr)
                throw std::runtime_error ("Can not open TSKY column");
              if (glon_column == nullptr)
                throw std::runtime_error ("Can not open GLON column");
              if (glat_column == nullptr)
                throw std::runtime_error ("Can not open GLAT column");
              if (utc_column == nullptr)
                throw std::runtime_error ("Can not open UTC column");
              if (ring_column == nullptr)
                throw std::runtime_error ("Can not open RING column");

              std::vector<std::string> keys;
              keys.push_back ("TSKY");
              keys.push_back ("GLON");
              keys.push_back ("GLAT");
              keys.push_back ("UTC");
              keys.push_back ("RING");
              image->readData (true, keys);

              const size_t num_points_in_file (tsky_column->data ().size ());
              npoints += num_points_in_file;
              for (size_t i = 0; i < num_points_in_file; ++i)
                {
                  planck_tod_entry entry;
                  /* CCfits data starts at 1, not 0 */
                  if (htm_sc_init (&entry.sc, glon_column->data (i + 1),
                                   glat_column->data (i + 1)) != HTM_OK)
                    {
                      std::stringstream ss;
                      ss << "Bad latitude or longitude in record: " << i
                         << " lon: " << glon_column->data (i + 1)
                         << " lat: " << glat_column->data (i + 1) << "\n";
                      throw std::runtime_error (ss.str ());
                    }
                  struct htm_v3 v;
                  if (htm_sc_tov3 (&v, &entry.sc) != HTM_OK)
                    {
                      std::stringstream ss;
                      ss << "Could not convert lon/lat to a vector in record: "
                         << i << " lon: " << glon_column->data (i + 1)
                         << " lat: " << glat_column->data (i + 1) << "\n";
                      throw std::runtime_error (ss.str ());
                    }
                  entry.htmid = htm_v3_id (&v, htm_depth);
                  out.append (&entry);
                }
            }
          else if (boost::iequals (path.extension ().string (), ".hdf")
                   || boost::iequals (path.extension ().string (), ".h5")
                   || boost::iequals (path.extension ().string (), ".hdf5"))
            {
              H5::Exception::dontPrint ();
              H5::H5File file (path.string (), H5F_ACC_RDONLY);
              H5::DataSet dataset;
              H5::Group group;
              try
              {
                group = file.openGroup ("/");
                H5std_string sstr = group.getObjnameByIdx (0);
                std::cout << sstr.c_str () << "\n";
                dataset = file.openDataSet (sstr.c_str ());
              }
              catch (H5::Exception &e)
              {
                H5::Exception::dontPrint();
                H5::H5File file(path.string(), H5F_ACC_RDONLY);
                H5::DataSet dataset;
                H5::Group group;
                try {
                      group = file.openGroup("/");
                      H5std_string sstr = group.getObjnameByIdx(0);
                      std::cout << sstr.c_str() << "\n";
                      dataset = file.openDataSet(sstr.c_str());

                }
                catch (H5::Exception &e)
                {
                    std::cout << "Warning: failed to find any dataset in "
                              << "specified HDF5 file." << std::endl;
                    continue;
                }

// Grab the attributes from the first successful file read
                if (npoints == 0) {
                    nattr = group.getNumAttrs();
                    std::cout << "N attr = " << nattr << "\n";
                    avec.resize(nattr);
                    for (int i = 0; i < nattr; i++) {
                       H5::Attribute a = group.openAttribute(i);
                       std::cout << i << " " << a.getName() << "\n";
                       avec[i] = a;
                      }
                }
//
                H5::DataSpace dataspace = dataset.getSpace();
                hsize_t size;
                dataspace.getSimpleExtentDims(&size, NULL);
                npoints+=size;

                planck_tod_entry foo1;
                std::cout << "sizeof tod = " << sizeof(foo1) << "\n";

                htm_entry<planck_tod_entry> foo2;
                std::cout << "sizeof htm_entry<tod> = " << sizeof(foo2) << "\n";

                std::cout << "npoints = " << npoints << "\n";

                std::vector<planck_hdf5_entry> hdf_entries(size);
                
                H5::CompType compound(sizeof(planck_hdf5_entry));
                compound.insertMember("glon",HOFFSET(planck_hdf5_entry,glon),
                                      H5::PredType::NATIVE_FLOAT);
                compound.insertMember("glat",HOFFSET(planck_hdf5_entry,glat),
                                      H5::PredType::NATIVE_FLOAT);
                compound.insertMember("psi",HOFFSET(planck_hdf5_entry,psi),
                                      H5::PredType::NATIVE_FLOAT);
                compound.insertMember("healpix_2048",
                                      HOFFSET(planck_hdf5_entry,healpix_2048),
                                      H5::PredType::NATIVE_INT32);
                compound.insertMember("tsky",HOFFSET(planck_hdf5_entry,tsky),
                                      H5::PredType::NATIVE_FLOAT);
                compound.insertMember("utc",HOFFSET(planck_hdf5_entry,utc),
                                      H5::PredType::NATIVE_DOUBLE);
                compound.insertMember("sso",HOFFSET(planck_hdf5_entry,sso),
                                      H5::PredType::NATIVE_UCHAR);

                try {
                    dataset.read(hdf_entries.data(), compound);
                }
                catch (H5::Exception &e)
                {
                    std::cout << "Warning:: Failed to read dataset in "
                              << "specified HDF5 file." << std::endl;
                }

                long nentries(0);

                for(auto &hdf_entry: hdf_entries)
                  {
                    nentries++;
                    planck_tod_entry entry;

                    entry.psi = hdf_entry.psi;
                    const double MJD_1958_01_01=36204.0;
                    entry.mjd = MJD_1958_01_01 + hdf_entry.utc / 1.0e9 / 86400.0;
                    entry.tsky = hdf_entry.tsky;
                    entry.sso = hdf_entry.sso;

                    double ra=hdf_entry.glon;
                    double dec=hdf_entry.glat;
                    wcscon (WCS_GALACTIC, WCS_J2000, 0, 0, &ra, &dec, 1950);
                    if(htm_sc_init(&entry.sc, ra, dec)!= HTM_OK)
                      {
                        std::stringstream ss;
                        ss << "Bad latitude or longitude in record: "
                           << " lon: " << hdf_entry.glon
                           << " lat: " << hdf_entry.glat
                           << "\n";
                        throw std::runtime_error(ss.str());
                      }
                    struct htm_v3 v;
                    if (htm_sc_tov3(&v, &entry.sc) != HTM_OK)
                      {
                        std::stringstream ss;
                        ss << "Could not convert lon/lat to a vector in record: "
                           << " lon: " << hdf_entry.glon
                           << " lat: " << hdf_entry.glat
                           << "\n";
                        throw std::runtime_error(ss.str());
                      }
                    entry.htmid=htm_v3_id(&v,htm_depth);
                    out.append(&entry);
                  }
                  std::cout << "# entries processed = " << nentries << "\n";
              }
              H5::DataSpace dataspace = dataset.getSpace ();
              hsize_t size;
              dataspace.getSimpleExtentDims (&size, NULL);
              npoints += size;

              planck_tod_entry foo1;
              std::cout << "sizeof tod = " << sizeof(foo1) << "\n";

              htm_entry<planck_tod_entry> foo2;
              std::cout << "sizeof htm_entry<tod> = " << sizeof(foo2) << "\n";

              std::cout << "npoints = " << npoints << "\n";

              std::vector<planck_hdf5_entry> hdf_entries (size);

              H5::CompType compound (sizeof(planck_hdf5_entry));
              compound.insertMember ("glon", HOFFSET (planck_hdf5_entry, glon),
                                     H5::PredType::NATIVE_FLOAT);
              compound.insertMember ("glat", HOFFSET (planck_hdf5_entry, glat),
                                     H5::PredType::NATIVE_FLOAT);
              compound.insertMember ("psi", HOFFSET (planck_hdf5_entry, psi),
                                     H5::PredType::NATIVE_FLOAT);
              compound.insertMember ("healpix_2048",
                                     HOFFSET (planck_hdf5_entry, healpix_2048),
                                     H5::PredType::NATIVE_INT32);
              compound.insertMember ("tsky", HOFFSET (planck_hdf5_entry, tsky),
                                     H5::PredType::NATIVE_FLOAT);
              compound.insertMember ("utc", HOFFSET (planck_hdf5_entry, utc),
                                     H5::PredType::NATIVE_DOUBLE);
              compound.insertMember ("sso", HOFFSET (planck_hdf5_entry, sso),
                                     H5::PredType::NATIVE_UCHAR);

              try { dataset.read (hdf_entries.data (), compound); }
              catch (H5::Exception &e)
              {
                std::cout << "Warning:: Failed to read dataset in "
                          << "specified HDF5 file." << std::endl;
              }

              long nentries (0);

              for (auto &hdf_entry : hdf_entries)
                {
                  nentries++;
                  planck_tod_entry entry;

                  entry.psi = hdf_entry.psi;
                  const double MJD_1958_01_01 = 36204.0;
                  entry.mjd = MJD_1958_01_01 + hdf_entry.utc / 1.0e9 / 86400.0;
                  entry.tsky = hdf_entry.tsky;
                  entry.sso = hdf_entry.sso;

                  double ra = hdf_entry.glon;
                  double dec = hdf_entry.glat;
                  wcscon (WCS_GALACTIC, WCS_J2000, 0, 0, &ra, &dec, 1950);
                  if (htm_sc_init (&entry.sc, ra, dec) != HTM_OK)
                    {
                      std::stringstream ss;
                      ss << "Bad latitude or longitude in record: "
                         << " lon: " << hdf_entry.glon
                         << " lat: " << hdf_entry.glat << "\n";
                      throw std::runtime_error (ss.str ());
                    }
                  struct htm_v3 v;
                  if (htm_sc_tov3 (&v, &entry.sc) != HTM_OK)
                    {
                      std::stringstream ss;
                      ss << "Could not convert lon/lat to a vector in record: "
                         << " lon: " << hdf_entry.glon
                         << " lat: " << hdf_entry.glat << "\n";
                      throw std::runtime_error (ss.str ());
                    }
                  entry.htmid = htm_v3_id (&v, htm_depth);
                  out.append (&entry);
                }
              std::cout << "# entries processed = " << nentries << "\n";
            }
          else
            {
              throw std::runtime_error ("Unknown file format: "
                                        + path.string ());
            }
        }
    }

    std::chrono::milliseconds msecs (
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now () - start_time));
    std::cout << "Read files "
              << "\t" << msecs.count () << " msec total\n";

      const size_t minpoints = 1024;
      const uint64_t leafthresh = 64;
      sort_and_index<planck_tod_entry>(data_file, scratch_file,
                                       htm_file,mem,npoints,
                                       minpoints,leafthresh);
// Add attributes to indexed file.
      std::cout << "\nAdding attributes... ";
      H5::H5File file2(data_file, H5F_ACC_RDWR);
      H5::Group group2;
      group2 = file2.openGroup("/");     
      H5::Attribute att;
      std::string str;  	
      for (int i = 0; i < nattr; i++) {
          att = group2.createAttribute(avec[i].getName(), 
                                 avec[i].getDataType(),
                                 avec[i].getSpace());
          avec[i].read(avec[i].getDataType(), str);
          att.write(avec[i].getDataType(), str);
      }
      std::cout << " done.\n";
    }
  catch (H5::Exception &e)
  {
    std::cerr << "HDF5 Exception: " << e.getDetailMsg () << "\n";
    exit (1);
  }
  catch (CCfits::FitsException &e)
  {
    std::cerr << "CCFits Exception: " << e.message () << "\n";
    exit (1);
  }
  catch (std::runtime_error &e)
  {
    std::cerr << "Error: " << e.what () << "\n";
    exit (1);
  }
}
