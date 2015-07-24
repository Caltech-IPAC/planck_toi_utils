#include <thread>
#include <fstream>

#include "tinyhtm/Query.hxx"

#include "Results.hxx"
#include "Query.hxx"

void thread_callback (const std::string &data_file,
                      const std::string &query_shape,
                      const std::string &vertex_string, uint64_t *num_results,
                      Query *query)
{
  tinyhtm::Query htm_query (data_file, query_shape, vertex_string);
  *num_results = htm_query.search (std::bind (&Query::callback, query,
                                              std::placeholders::_1));
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
      std::cerr << "Example: $ planck_toi_search circle \"10 10 1\" "
                   "\"\" out.fits input1.fits input2.hdf5\n";
      exit (1);
    }
  try
  {
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
    std::vector<Query> queries;
    for (int i = 0; i < num_input_files; ++i)
      {
        queries.emplace_back (time_intervals, argv[i + min_args]);
      }

    /* Run the query */
    std::vector<std::thread> threads;
    std::vector<uint64_t> num_result (num_input_files);
    for (int i = 0; i < num_input_files; ++i)
      {
        threads.emplace_back (thread_callback, argv[i + min_args], argv[1],
                              argv[2], &num_result[i], &queries[i]);
      }
    for (auto &t : threads)
      t.join ();

    /* Print the results */
    Results results;
    for (auto &q : queries)
      {
        results.data.insert (results.data.end (), q.results.data.begin (),
                             q.results.data.end ());
      }

    std::string outpath (argv[4]);
    if (outpath == "-")
      {
        std::cout << results;
      }
    else
      {
        results.write_fits (argv[4]);
      }
  }
  catch (std::runtime_error &e)
  {
    std::cerr << e.what () << "\n";
    exit (1);
  }
}
