#include <iostream>
#include <string>
#include "H5Cpp.h"

using namespace H5;

// Args are
//    1) filename
//    2) Attribute name
//
int main (int argc, char **argv)
{
  if (argc != 3)
    {
      std::cerr << "Usage:\n";
      std::cerr << "\t$ remove_attribute <filename> <attribute_name>\n";
      exit (1);
    }

  try
  {
    Exception::dontPrint ();
    H5std_string fname (argv[1]);
    H5std_string attrname (argv[2]);

    H5File file (fname, H5F_ACC_RDWR);
    Group group = file.openGroup ("/");

    group.removeAttr (attrname);
  }

  // catch failure caused by the H5File operations
  catch (DataSpaceIException error)
  {
    error.printError ();
    return -1;
  }
  // catch failure caused by the H5File operations
  catch (AttributeIException error)
  {
    error.printError ();
    return -1;
  }
  // catch failure caused by the H5File operations
  catch (FileIException error)
  {
    error.printError ();
    return -1;
  }
  // catch failure caused by the DataSet operations
  catch (DataSetIException error)
  {
    error.printError ();
    return -1;
  }

  return 0;
}
