#include "Ranges.hxx"

int main(int argc, char *argv[])
{
  if(argc<4)
    {
      std::cerr << "Need at least 3 arguments.  Only got " << argc-1 << "\n";
      exit(1);
    }
  try
    {
      const int num_bins(40);
      Ranges ranges(num_bins);

      for(int i=3;i<argc;++i)
        {
          tinyhtm::Query query(argv[i],argv[1],argv[2]);
          query.callback(std::bind(&Ranges::callback,&ranges,
                                   std::placeholders::_1,
                                   std::placeholders::_2,
                                   std::placeholders::_3,
                                   std::placeholders::_4));
        }
      std::cout << ranges;
    }
  catch(std::runtime_error &e)
    {
      std::cerr << e.what() << "\n";
      exit(1);
    }
}
