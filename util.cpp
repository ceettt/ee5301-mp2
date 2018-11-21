#include <vector>
#include <random>
#include <limits>
#include <cmath>
#include <thread>

#include "libckt.hpp"
#include "librow.hpp"
#include "util.hpp"

#define MAX_TEMP 40000
#define FRZ_TEMP 0.1
#define INIT_RATE 0.99

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0,1);

int random_placement_depricated(const std::vector<node*>& nodes,
				std::vector<row*>& rows,
				int lWidth, int lHeight)
{
  std::vector<node*> *set_ptr = new std::vector<node*>(nodes);
  int attempts = 1, row_idx = 0;
  rows.clear();
  row *new_row = new row(lWidth);
  rows.push_back(new_row);
  while(set_ptr->size()) {
    // random generate a number
    int idx = gen() % set_ptr->size();
    node *current_node = remove_at(*set_ptr, idx);
    bool result = rows[row_idx]->random_insert(current_node);
    if (!result) { // current row is full
      ++row_idx;
      if (row_idx >= lHeight) {
	// rows are all full, try to insert to any available row
	set_ptr->push_back(current_node);
	while (set_ptr->size()) {
	  int idx = gen() % set_ptr->size();
	  current_node = remove_at(*set_ptr, idx);
	  bool ret = false;
	  for (auto i : rows) {
	    ret = i->random_insert(current_node);
	    if (ret) break; // success then break
	  }
	  if (!ret) { // no row can accept current node
	    set_ptr->push_back(current_node);
	    break;
	  }
	}
	if (set_ptr->empty()) // set is empty then successful placement
	  break;
	if (attempts == 500) {
	  std::cerr << "Failed to generate random placement after "
		    << "500 attempts. " << std::endl;
	  int placed = 0, nplaced = 0;
	  for (auto i : rows)
	    placed += i->getSum();
	  for (auto i : *set_ptr)
	    nplaced += i->getDoubleWidth();
	  nplaced += current_node->getDoubleWidth();
	  std::cerr << "Last Attempt" << std::endl
		    << "Width:" << lWidth << std::endl
		    << "Height:" << lHeight << std::endl
		    << "Total area:"
		    << node::doublearea/2.0 << std::endl
		    << "Placed area:"
		    << placed/2.0 << std::endl
		    << "Not Placed area:"
		    << nplaced/2.0 << std::endl
		    << "Not Placed cells:"
		    << set_ptr->size() << std::endl;
	  exit(1);
	}
	++attempts;
	delete set_ptr;
	set_ptr = new std::vector<node*>(nodes);
	rows.clear();
	row_idx = 0;
	row *new_row = new row(lWidth);
	rows.push_back(new_row);
	continue;
      }
      // add a new row, push the popped element back
      new_row = new row(lWidth);
      rows.push_back(new_row);
      set_ptr->push_back(current_node);
    }
  }
  std::cout << "Random Placement Generated in "
	    << attempts << " attempts." << std::endl;
  int placed_area = 0;
  for (auto i : rows)
    placed_area += i->getSum();
  std::cout << "Last Attempt" << std::endl
	    << "Width:" << lWidth << std::endl
	    << "Height:" << lHeight << std::endl
	    << "Total area:"
	    << node::doublearea/2.0 << std::endl
	    << "Placed area:"
	    << placed_area/2.0 << std::endl;
	  
  return 0;
}


