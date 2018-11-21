#ifndef LIBROW_HPP
#define LIBROW_HPP

#include <vector>

#include "libckt.hpp"

class row {
private:
  std::vector<node*> row_vector;
  int dWidthLimit;
  int dWidthSum = 0;
public:
  row(int dlimit) {
    dWidthLimit = dlimit;
  }
  ~row() {
    row_vector.clear();
  }
  bool push_back(node *new_node);
  node* operator[](std::size_t idx) {
    return row_vector[idx];
  }
  int getSum() const {
    return dWidthSum;
  }
  void setElement(std::size_t idx, node *new_node);
  bool checkElement(std::size_t idx, node *new_node);
  std::size_t size() {
    return row_vector.size();
  }
  void setCoordinate(int Y);
  double calHPWL();
  void updateCoordinate();
  node *random_pop();
  bool random_insert(node *new_node);
};


#endif
