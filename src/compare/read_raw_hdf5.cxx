#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <H5Cpp.h>

#include "raw_entry.hxx"

void read_raw_hdf5 (const std::string &filename,
                    std::vector<raw_entry> &raw_entries)
{
  boost::filesystem::path path (filename);

  H5::Exception::dontPrint ();
  H5::H5File file (path.string (), H5F_ACC_RDONLY);
  H5::DataSet dataset;
  H5::Group group;

  group = file.openGroup ("/");
  H5std_string sstr = group.getObjnameByIdx (0);
  dataset = file.openDataSet (sstr.c_str ());

  H5::DataSpace dataspace = dataset.getSpace ();
  hsize_t size;
  dataspace.getSimpleExtentDims (&size, NULL);
  raw_entries.resize (size);

  H5::CompType compound (sizeof(raw_entry));
  compound.insertMember ("glon", HOFFSET (raw_entry, glon),
                         H5::PredType::NATIVE_FLOAT);
  compound.insertMember ("glat", HOFFSET (raw_entry, glat),
                         H5::PredType::NATIVE_FLOAT);
  compound.insertMember ("psi", HOFFSET (raw_entry, psi),
                         H5::PredType::NATIVE_FLOAT);
  compound.insertMember ("healpix_2048", HOFFSET (raw_entry, healpix_2048),
                         H5::PredType::NATIVE_INT32);
  compound.insertMember ("tsky", HOFFSET (raw_entry, tsky),
                         H5::PredType::NATIVE_FLOAT);
  compound.insertMember ("utc", HOFFSET (raw_entry, utc),
                         H5::PredType::NATIVE_DOUBLE);
  compound.insertMember ("sso", HOFFSET (raw_entry, sso),
                         H5::PredType::NATIVE_UCHAR);

  dataset.read (raw_entries.data (), compound);

  sort (raw_entries.begin (), raw_entries.end ());
}
