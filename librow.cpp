#include <vector>
#include <random>
#include <iterator>

#include "libckt.hpp"
#include "librow.hpp"
#include "util.hpp"

extern std::mt19937 gen;

bool row::push_back(node *new_node) {
  int new_dWidth = new_node->getDoubleWidth();
  dWidthSum += new_dWidth;
  if (dWidthSum > dWidthLimit) {
    dWidthSum -= new_dWidth;
    return false;
  } else {
    row_vector.push_back(new_node);
    return true;
  }
}

// random insert a node
bool row::random_insert(node *new_node) {
  int new_dWidth = new_node->getDoubleWidth();
  dWidthSum += new_dWidth;
  if (dWidthSum > dWidthLimit) {
    dWidthSum -= new_dWidth;
    return false;
  } else {
    size_t idx = gen() % int(row_vector.size() + 1);
    auto pos = row_vector.begin();
    if (idx == row_vector.size())
      pos = row_vector.end();
    else
      std::advance(pos, idx);
    row_vector.insert(pos, new_node);
    return true; 
  }
}  

// check if new element can replace current element
bool row::checkElement(std::size_t idx, node *new_node) {
  int new_dWidth = new_node->getDoubleWidth();
  int current_dWidth = row_vector[idx]->getDoubleWidth();
  int newSum = dWidthSum + new_dWidth - current_dWidth;
  return (newSum > dWidthLimit) ? false : true;
}

void row::setElement(std::size_t idx, node *new_node) {
  int new_dWidth = new_node->getDoubleWidth();
  int current_dWidth = row_vector[idx]->getDoubleWidth();
  dWidthSum = dWidthSum + new_dWidth - current_dWidth;
  row_vector[idx] = new_node;
}

// set coordinate for current row
void row::setCoordinate(int Y) {
  int current_dWidth = 0;
  for (auto i: row_vector) {
    i->setDoubleX(current_dWidth);
    i->setY(Y);
    current_dWidth += i->getDoubleWidth();
  }
  // after all set, set the total width of the row
  dWidthSum = current_dWidth;
}

// calculate HPWL of current row
double row::calHPWL() {
  double result = 0.0;
  for (auto i: row_vector) 
    result += i->netHPWLCal();
  return result;
}

// update coordinate
void row::updateCoordinate() {
  int current_dWidth = 0;
  for (auto i: row_vector) {
    if (current_dWidth != i->getDoubleWidth()) {
      i->setDoubleX(current_dWidth);
    }
    current_dWidth += i->getDoubleWidth();
  }
}

// random pop an element
node *row::random_pop() {
  if (row_vector.empty())
    return nullptr;
  int idx = gen() % int(row_vector.size());
  return remove_at(row_vector, idx);
}

