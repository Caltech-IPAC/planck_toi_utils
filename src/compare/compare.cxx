#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <stdexcept>
#include <math.h>
#include <iomanip>

#include <H5Cpp.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "../lzf/lzf_filter.h"

struct raw_entry
{
  float glon, glat, psi;
  int32_t healpix_2048;
  float tsky;
  double utc;
  unsigned char sso;

  bool operator<(const raw_entry &p) const
  {
    return utc < p.utc;
  }
};

void read_raw_hdf5(const std::string &filename,
                   std::vector<raw_entry> &raw_entries)
{
  boost::filesystem::path path(filename);

  H5::Exception::dontPrint();
  H5::H5File file(path.string(), H5F_ACC_RDONLY);
  H5::DataSet dataset;
  H5::Group group;

  group = file.openGroup("/");
  H5std_string sstr = group.getObjnameByIdx(0);
  dataset = file.openDataSet(sstr.c_str());

  H5::DataSpace dataspace = dataset.getSpace();
  hsize_t size;
  dataspace.getSimpleExtentDims(&size, NULL);
  raw_entries.resize(size);
                
  H5::CompType compound(sizeof(raw_entry));
  compound.insertMember("glon",HOFFSET(raw_entry,glon),
                        H5::PredType::NATIVE_FLOAT);
  compound.insertMember("glat",HOFFSET(raw_entry,glat),
                        H5::PredType::NATIVE_FLOAT);
  compound.insertMember("psi",HOFFSET(raw_entry,psi),
                        H5::PredType::NATIVE_FLOAT);
  compound.insertMember("healpix_2048",
                        HOFFSET(raw_entry,healpix_2048),
                        H5::PredType::NATIVE_INT32);
  compound.insertMember("tsky",HOFFSET(raw_entry,tsky),
                        H5::PredType::NATIVE_FLOAT);
  compound.insertMember("utc",HOFFSET(raw_entry,utc),
                        H5::PredType::NATIVE_DOUBLE);
  compound.insertMember("sso",HOFFSET(raw_entry,sso),
                        H5::PredType::NATIVE_UCHAR);

  dataset.read(raw_entries.data(), compound);

  sort(raw_entries.begin(), raw_entries.end());
}



struct indexed_entry
{
  double ra, dec;
  float psi;
  double mjd;
  float tsky;
  double glon, glat;
  unsigned char sso;

  bool operator<(const indexed_entry &p) const
  {
    return mjd < p.mjd;
  }
};

void read_indexed_hdf5(const std::string &filename,
                       std::vector<indexed_entry> &indexed_entries)
{
  boost::filesystem::path path(filename);

  H5::Exception::dontPrint();
  H5::H5File file(path.string(), H5F_ACC_RDONLY);
  H5::DataSet dataset;
  H5::Group group;

  group = file.openGroup("/");
  H5std_string sstr = group.getObjnameByIdx(0);
  dataset = file.openDataSet(sstr.c_str());

  H5::DataSpace dataspace = dataset.getSpace();
  hsize_t size;
  dataspace.getSimpleExtentDims(&size, NULL);
  indexed_entries.resize(size);
                
  H5::CompType compound(sizeof(indexed_entry));
  compound.insertMember("ra",HOFFSET(indexed_entry,ra),
                        H5::PredType::NATIVE_DOUBLE);
  compound.insertMember("dec",HOFFSET(indexed_entry,dec),
                        H5::PredType::NATIVE_DOUBLE);
  compound.insertMember("psi",HOFFSET(indexed_entry,psi),
                        H5::PredType::NATIVE_FLOAT);
  compound.insertMember("mjd",HOFFSET(indexed_entry,mjd),
                        H5::PredType::NATIVE_DOUBLE);
  compound.insertMember("tsky",HOFFSET(indexed_entry,tsky),
                        H5::PredType::NATIVE_FLOAT);
  compound.insertMember("glon",HOFFSET(indexed_entry,glon),
                        H5::PredType::NATIVE_DOUBLE);
  compound.insertMember("glat",HOFFSET(indexed_entry,glat),
                        H5::PredType::NATIVE_DOUBLE);
  compound.insertMember("sso",HOFFSET(indexed_entry,sso),
                        H5::PredType::NATIVE_UCHAR);

  dataset.read(indexed_entries.data(), compound);

  sort(indexed_entries.begin(), indexed_entries.end());
}


int main(int argc, char *argv[])
{
  if(argc!=3)
    {
      std::cout << "compare <raw_hdf5> <indexed_hdf5>\n";
      exit(1);
    }
  try
    {
      register_lzf();
      std::vector<raw_entry> raw_entries;
      read_raw_hdf5(argv[1],raw_entries);

      std::vector<indexed_entry> indexed_entries;
      read_indexed_hdf5(argv[2],indexed_entries);

      if(indexed_entries.size()!=raw_entries.size())
        throw std::runtime_error("Sizes are different\n\traw: "
                                 + std::to_string(raw_entries.size())
                                 + "\n\tindexed: "
                                 + std::to_string(indexed_entries.size()));
      double max_error=0;
      float min_glon=1e7, max_glon=-1e7, min_glat=1e7, max_glat=-1e7;
      for(size_t i=0; i<indexed_entries.size(); ++i)
        {
          min_glon=std::min(min_glon,raw_entries[i].glon);
          min_glat=std::min(min_glat,raw_entries[i].glat);
          max_glon=std::max(max_glon,raw_entries[i].glon);
          max_glat=std::max(max_glat,raw_entries[i].glat);

          double glon_error=
            3600*std::abs(raw_entries[i].glon-indexed_entries[i].glon);
          if(glon_error>3600*359 && glon_error<3600*361)
            glon_error-=3600*360;
          double glat_error=
            3600*std::abs(raw_entries[i].glat-indexed_entries[i].glat);

          max_error=std::max(max_error,glon_error);
          max_error=std::max(max_error,glat_error);
        }
      std::cout << "max error in arcseconds: " << max_error << "\n";
      std::cout << "glon: " << min_glon << " " << max_glon << "\n";
      std::cout << "glat: " << min_glat << " " << max_glat << "\n";
    }
  catch (H5::Exception &e)
    {
      std::cerr << "HDF5 Exception: "
                << e.getDetailMsg() << "\n";
      exit(1);
    }
  catch (std::runtime_error& e) 
    {
      std::cerr << "Error: "
                << e.what() << "\n";
      exit(1);
    }
}
