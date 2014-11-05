#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <H5Cpp.h>

#include "indexed_entry.hxx"

void read_indexed_hdf5 (const std::string &filename,
                        std::vector<indexed_entry> &indexed_entries)
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
  indexed_entries.resize (size);

  H5::CompType compound (sizeof(indexed_entry));
  compound.insertMember ("ra", HOFFSET (indexed_entry, ra),
                         H5::PredType::NATIVE_DOUBLE);
  compound.insertMember ("dec", HOFFSET (indexed_entry, dec),
                         H5::PredType::NATIVE_DOUBLE);
  compound.insertMember ("psi", HOFFSET (indexed_entry, psi),
                         H5::PredType::NATIVE_FLOAT);
  compound.insertMember ("mjd", HOFFSET (indexed_entry, mjd),
                         H5::PredType::NATIVE_DOUBLE);
  compound.insertMember ("signal", HOFFSET (indexed_entry, signal),
                         H5::PredType::NATIVE_FLOAT);
  compound.insertMember ("glon", HOFFSET (indexed_entry, glon),
                         H5::PredType::NATIVE_DOUBLE);
  compound.insertMember ("glat", HOFFSET (indexed_entry, glat),
                         H5::PredType::NATIVE_DOUBLE);
  compound.insertMember ("sso", HOFFSET (indexed_entry, sso),
                         H5::PredType::NATIVE_UCHAR);

  dataset.read (indexed_entries.data (), compound);

  sort (indexed_entries.begin (), indexed_entries.end ());
}
