#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <exception>
#include <string>

#include "libckt.hpp"


// out-of-class initializations
int node::count[TypeMAX+1] = {};

int node::doublearea = 0;

bool node::fPosition() {
  if (dX == -1 || Y == -1)
    return false;
  else
    return true;
}

double node::netHPWLCal() {
  int minDoubleX = dX, minY = Y;
  int maxDoubleX = dX, maxY = Y;
  for (auto i : inputs) {
    minDoubleX = std::min(minDoubleX, i->getDoubleX());
    maxDoubleX = std::max(maxDoubleX, i->getDoubleX());
    minY = std::min(minY, i->getY());
    maxY = std::min(maxY, i->getY());
  }
  for (auto i : outputs) {
    minDoubleX = std::min(minDoubleX, i->getDoubleX());
    maxDoubleX = std::max(maxDoubleX, i->getDoubleX());
    minY = std::min(minY, i->getY());
    maxY = std::min(maxY, i->getY());
  }
  return double(maxDoubleX-minDoubleX) / 2.0
    + double(maxY-minY);
}

std::string node::printAllFanout() const
{
  std::string target;
  if (type == INP)
    target = "";
  else {
    target = getTypeString(type);
    target += "-" + getName() + ": ";
    if (outputs.empty())
      target += "OUTP\n";
    else { // iterate through the vector
      for (auto i = outputs.begin(); i < outputs.end(); ) {
	target += getTypeString((*i)->type) + "-" + (*i)->getName();
	if ((++i) != outputs.end())
	  target += ", ";
      }
      target += "\n";
    }
  }
  return target;
}

std::string node::printAllFanin() const
{
  std::string target;
  if (type == INP)
    target = "";
  else {
    target = getTypeString(type);
    target += "-" + getName() + ": ";
    for (auto i = inputs.begin(); i < inputs.end(); ) {
      target += getTypeString((*i)->type) + "-" + (*i)->getName();
      if ((++i) != inputs.end())
	target += ", ";
    }
    target += "\n";
  }
  return target;
}

int parseCkt(std::ifstream& file,
	     std::vector<node*>& inputs,
	     std::vector<node*>& outputs,
	     std::vector<node*>& nodes_vector,
	     std::map<std::string, node*>& nodes)
{
  std::string currentLine;
  node *ptrNodeCell = nullptr;
  while (std::getline(file, currentLine)) {
    if (currentLine == "" || currentLine.find_first_of("#") == 0) {
      continue; // skip empty line and comments
    }
    std::vector<std::string> elements;
    // note: it seems iscas85 uses "\r\n" instead of "\n"
    //       so it is better to get rid of it during tokenizing
    //       otherwise it'll screw up when printing 
    parseLine(currentLine, elements, "\r\t (),");
    if (elements.empty()) // skip empty line
      continue;
    try {
      ptrNodeCell = nullptr;
      if (elements.front() == "INPUT") {
	// input declaration line
	// front() indicate input, [1] is the node name
	ptrNodeCell = new node(elements.at(1), elements.front());
	nodes.insert(std::make_pair(elements.at(1),
				    ptrNodeCell));
	inputs.push_back(ptrNodeCell);
	nodes_vector.push_back(ptrNodeCell);
	ptrNodeCell->setWidth();
      } else if (elements.front() == "OUTPUT") {
	// create a new output gate and link to the inner one
	std::string port_name = elements.at(1) + "-OUTPUT";
	ptrNodeCell = new node(port_name, elements.front());
	nodes.insert(std::make_pair(port_name, ptrNodeCell));
	outputs.push_back(ptrNodeCell);
	nodes_vector.push_back(ptrNodeCell);
	ptrNodeCell->setWidth();
	node *temp = ptrNodeCell;
	// search for or create the actual cell
	auto search = nodes.find(elements.at(1));
	if (search != nodes.end()) {
	  ptrNodeCell = search->second;
	} else {
	  ptrNodeCell = new node(elements.at(1), "UNDEF");
	  nodes.insert(std::make_pair(elements.at(1),
				      ptrNodeCell));
	  nodes_vector.push_back(ptrNodeCell);
	}
	ptrNodeCell->pushFanout(temp);
	temp->pushFanin(ptrNodeCell);
      } else if (elements.at(1) == "=") {
	// value assignment line
	// front() element is the name, [2] is the node name
	// check if it is already in map, set the type and add nodes
	// otherwise create a new node and add to map
	auto search = nodes.find(elements.front());
	if (search != nodes.end()) {
	  ptrNodeCell = search->second;
	  ptrNodeCell->setType(elements.at(2));
	} else {
	  ptrNodeCell = new node(elements.front(), elements.at(2));
	  nodes.insert(std::make_pair(elements.front(),
				      ptrNodeCell));
	  nodes_vector.push_back(ptrNodeCell);
	}
	// add edges to the adjacent vector, elements starting at [3]
	for (auto Iter = std::next(elements.begin(), 3);
	     Iter < elements.end(); ++Iter) {
	  search = nodes.find(*Iter);
	  node *adjPtr = nullptr;	  
	  if (search != nodes.end()) {
	    adjPtr = nodes[*Iter];
	  } else {
	    adjPtr = new node(*Iter, "UNDEF");
	    nodes.insert(std::make_pair(*Iter, adjPtr));
	    nodes_vector.push_back(adjPtr);
	  }
	  ptrNodeCell->pushFanin(adjPtr);
	  adjPtr->pushFanout(ptrNodeCell);
	}
	ptrNodeCell->setWidth();
      } else {
	std::cout << "Line can't be parsed: ";
	printParsedLine(elements);
      }
    } catch (const std::out_of_range& e) {
      std::cout << "Line can't be parsed: ";
      printParsedLine(elements);
    }
    elements.clear();
  }	      
  return 0;
}


