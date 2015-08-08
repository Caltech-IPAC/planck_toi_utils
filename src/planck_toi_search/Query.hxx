#pragma once

#include <vector>
#include <list>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <tuple>

#include <hdf5.h>

#include "tinyhtm/Query.hxx"
#include "Results.hxx"

class Query
{
public:
  Results results;
  std::list<std::pair<double, double> > &time_intervals;
  size_t x_offset, y_offset, z_offset, psi_offset, mjd_offset, signal_offset,
      sso_offset;
  bool callback (const char *entry);

  Query (std::list<std::pair<double, double> > &t,
         const std::string &data_file)
      : time_intervals (t), x_offset (-1), y_offset (-1), z_offset (-1),
        psi_offset (-1), mjd_offset (-1), signal_offset (-1), sso_offset (-1)
  {
    const size_t offsets[] = { 0, 4, 8, 12, 16, 24, 28 };
    tinyhtm::Tree tree (data_file);

    size_t index = 0;
    for (auto &name : tree.tree.element_names)
      {
        if (name == "PSI")
          {
            psi_offset = offsets[index];
          }
        else if (name == "MJD")
          {
            mjd_offset = offsets[index];
          }
        else if (name == "SIGNAL")
          {
            signal_offset = offsets[index];
          }
        else if (name == "SSO")
          {
            sso_offset = offsets[index];
          }
        else if (name == "x")
          {
            x_offset = offsets[index];
          }
        else if (name == "y")
          {
            y_offset = offsets[index];
          }
        else if (name == "z")
          {
            z_offset = offsets[index];
          }
        else
          {
            throw std::runtime_error ("In file: " + data_file
                                      + "\n\tBad column name: " + name);
          }
        ++index;
      }
    if (x_offset == static_cast<size_t>(-1))
      throw std::runtime_error ("Missing column 'x' in HTM file");
    if (y_offset == static_cast<size_t>(-1))
      throw std::runtime_error ("Missing column 'y' in HTM file");
    if (z_offset == static_cast<size_t>(-1))
      throw std::runtime_error ("Missing column 'z' in HTM file");
    if (mjd_offset == static_cast<size_t>(-1))
      throw std::runtime_error ("Missing column 'MJD' in HTM file");
    if (psi_offset == static_cast<size_t>(-1))
      throw std::runtime_error ("Missing column 'PSI' in HTM file");
    if (signal_offset == static_cast<size_t>(-1))
      throw std::runtime_error ("Missing column 'SIGNAL' in HTM file");
    if (sso_offset == static_cast<size_t>(-1))
      throw std::runtime_error ("Missing column 'SSO' in HTM file");
  }
};
