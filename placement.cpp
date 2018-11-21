#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <chrono>
#include <iterator>

#include "libckt.hpp"
#include "librow.hpp"
#include "util.hpp"

bool enableMultiThread = false;

int main(int argc, char *argv[])
{
  typedef std::chrono::high_resolution_clock Time;
  typedef std::chrono::microseconds us;
  typedef std::chrono::duration<float> fsec;
  
  std::vector<node*> inputs, outputs, nodes;
  std::map<std::string, node*> circuit; 
  std::string ckt_result = "ckt_details.txt";
  std::vector<row*> rows;
  
  std::vector<std::string> args(argv, argv+argc);

  auto start = Time::now();
  try {
    if (args.at(1) == "read_ckt") {
      std::string ckt_filename(args.at(2));
      std::cout << "Reading circuit file from " << ckt_filename << std::endl;
      std::ifstream ckt_file(ckt_filename);
      if (!ckt_file.is_open()) {
	std::cout << "failed to open " << ckt_filename << std::endl;
	exit(1);
      }
      parseCkt(ckt_file, inputs, outputs, nodes, circuit);
      ckt_file.close();
      std::ofstream ckt_result_file(ckt_result);
      if(!ckt_result_file.is_open()) {
	std::cout << "failed to open " << ckt_result << std::endl;
	exit(1);
      }
      std::cout << "Writing to " << ckt_result << std::endl;
      printCktStatistics(nodes, ckt_result_file);
      ckt_result_file.close();
    } else if (args.at(1) == "place") {
      std::string ckt_filename(args.at(2));
      std::cout << "Reading circuit file from " << ckt_filename << std::endl;
      std::ifstream ckt_file(ckt_filename);
      if (!ckt_file.is_open()) {
	std::cout << "failed to open " << ckt_filename << std::endl;
	exit(1);
      }
      parseCkt(ckt_file, inputs, outputs, nodes, circuit);
      ckt_file.close();
      auto begin_iter = args.begin();
      std::advance(begin_iter, 2);
      for (auto iter = begin_iter; iter != args.end(); ++iter) {
	if (*iter == "--thread") {
	  enableMultiThread = true;
	  if (enableMultiThread)
	    std::cout << "Enabling multithread calculation." << std::endl;
	  break;
	}
      }
      int lWidth = std::ceil(std::sqrt(node::doublearea/2.0));
      //int lHeight = std::ceil(double(node::doublearea)/(2.0*lWidth));
      int lHeight = lWidth;
      int dlWidth = 2*lWidth; // double to make sure it is int
      int attempts = 0;
      bool ret = false;
      while (!ret) {
	destroy(rows);
	ret = random_placement(nodes, rows, dlWidth, lHeight);
	++ attempts;
	if (attempts == 100) { // after 100 tries add 0.5 to Width
	  attempts = 0;
	  ++ dlWidth;
	}
      }
      double currentHPWL = layoutHPWL(rows);
      std::cout << "Initial HPWL:" << currentHPWL << std::endl;
      double k = kboltz(rows, currentHPWL);
      std::cout << "Initial k:" << k << std::endl;
      // test if hpwl changed
      currentHPWL = layoutHPWL(rows);
      std::cout << "Current HPWL:" << currentHPWL << std::endl;
    } else {
      std::cout << "Not enough parameters." << std::endl;
      printUsage();
      exit(1);
    }
  } catch (const std::out_of_range& e) {
    std::cout << "Not enough parameters." << std::endl;
    printUsage();
    exit(1);
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    exit(1);
  }
  auto stop = Time::now();
  fsec fs = stop - start;
  us d = std::chrono::duration_cast<us>(fs);
  std::cout << "Program took \t" << fs.count() << "s" << std::endl
	    << "\tOr \t"<< d.count() << "us" << std::endl;
  
  return 0;
}