bool random_placement(const std::vector<node*>& nodes,
		      std::vector<row*>& rows,
		      int dlWidth, int lHeight)
{
  std::vector<node*> cell_list(nodes);
  // Sort the list according to the width
  std::sort(cell_list.begin(), cell_list.end(),
	    [](node *a, node *b) {
	      return a->getDoubleWidth() < b->getDoubleWidth();
	    });
  rows.clear();
  for (auto i = 0; i < lHeight; ++i) {
    row *new_row = new row(dlWidth);
    rows.push_back(new_row);
  }
  bool attempt = true;
  // this flag fails if a node can't be inserted to any row
  while (cell_list.size()) {
    // pop a node from the back (largest) and random insert to a row
    node *current_node = cell_list.back();
    cell_list.pop_back();
    bool ret = false;
    attempt = true;
    int attempts = 0;
    while (!ret) { // ret is false, no node is inserted, try again
      if (attempts == 100) {
	bool new_ret = false;
	// if tried 100 times just find the first available row
	for (auto i : rows) {
	  new_ret = i->random_insert(current_node);
	  if (new_ret) break; // success then break
	}
	if (!new_ret) { // no row can accept current node
	  attempt = false;
	}
	break;
	// always get out of the while loop after trying to allocate node
      }
      int idx = gen() % lHeight;
      ret = rows[idx]->random_insert(current_node);
      ++attempts;
    }
    if (!attempt) {
      cell_list.push_back(current_node);
      break; // a node failed, break process
    }
  }
  if (attempt) {
    std::cout << "Random Placement Generated" << std::endl;
    int placed_area = 0;
    for (auto i : rows)
      placed_area += i->getSum();
    std::cout << "Last Attempt" << std::endl
	      << "Width:" << dlWidth/2.0 << std::endl
	      << "Height:" << lHeight << std::endl
	      << "Total area:"
	      << node::doublearea/2.0 << std::endl
	      << "Placed area:"
	      << placed_area/2.0 << std::endl;
    return true;
  } else {
    /*
    std::cerr << "Failed attempt on random placement" << std::endl;
    int placed = 0, nplaced = 0;
    for (auto i : rows)
      placed += i->getSum();
    for (auto i : cell_list)
      nplaced += i->getDoubleWidth();
    std::cerr << "Width:" << dlWidth/2.0 << std::endl
	      << "Height:" << lHeight << std::endl
	      << "Total area:"
	      << node::doublearea/2.0 << std::endl
	      << "Placed area:"
	      << placed/2.0 << std::endl
	      << "Not Placed area:"
	      << nplaced/2.0 << std::endl
	      << "Not Placed cells:"
	      << cell_list.size() << std::endl;
    */
    return false;
  }
}

// destroy a vector
void destroy(std::vector<row*>& rows)
{
  while (!rows.empty()) {
    row *back = rows.back();
    rows.pop_back();
    delete back;
  }
}

bool accept_move(double dCost,
		 double k,
		 double T)
{
  if (dCost < 0) return true;
  double boltz = std::exp(-dCost/(k*T));
  double r = dis(gen);
  return r < boltz;
}

void annealing()
{

}

// swap two elements
void swap(std::vector<row*>& rows,
	  int row_idx1, int itm_idx1,
	  int row_idx2, int itm_idx2)
{
  node *a = (*rows[row_idx1])[itm_idx1];
  node *b = (*rows[row_idx2])[itm_idx2];

  rows[row_idx1]->setElement(itm_idx1, b);
  rows[row_idx2]->setElement(itm_idx2, a);
}

double layoutHPWL(std::vector<row*>& rows)
{
  double sum = 0.0;
  for (std::size_t i = 0; i < rows.size(); ++i) 
    rows[i]->setCoordinate(i+1); // set coordinate for each row
  for (auto i: rows)
    sum += i->calHPWL();
  return sum;
}

double kboltz(std::vector<row*>& rows,
	      double initHPWL)
{
  double currentHPWL = 0;
  double dCost = 0;
  const int attempts = 50;
  for (int i = 0; i < attempts; ++i) {
    // generate a pair of node, swap then swap back
    int row_idx1 = gen() % rows.size();
    int row_idx2 = gen() % rows.size();
    int itm_idx1 = gen() % rows[row_idx1]->size();
    int itm_idx2 = gen() % rows[row_idx2]->size();
    swap(rows, row_idx1, itm_idx1, row_idx2, itm_idx2);
    currentHPWL = layoutHPWL(rows);
    dCost += currentHPWL - initHPWL;
    swap(rows, row_idx1, itm_idx1, row_idx2, itm_idx2);
  }
  dCost /= attempts;
  return 0-(dCost/(std::log(INIT_RATE)*MAX_TEMP));
}
