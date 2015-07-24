#include <stdexcept>
#include <CCfits/CCfits>

#include "Results.hxx"
#include "tinyhtm.h"

void Results::write_fits (char *fname)
{
  htm_v3 v3;
  long start_row;

  std::auto_ptr<CCfits::FITS> pFits (0);
  const std::string fileName (fname);
  pFits.reset (new CCfits::FITS (fileName, CCfits::Write));

  int size = data.size ();

  htm_sc sc;
  int bufsize (1024);
  double mjd[bufsize];
  float ra[bufsize];
  float dec[bufsize];
  float psi[bufsize];
  float signal[bufsize];
  int sso[bufsize];

  string hduName ("TABLE_BINARY");

  //
  // Need to add handling of SSO object searches - which add
  // SSO_GLON and SSO_GLAT columns.
  //
  std::vector<string> colName (6);
  std::vector<string> colForm (6);
  std::vector<string> colUnit (6);

  colName[0] = "MJD";
  colName[1] = "RA";
  colName[2] = "DEC";
  colName[3] = "PSI";
  colName[4] = "SIGNAL";
  colName[5] = "SSO";

  colForm[0] = "1D";
  colForm[1] = "1E";
  colForm[2] = "1E";
  colForm[3] = "1E";
  colForm[4] = "1E";
  colForm[5] = "1J";

  colUnit[0] = "days";
  colUnit[1] = "degrees";
  colUnit[2] = "degrees";
  colUnit[3] = "degrees";
  colUnit[4] = "Need from header";
  colUnit[5] = " ";

  CCfits::Table *newTable
      = pFits->addTable (hduName, 1, colName, colForm, colUnit);

  //
  // Populate the extension header
  //

  newTable->addKey ("TELESCOP", "Planck", "Telescope Name");
  newTable->addKey ("INSTRUME", "Need from header", "Instrument Name");
  newTable->addKey ("DETECTOR", "Need from header", "Detector Name");
  newTable->addKey ("FREQ", "Need from header", "GHz");
  newTable->addKey ("VERSION", "DX10", "Processing Version");
  newTable->addKey ("TARGET", "get from command line", "Target Name");
  newTable->addKey ("DATE", "generate from now",
                    "File Creation Date (YYYY-MM-DDThh:mm:ss UT)");
  newTable->addKey ("OBJ_TYPE", "FIXED", "Fixed or Moving object");
  newTable->addKey ("T_RA", 0.0, "[deg] Search RA [J2000]");
  newTable->addKey ("T_DEC", 0.0, "[deg] Search DEC [J2000]");
  newTable->addKey ("RADIUS", 0.0, "[arcmin] Search radius");
  newTable->addKey (
      "MJDBEG1", 0.0,
      "Start time of data in MJD for first time range requested");
  newTable->addKey ("MJDEND1", 0.0,
                    "End time of data in MJD for first time range requested");

  int i = 0, j = 0;
  int nbuf = 0;

  for (auto &r : data)
    {

      v3.x = std::get<0>(r);
      v3.y = std::get<1>(r);
      v3.z = std::get<2>(r);
      psi[i] = std::get<3>(r);
      mjd[i] = std::get<4>(r);
      signal[i] = std::get<5>(r);
      sso[i] = std::get<6>(r);

      // FIXME: Need to check error codes
      htm_v3_tosc (&sc, &v3);
      ra[i] = sc.lon;
      dec[i] = sc.lat;

      // kludge for now

      if (++i == bufsize || ++j == size)
        {
          start_row = nbuf * bufsize + 1;
          newTable->column ("MJD").write (mjd, i, start_row);
          newTable->column ("RA").write (ra, i, start_row);
          newTable->column ("DEC").write (dec, i, start_row);
          newTable->column ("PSI").write (psi, i, start_row);
          newTable->column ("SIGNAL").write (signal, i, start_row);
          newTable->column ("SSO").write (sso, i, start_row);
          i = 0;
          nbuf++;
        }
    }
}
