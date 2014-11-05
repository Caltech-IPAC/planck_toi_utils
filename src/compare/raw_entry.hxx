#pragma once

struct raw_entry
{
  float glon, glat, psi;
  int32_t healpix_2048;
  float signal;
  double utc;
  unsigned char sso;

  bool operator<(const raw_entry &p) const { return utc < p.utc; }
};
