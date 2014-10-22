#include <set>
#include <fstream>
#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "tinyhtm/Spherical.hxx"
#include "tinyhtm/Shape.hxx"
#include "tinyhtm/Circle.hxx"
#include "tinyhtm/Box.hxx"
#include "tinyhtm/Polygon.hxx"
#include "Hires.hxx"

#include "json5_parser.h"

#include "Coordinate_Frame.hxx"

tinyhtm::Spherical extract_position(const json5_parser::mValue &value)
{
  auto &pos=value.get_array();
  if(pos.size()!=2)
    throw std::runtime_error("Wrong number of elements in an array.  Expected "
                             "2 but got " + std::to_string(pos.size()));
  if(pos[0].type()!=json5_parser::real_type
     || pos[1].type()!=json5_parser::real_type)
    throw std::runtime_error("Expected a number in an array");
  return tinyhtm::Spherical(pos[0].get_real(),pos[1].get_real());
}

void read_input(json5_parser::mValue &json5, const std::string &arg,
                std::string &output_prefix, std::string &input_file,
                boost::filesystem::path &drf_file,
                std::set<hires::Hires::Image_Type> &output_types,
                std::map<std::string,std::string> &columns,
                Coordinate_Frame &coordinate_frame,
                std::set<int> &iterations,
                std::string &boost_function_string,
                std::unique_ptr<tinyhtm::Shape> &shape,
                std::vector<std::pair<std::string, std::pair<std::string,
                                                             std::string> > >
                &keywords,
                double &angResolution)
{
  try
    {
      const std::string query("--query=");
      if(arg=="-")
        {
          read_or_throw(std::cin,json5);
        }
      else if(arg.substr(0,query.size())==query)
        {
          read_or_throw(arg.substr(query.size()),json5);
        }
      else
        {
          std::ifstream infile(arg);
          read_or_throw(infile,json5);
        }
    }
  catch (json5_parser::Error_position &e)
    {
      std::cerr << "Invalid JSON5 at line " << e.line_ << ", column "
                << e.column_ << "\n" << e.reason_ << "\n";
    }

  if(json5.type()!=json5_parser::obj_type)
    throw std::runtime_error("Invalid json5 type.  It must be an object.");

  angResolution=0;
  bool running_hires(false);
  for(auto &v: json5.get_obj())
    {
      if(v.first=="output")
        {
          if(v.second.type()!=json5_parser::obj_type)
            throw std::runtime_error("Expected an object for 'output'");
          for(auto &o: v.second.get_obj ())
            {
              if(o.first=="prefix")
                {
                  if(o.second.type()!=json5_parser::str_type)
                    throw std::runtime_error("Expected a string for "
                                             "'output.prefix'");
                  output_prefix=o.second.get_str();
                }
              else if(o.first=="minimap")
                {
                  if(o.second.type()!=json5_parser::array_type)
                    throw std::runtime_error("Expected an array for "
                                             "'output.minimap'");
                  for(auto &m: o.second.get_array ())
                    {
                      if(m.type()!=json5_parser::str_type)
                        throw std::runtime_error("Expected only strings in the "
                                                 "'output.minimap' array");
                      if(m.get_str()=="image")
                        output_types.insert(hires::Hires::Image_Type
                                            ::minimap_image);
                      else if(m.get_str()=="hitmap")
                        output_types.insert(hires::Hires::Image_Type
                                            ::minimap_hitmap);
                      else
                        throw std::runtime_error("Expected either 'image' or "
                                                 "'hitmap' in 'output.minimap'");
                    }
                }
              else if(o.first=="hires")
                {
                  running_hires=true;
                  if(o.second.type()!=json5_parser::array_type)
                    throw std::runtime_error("Expected an array for "
                                             "'output.hires'");
                  for(auto &m: o.second.get_array ())
                    {
                      if(m.type()!=json5_parser::str_type)
                        throw std::runtime_error("Expected only strings in the "
                                                 "'output.hires' array");
                      if(m.get_str()=="image")
                        output_types.insert(hires::Hires::Image_Type
                                            ::hires_image);
                      else if(m.get_str()=="covariance")
                        output_types.insert(hires::Hires::Image_Type
                                            ::hires_covariance);
                      else
                        throw std::runtime_error
                          ("Expected either 'image', 'covariance', "
                           "'correction', or 'beam' in 'output.hires'");
                    }
                }
            }

        }
      else if(v.first=="input_file")
        {
          if(v.second.type()!=json5_parser::str_type)
            throw std::runtime_error("Expected a string for 'input_file'");
          input_file=v.second.get_str();
        }
      else if(v.first=="drf_file")
        {
          if(v.second.type()!=json5_parser::str_type)
            throw std::runtime_error("Expected a string for 'drf_file'");
          drf_file=v.second.get_str();
        }
      else if(v.first=="iterations")
        {
          if(v.second.type()==json5_parser::int_type)
            {
              iterations.insert(v.second.get_int());
            }
          else if(v.second.type()==json5_parser::array_type)
            {
              for(auto &d: v.second.get_array())
                {
                  iterations.insert(d.get_int());
                }
            }
          else
            {
              throw std::runtime_error("Expected an int or an array of int's "
                                       "for 'iterations'");
            }
        }
      else if(v.first=="boost_function")
        {
          if(v.second.type()!=json5_parser::str_type)
            throw std::runtime_error("Expected a string for 'boost_function'");
          boost_function_string=v.second.get_str();
        }
      else if(v.first=="columns")
        {
          if(v.second.type()!=json5_parser::obj_type)
            throw std::runtime_error("Expected an object for 'columns'");
          auto &columns_object=v.second.get_obj();
          for(auto &c: columns_object)
            {
              std::string key=c.first;
              if(columns.find(key)==columns.end())
                throw std::runtime_error("Unknown key in columns: " + key);

              if(c.second.type()!=json5_parser::str_type)
                throw std::runtime_error("Expected a string for 'columns."
                                         + key + "'");
              columns[key]=c.second.get_str();
            }
        }
      else if(v.first=="pos")
        {
          if(v.second.type()!=json5_parser::obj_type)
            throw std::runtime_error("Expected an object for 'pos'");
          auto &p=v.second.get_obj();
          for(auto &element: p)
            {
              if(element.first=="shape")
                {
                  if(v.second.type()!=json5_parser::obj_type)
                    throw std::runtime_error("Expected an object for 'pos.shape'");
                  auto &s=element.second.get_obj();

                  auto type_iterator=s.find("type");
                  if(type_iterator==s.end())
                    throw std::runtime_error("In 'pos', expected 'type'");

                  if(type_iterator->second.type()!=json5_parser::str_type)
                    throw std::runtime_error("Expected a string for 'pos.shape.type'");
                  auto &type=type_iterator->second.get_str();
                  if(type=="circle")
                    {
                      try
                        {
                          shape=std::unique_ptr<tinyhtm::Circle>
                            (new tinyhtm::Circle(extract_position(s.at("center")),
                                                 s.at("r").get_real()));
                        }
                      catch (std::runtime_error &err)
                        {
                          throw std::runtime_error("In 'pos.shape.circle': "
                                                   + std::string(err.what()));
                        }
                    }
                  else if(type=="box")
                    {
                      try
                        {
                          shape=std::unique_ptr<tinyhtm::Box>
                            (new tinyhtm::Box(extract_position(s.at("center")),
                                              extract_position(s.at("size"))));
                        }
                      catch (std::runtime_error &err)
                        {
                          throw std::runtime_error("In 'pos.shape.box': "
                                                   + std::string(err.what()));
                        }
                    }
                  else if(type=="polygon")
                    {
                      try
                        {
                          std::vector<tinyhtm::Spherical> vertices;
                          for(auto &vertex: s.at("vertices").get_array())
                            vertices.push_back(extract_position(vertex));
                          shape=std::unique_ptr<tinyhtm::Polygon>
                            (new tinyhtm::Polygon(vertices));
                        }
                      catch (std::runtime_error &err)
                        {
                          throw std::runtime_error("In 'pos.shape.polygon': "
                                                   + std::string(err.what()));
                        }
                    }
                  else
                    {
                      throw std::runtime_error("Unknown value for "
                                               "pos.shape.type: "
                                               + type
                                               + "\nExpected cirlce, box, "
                                               "or polygon");
                    }
                }
              else if(element.first=="frame")
                {
                  if(element.second.type()!=json5_parser::str_type)
                    throw std::runtime_error("Expected a string for 'pos.frame'");

                  std::string cframe=element.second.get_str();
                  if(boost::iequals("galactic",cframe))
                    {
                      coordinate_frame=Coordinate_Frame::Galactic;
                    }
                  else if(boost::iequals("j2000",cframe))
                    {
                      coordinate_frame=Coordinate_Frame::J2000;
                    }
                  else if(boost::iequals("icrs",cframe))
                    {
                      coordinate_frame=Coordinate_Frame::ICRS;
                    }
                  else
                    {
                      throw std::runtime_error("Unknown value for pos.frame: "
                                               + cframe
                                               + "\nExpected galactic, j2000, "
                                               "or icrs");
                    }
                }
              else if(element.first=="angResolution")
                {
                  if(element.second.type()!=json5_parser::real_type)
                    throw std::runtime_error("Expected a number for "
                                             "'pos.angResolution'");

                  angResolution=element.second.get_real();
                  if(angResolution<=0)
                    throw std::runtime_error("pos.angResolution must be "
                                             "positive.  Instead got: "
                                             + std::to_string(angResolution));
                }
              else
                {
                  throw std::runtime_error("Unknown field in 'pos' structure: "
                                           + element.first);
                }
            }
        }
      else if(v.first=="keywords")
        {
          if(v.second.type()!=json5_parser::obj_type)
            throw std::runtime_error("Expected an object for 'keywords'");

          auto &k=v.second.get_obj();
          for(auto &element: k)
            {
              std::string key=element.first;
              if(element.second.type()==json5_parser::str_type)
                {
                  keywords.push_back({key,{element.second.get_str(),
                          std::string("")}});
                }
              else if(element.second.type()==json5_parser::array_type)
                {
                  auto &a=element.second.get_array();
                  if(a.size()!=2)
                    throw std::runtime_error("The array for keywords." + key
                                             + " must be exactly 2 elements "
                                             "long, but found "
                                             + std::to_string(a.size()));
                  if(a[0].type()!=json5_parser::str_type
                     || a[1].type()!=json5_parser::str_type)
                    throw std::runtime_error("Expected a string or an array "
                                             "of strings for 'keywords."
                                             + key + "'");
                    
                  keywords.push_back(std::make_pair
                                     (key,std::make_pair(a[0].get_str(),
                                                         a[1].get_str())));
                }
              else
                {
                  throw std::runtime_error("Expected a string or an array of "
                                           "strings for 'keywords."
                                           + key + "'");
                }
            }
        }

      else
        {
          throw std::runtime_error("Unknown parameter in input file: "
                                   + v.first);
        }
    }
  switch(coordinate_frame)
    {
    case Coordinate_Frame::ICRS:
    case Coordinate_Frame::J2000:
      keywords.push_back({"CTYPE1",{"RA---TAN",""}});
      keywords.push_back({"CTYPE2",{"DEC--TAN",""}});
      if(columns.find("ra")==columns.end())
        columns.insert({"ra","ra"});
      if(columns.find("dec")==columns.end())
        columns.insert({"dec","dec"});
      break;
    case Coordinate_Frame::Galactic:
      keywords.push_back({"CTYPE1",{"GLON-TAN",""}});
      keywords.push_back({"CTYPE2",{"GLAT-TAN",""}});
      if(columns.find("ra")==columns.end())
        columns.insert({"ra","glon"});
      if(columns.find("dec")==columns.end())
        columns.insert({"dec","glat"});
      break;
    }

  if(output_prefix.empty())
    throw std::runtime_error("output_prefix required");
  if(input_file.empty())
    throw std::runtime_error("input_file required");
  if(running_hires && iterations.empty())
    throw std::runtime_error("iterations required when running hires");
  if(angResolution==0)
    throw std::runtime_error("pos.angResolution required");
  
  if(running_hires && !boost::filesystem::exists(drf_file))
    throw std::runtime_error("drf_file :" + drf_file.string ()
                             + "does not exist");
}