// read a line, seperate them according to delimiters and
// push them to the vector
// ignore empty strings
int parseLine(const std::string& line, std::vector<std::string>& elements,
	      const std::string& delimiters)
{
  std::size_t start=0, end;
  while((end = line.find_first_of(delimiters, start)) != std::string::npos) {
    std::string temp = line.substr(start, end-start);
    if (temp != "") {
      elements.push_back(temp);
    }
    start = end + 1; // move start to next charactor
  }
  if (start < line.size()) // more char left
    elements.push_back(line.substr(start, line.size()));
  return 0;
}

void printParsedLine(const std::vector<std::string>& elements)
{
  std::cout << "[";
  for (auto& Iter: elements) {
    std::cout << "\"" << Iter << "\",";
  }
  std::cout << "]" << std::endl;
}

void printUsage()
{
  std::cout << "USAGE:\t./placement read_ckt <FILENAME>\t\tRead circuit and write statistics to file" << std::endl;
  std::cout << "\t./placement place <FILENAME>\t\tRead and parse a circuit using default library" << std::endl;
}

GateType parseType(const std::string& name)
{
  GateType type;
  if(name == "NAND2_X1" || name == "NAND")
    type = NAND;
  else if (name == "NOR2_X1" || name == "NOR")
    type = NOR;
  else if (name == "AND2_X1" || name == "AND")
    type = AND;
  else if (name == "OR2_X1" || name == "OR")
    type = OR;
  else if (name == "XOR2_X1" || name == "XOR")
    type = XOR;
  else if (name == "INV_X1" || name == "NOT")
    type = INV;
  else if (name == "BUF_X1" || name == "BUFF")
    type = BUF;
  else if (name == "XNOR")
    type = XNOR;
  else if (name == "INPUT")
    type = INP;
  else if (name == "OUTPUT")
    type = OUTP;
  else
    type = UNDEF;
  return type;
}

std::string getTypeString(GateType Type)
{
  switch (Type) {
  case NAND:
    return "NAND";
  case NOR:
    return "NOR";
  case AND:
    return "AND";
  case OR:
    return "OR";
  case XOR:
    return "XOR";
  case XNOR:
    return "XNOR";
  case INV:
    return "INV";
  case BUF:
    return "BUF";
  case INP:
    return "INP";
  case OUTP:
    return "OUTP";
  default:
    return "Undefined";
  }
}

int assignDoubleWidth(GateType Type, int size)
{
  int adj_size = (size < 2) ? 2 : size;
  switch (Type) {
  case INP: /* fall-thru */
  case OUTP:/* fall-thru */
  case INV: /* fall-thru */
    return 2;
  case BUF:
    return 4;
  case NAND:
    return 2 * adj_size;
  case NOR: /* fall-thru */
  case AND:
    return 3 * adj_size;
  case XOR:
    return 5 * adj_size;
  case OR:
    return 4 * adj_size;
  case XNOR:
    return 6 * adj_size;
  default:
    return 0;
  }
}

void printCktStatistics(const std::vector<node*>& nodes,
			 std::ofstream& outFile)
{
  outFile << node::getTypeCount(INP) << " primary inputs" << std::endl;
  outFile << node::getTypeCount(OUTP) << " primary outputs" << std::endl;
  for (GateType i = NAND; i < INP; i = GateType(1 + int(i))) {
    if (node::getTypeCount(i))
      outFile << node::getTypeCount(i) << " " << getTypeString(i)
	      << " gates" << std::endl;
  }
  outFile << "Total Area: " << node::doublearea/2.0 << std::endl;
  outFile << std::endl << std::endl << "Fanout..." << std::endl;
  for (const auto& node: nodes) {
    std::string result = node->printAllFanout();
    outFile << result;
  }
  outFile << std::endl << std::endl << "Fanin..." << std::endl;
  for (const auto& node: nodes) {
    std::string result = node->printAllFanin();
    outFile << result;
  }
  outFile << std::endl << std::endl << "Size..." << std::endl;
  for (const auto& node: nodes) {
    outFile << getTypeString(node->getType())
	    << "-" << node->getName()
	    << ": " << node->getWidth()
	    << std::endl;
  }
  outFile << std::endl;
}


