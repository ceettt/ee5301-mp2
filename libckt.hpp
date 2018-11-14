#ifndef LIBCKT_HPP
#define LIBCKT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>

enum GateType {NAND, NOR, AND, OR, XOR, XNOR, INV, BUF, INP, OUTP, UNDEF,
	       TypeMAX = UNDEF};

// library function declaration
int assignDoubleWidth(GateType Type, int size);
std::string getTypeString(GateType Type);
GateType parseType(const std::string& name);
void printUsage();
int parseLine(const std::string& line, std::vector<std::string>& elements,
	      const std::string& delimiters);
void printParsedLine(const std::vector<std::string>& elements);


class node {
private:
  // type indicates cell type (nand nor etc)
  GateType type = UNDEF;
  // outname denotes the output wire name
  std::string outname;
  // fanin of the node
  std::vector<node*> inputs;
  // fanout of the node
  std::vector<node*> outputs;
  // height always 1, store width with doublewidth to reduce flop
  int doublewidth;
  
public:
  // gate count
  static int count[TypeMAX + 1];
  // total area
  static int doublearea;
  // constructor
  node(const std::string& name, const std::string& gatetype): outname(name) {
    type = parseType(gatetype);
    ++count[type];
  }
  void setType(const std::string& gatetype) {
    type = parseType(gatetype);
    ++count[type];
  }
  GateType getType() const {
    return type;
  }
  std::string getName() const {
    return outname;
  }
  int getDoubleWidth() const {
    return doublewidth;
  }
  double getWidth() const {
    return (double)doublewidth / 2.0;
  }
  void setWidth() {
    int size = inputs.size();
    doublewidth = assignDoubleWidth(type, size);
    doublearea += doublewidth;
  }
  void pushFanin(node *newnode) {
    inputs.push_back(newnode);
  }
  void pushFanout(node *newnode) {
    outputs.push_back(newnode);
  }
  static int getTypeCount(GateType type) {
    return count[type];
  }
  std::string printAllFanin() const;
  std::string printAllFanout() const;
};


int parseCkt(std::ifstream& file, std::vector<node*>& inputs,
	     std::vector<node*>& outputs,
	     std::vector<node*>& nodes_vector,
	     std::map<std::string, node*>& nodes);
void printCktStatistics(const std::vector<node*>& nodes,
			std::ofstream& outFile);

#endif
