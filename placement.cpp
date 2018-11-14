#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>

#include "libckt.hpp"

int main(int argc, char *argv[])
{
  std::vector<node*> inputs, outputs, nodes;
  std::map<std::string, node*> circuit; 
  std::string ckt_result = "ckt_details.txt";

  std::vector<std::string> args(argv, argv+argc);
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
      if (nodes.size() != circuit.size()) {
	std::cout << "failed to parse circuit." << std::endl;
	exit(1);
      }
      std::ofstream ckt_result_file(ckt_result);
      if(!ckt_result_file.is_open()) {
	std::cout << "failed to open " << ckt_result << std::endl;
	exit(1);
      }
      std::cout << "Writing to " << ckt_result << std::endl;
      printCktStatistics(nodes, ckt_result_file);
      ckt_result_file.close();
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
}
