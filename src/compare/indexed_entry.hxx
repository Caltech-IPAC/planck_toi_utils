#pragma once

struct indexed_entry
{
  double ra, dec;
  float psi;
  double mjd;
  float signal;
  double glon, glat;
  unsigned char sso;

  bool operator<(const indexed_entry &p) const { return mjd < p.mjd; }
};
