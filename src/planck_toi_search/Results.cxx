#include <stdexcept>
#include <CCfits/CCfits>

#include "Results.hxx"
#include "tinyhtm.h"

bool Results::callback(void *entry, int num_elements, hid_t *, char **names)
{
  if(mjd_index==static_cast<size_t>(-1))
    {
      for(int i=0;i<num_elements;++i)
        {
          if(names[i]==std::string("PSI"))
            {
              psi_index=i;
            }
          else if(names[i]==std::string("MJD"))
            { 
              mjd_index = i;
            }
          else if(names[i]==std::string("TSKY"))
            {
              tsky_index=i;
            }
          else if(names[i]==std::string("SSO"))
            {
              sso_index=i;
            }
          else if(names[i]==std::string("x"))
            {
              x_index=i;
            }
          else if(names[i]==std::string("y"))
            {
              y_index=i;
            }
          else if(names[i]==std::string("z"))
            {
              z_index=i;
            }
          else
            {
              // FIXME: This will certainly be fatal, since we can not
              // catch exceptions called from threads.
              throw std::runtime_error("Bad column name: "
                                       + std::string(names[i]));
            }
        }
      if(x_index==static_cast<size_t>(-1))
        throw std::runtime_error("Missing column 'x' in HTM file");
      if(y_index==static_cast<size_t>(-1))
        throw std::runtime_error("Missing column 'y' in HTM file");
      if(z_index==static_cast<size_t>(-1))
        throw std::runtime_error("Missing column 'z' in HTM file");
      if(mjd_index==static_cast<size_t>(-1))
        throw std::runtime_error("Missing column 'MJD' in HTM file");
      if(psi_index==static_cast<size_t>(-1))
        throw std::runtime_error("Missing column 'PSI' in HTM file");
      if(tsky_index==static_cast<size_t>(-1))
        throw std::runtime_error("Missing column 'TSKY' in HTM file");
      if(sso_index==static_cast<size_t>(-1))
        throw std::runtime_error("Missing column 'SSO' in HTM file");
    }

  double mjd=*((double *)(entry)+mjd_index);

  bool in_interval(time_intervals.empty());
  for(auto &t: time_intervals)
    {
      if((mjd >= t.first && mjd <= t.second))
        {
          in_interval=true;
          break;
        }
    }
    if(in_interval && !count)
    {
      float x(((float *)(entry))[x_index]),
        y(((float *)(entry))[y_index]),
        z(((float *)(entry))[z_index]),
        psi(((float *)(entry))[psi_index]),
        tsky(((float *)(entry))[tsky_index]),
        sso(((char *)(entry))[sso_index]);
      data.push_back(std::make_tuple(x,y,z,psi,mjd,tsky,sso));
    }
  return in_interval;
}

void Results::write_fits(char * fname)
{
    htm_v3 v3;
    int errcode;
    long start_row;

    std::auto_ptr<CCfits::FITS> pFits(0);
    const std::string fileName(fname);
    pFits.reset(new CCfits::FITS(fileName,  CCfits::Write));

    int size = data.size();
    std::cout << "Number of rows = " << size << "\n";

    htm_sc sc;
    int bufsize(1024);
    double mjd[bufsize];
    float glon[bufsize];
    float glat[bufsize];
    float psi[bufsize];
    float signal[bufsize];
    char sso[bufsize];

    string hduName("TABLE_BINARY");


//
// Need to add handling of SSO object searches - which add
// SSO_GLON and SSO_GLAT columns.
//
    std::vector<string> colName(5);
    std::vector<string> colForm(5);
    std::vector<string> colUnit(5);

    colName[0] = "MJD";
    colName[1] = "GLON";
    colName[2] = "GLAT";
    colName[3] = "PSI";
    colName[4] = "TSKY";  

    colForm[0] = "1D";
    colForm[1] = "1E";
    colForm[2] = "1E";
    colForm[3] = "1E";
    colForm[4] = "1E";

    colUnit[0] = "days";
    colUnit[1] = "degrees";
    colUnit[2] = "degrees";
    colUnit[3] = "degrees";
    colUnit[4] = "Need from header";  

    CCfits::Table *newTable =  pFits->addTable(hduName, 1, colName, colForm, colUnit);

//
// Populate the extension header
//

    newTable->addKey("TELESCOP", "Planck", "Telescope Name");
    newTable->addKey("INSTRUME", "Need from header", "Instrument Name");
    newTable->addKey("DETECTOR", "Need from header", "Detector Name");
    newTable->addKey("FREQ", "Need from header", "GHz");
    newTable->addKey("VERSION", "DX10", "Processing Version");
    newTable->addKey("TARGET", "get from command line", "Target Name");
    newTable->addKey("DATE", "generate from now", "File Creation Date (YYYY-MM-DDThh:mm:ss UT)"); 
    newTable->addKey("OBJ_TYPE", "FIXED", "Fixed or Moving object");
    newTable->addKey("T_GLON", 0.0, "[deg] Search GLON");
    newTable->addKey("T_GLAT", 0.0, "[deg] Search GLAT");
    newTable->addKey("RADIUS", 0.0, "[arcmin] Search radius");
    newTable->addKey("MJDBEG1", 0.0, "Start time of data in MJD for first time range requested");
    newTable->addKey("MJDEND1", 0.0, "End time of data in MJD for first time range requested");

    int i = 0, j = 0;
    int nbuf = 0;

    for (auto &r: data)
    {
         
        v3.x = std::get<0>(r);
        v3.y = std::get<1>(r);
        v3.z = std::get<2>(r);
        psi[i] = std::get<3>(r); 
        mjd[i] = std::get<4>(r);
        signal[i] = std::get<5>(r);
        sso[i] = std::get<6>(r);

        errcode = htm_v3_tosc(&sc, &v3);
        glon[i] = sc.lon;
        glat[i] = sc.lat;

// kludge for now
        
    
        if (++i == bufsize || ++j == size) {
            start_row = nbuf*bufsize + 1;
            newTable->column("MJD").write(mjd, i, start_row);
            newTable->column("GLON").write(glon, i, start_row);
            newTable->column("GLAT").write(glat, i, start_row);
            newTable->column("PSI").write(psi, i, start_row);
            newTable->column("TSKY").write(signal, i, start_row);
            i = 0;
            nbuf++;
        }
    }
}
