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
      const size_t num_bins(40);
      std::vector<Ranges> ranges;
      for(int i=3;i<argc;++i)
        {
          ranges.emplace_back(num_bins);
        }

      for(int i=3;i<argc;++i)
        {
          tinyhtm::Query query(argv[i],argv[1],argv[2]);
          query.callback(std::bind(&Ranges::callback,&ranges[i-3],
                                   std::placeholders::_1,
                                   std::placeholders::_2,
                                   std::placeholders::_3,
                                   std::placeholders::_4));
        }
      Ranges result(num_bins);
      for(auto &r: ranges)
        {
          for(size_t i=0;i<num_bins;++i)
            result.r[i]+=r.r[i];
        }
      std::cout << result;
    }
  catch(std::runtime_error &e)
    {
      std::cerr << e.what() << "\n";
      exit(1);
    }
}
