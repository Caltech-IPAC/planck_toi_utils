#include <thread>
#include <fstream>

#include "tinyhtm/Query.hxx"

#include "Results.hxx"

void thread_callback (const std::string &data_file,
                      const std::string &query_shape,
                      const std::string &vertex_string, uint64_t *num_results,
                      Results *results)
{
  tinyhtm::Query query (data_file, query_shape, vertex_string);

  if (results->count && results->time_intervals.empty ())
    *num_results = query.count ();
  else
    *num_results = query.search (std::bind (
        &Results::callback, results, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}

int main (int argc, char *argv[])
{
  const int min_args (5);
  if (argc - 1 < min_args)
    {
      std::cerr << "Need at least " << min_args << " arguments.  Only got "
                << argc - 1 << "\n";
      std::cerr << "Usage: $ planck_toi_search (circle | box | poly) {coords} "
                   "{time ranges} {output_file} {input_file1} {input_file2} "
                   "...\n";
      exit (1);
    }
  try
  {
    bool count (false);

    /* Parse time intervals */
    std::list<std::pair<double, double> > time_intervals;
    std::stringstream ss (argv[3]);
    double begin_time, end_time;
    ss >> begin_time;
    while (ss)
      {
        ss >> end_time;
        if (!ss)
          throw std::runtime_error ("Need an even number of integers "
                                    "in the time interval string.");
        time_intervals.push_back (std::make_pair (begin_time, end_time));
        ss >> begin_time;
      }

    const int num_input_files (argc - min_args);
    std::vector<Results> results;
    for (int i = 0; i < num_input_files; ++i)
      {
        results.emplace_back (time_intervals);
      }

    /* Run the query */
    std::vector<std::thread> threads;
    std::vector<uint64_t> num_result (num_input_files);
    for (int i = 0; i < num_input_files; ++i)
      {
        threads.emplace_back (thread_callback, argv[i + min_args], argv[1],
                              argv[2], &num_result[i], &results[i]);
      }
    for (auto &t : threads)
      t.join ();

    /* Print the results */
    if (count)
      {
        uint64_t num_results (0);
        for (auto &n : num_result)
          num_results += n;
      }
    else
      {
        Results result (time_intervals);
        for (auto &r : results)
          {
            result.data.insert (result.data.end (), r.data.begin (),
                                r.data.end ());
          }

        std::string outpath (argv[4]);
        if (outpath == "-")
          {
            std::cout << result;
          }
        else
          {
            result.write_fits (argv[4]);
          }
      }
  }
  catch (std::runtime_error &e)
  {
    std::cerr << e.what () << "\n";
    exit (1);
  }
}
