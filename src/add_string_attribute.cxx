#include <iostream>
#include <string>
#include "H5Cpp.h"

using namespace H5;

// Args are 
//    1) filename
//    2) Attribute name
//    3) Attribute value
//
int main (int argc, char **argv)
{
    if (argc != 4) {
        std::cerr << "Usage:\n";
        std::cerr << "\t$ add_string_attribute <filename> <attribute_name> <attribute_value>\n";
        exit (1);
    }

    try
    {
        Exception::dontPrint();
        H5std_string fname(argv[1]);
        H5std_string attrname(argv[2]);
        H5std_string attrval(argv[3]);

        H5File file(fname, H5F_ACC_RDWR);
        Group group = file.openGroup("/");

        StrType str_type(0, H5T_VARIABLE);
        DataSpace attr_dataspace = DataSpace(H5S_SCALAR);

        Attribute attribute = group.createAttribute(attrname, str_type, 
                                                attr_dataspace);
        attribute.write(str_type, attrval);
    }  


    // catch failure caused by the H5File operations
    catch( DataSpaceIException error )
    {
        error.printError();
        return -1;
    }
    // catch failure caused by the H5File operations
    catch( AttributeIException error )
    {
       error.printError();
       return -1;
    }
    // catch failure caused by the H5File operations
    catch( FileIException error )
    {
       error.printError();
       return -1;
    }
    // catch failure caused by the DataSet operations
    catch( DataSetIException error )
    {
       error.printError();
       return -1;
    }

    return 0;  
}

