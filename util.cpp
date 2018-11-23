#include <vector>
#include <random>
#include <limits>
#include <cmath>
#include <thread>
#include <exception>
#include <fstream>

#include "libckt.hpp"
#include "librow.hpp"
#include "util.hpp"

#define MAX_TEMP 4e4
#define FRZ_TEMP 0.1
#define INIT_RATE 0.995
#define COOL_RATE 0.95

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0,1);

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

void setCoordinate(std::vector<row*>& rows)
{
  for (std::size_t i = 0; i < rows.size(); ++i) 
    rows[i]->setCoordinate(i+1); // set coordinate for each row
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

// swap two elements and set coordinate
void swap(std::vector<row*>& rows,
	  int row_idx1, int itm_idx1,
	  int row_idx2, int itm_idx2)
{
  node *a = (*rows[row_idx1])[itm_idx1];
  node *b = (*rows[row_idx2])[itm_idx2];

  rows[row_idx1]->setElement(itm_idx1, b);
  rows[row_idx2]->setElement(itm_idx2, a);

  rows[row_idx1]->setCoordinate(row_idx1+1);
  rows[row_idx2]->setCoordinate(row_idx2+1);
}

void annealing(std::vector<row*>& rows,
	       const double k,
	       const double initHPWL,
	       const int num_moves,
	       std::ofstream& outFile)
{
  double currentHPWL = initHPWL;
  double T = MAX_TEMP;
  while (T > FRZ_TEMP) {
    int accepted_moves = 0, rejected_moves = 0;
    for (auto i = 0; i < num_moves; ++i) {
      // generate a pair of node, swap, if not accepted swap back
      std::size_t size = 0;
      double newHPWL, dCost;
      int row_idx1, row_idx2;
      while (!size) { // in case generated an empty row
	row_idx1 = gen() % rows.size();
	size = rows[row_idx1]->size();
      }
      size = 0;
      while (!size) { // in case generated an empty row
	row_idx2 = gen() % rows.size();
	size = rows[row_idx2]->size();
      }
      int itm_idx1 = gen() % rows[row_idx1]->size();
      int itm_idx2 = gen() % rows[row_idx2]->size();
      swap(rows, row_idx1, itm_idx1, row_idx2, itm_idx2);
      newHPWL = layoutHPWL(rows);
      dCost = newHPWL - currentHPWL;
      if (accept_move(dCost, k, T)) {
	currentHPWL = newHPWL;
	++accepted_moves;
      } else { // if not accepted, change the items back
	swap(rows, row_idx1, itm_idx1, row_idx2, itm_idx2);
	++rejected_moves;
      }
    }
    T *= COOL_RATE; // cool down
    std::cout << "Current Temperature:" << T << std::endl;
    outFile << T << "," << accepted_moves << ","
	    << rejected_moves << ","
	    << currentHPWL << std::endl;
  }
}

double layoutHPWL(std::vector<row*>& rows)
{
  double sum = 0.0;
  for (auto i: rows)
    sum += i->calHPWL();
  return sum;
}

double layoutHPWL_multithread(std::vector<row*>& rows)
{
  // use threads to compute layout HPWL
  std::size_t height = rows.size();
  double sum = 0.0;
  const unsigned int n = std::thread::hardware_concurrency();
  double *results = new double[n];
  std::size_t *split = new std::size_t[n+1];
  std::thread *threads = new std::thread[height];
  for (unsigned i = 0; i < n; ++i)
    split[i] = i * height / n;
  split[n] = height;
  // each thread calculate the hpwl for one row
  for (unsigned i = 1; i < n; ++i)
    threads[i] = std::thread([&rows, results, split, i]() {
			       double inner_sum = 0.0;
			       for (auto j = split[i]; j < split[i+1]; ++j)
				 inner_sum += rows[j]->calHPWL();
			       results[i] = inner_sum;
			     });
  for (auto j = split[0]; j < split[1]; ++j)
    sum += rows[j]->calHPWL();
  for (unsigned i = 1; i < n; ++i) {
    threads[i].join();
    sum += results[i];
  }
  return sum;
}

// choose k based on 50 increasing cost
double kboltz(std::vector<row*>& rows,
	      double initHPWL)
{
  double currentHPWL = 0;
  double avgdCost = 0;
  int i = 0;
  const int attempts = 50;
  while (i < 50) {
    // generate a pair of node, swap then swap back
    std::size_t size = 0;
    int row_idx1, row_idx2;
    while (!size) { // in case generated an empty row, likely to happen on c17
      row_idx1 = gen() % rows.size();
      size = rows[row_idx1]->size();
    }
    size = 0;
    while (!size) { // in case generated an empty row
      row_idx2 = gen() % rows.size();
      size = rows[row_idx2]->size();
    }
    int itm_idx1 = gen() % rows[row_idx1]->size();
    int itm_idx2 = gen() % rows[row_idx2]->size();
    swap(rows, row_idx1, itm_idx1, row_idx2, itm_idx2);
    currentHPWL = layoutHPWL(rows);
    double dCost = currentHPWL - initHPWL;
    if (dCost > 0) {
      avgdCost += dCost;
      ++i;
    }
    swap(rows, row_idx1, itm_idx1, row_idx2, itm_idx2);
  }
  avgdCost /= attempts;
  return 0 - avgdCost / (std::log(INIT_RATE)*MAX_TEMP);
}

void annealingStatistics(std::ofstream& outFile,
			 std::vector<row*>& rows,
			 std::vector<node*>& nodes,
			 double initHPWL)
{
  double finalHPWL = layoutHPWL(rows);
  int Height = rows.size();
  int dWidth = 0;
  for (auto i: rows) 
    dWidth = std::max(dWidth, i->getSum());
  outFile << "Initial HPWL:\t" << initHPWL << std::endl
	    << "Final HPWL:\t" << finalHPWL << std::endl
	    << "Final Width:\t" << dWidth / 2.0 << std::endl
	    << "Final Height:\t" << Height << std::endl
	    << "Total Area:\t" << Height * dWidth / 2.0
	    << std::endl << std::endl
	    << "Coordinates of bottem-left corner of each cell" << std::endl;
  for (auto i:nodes)
    outFile << i->getName() << "\t\t"
	      << "X:" << i->getDoubleX() / 2.0 << "\t"
	      << "Y:" << i->getY() << std::endl;
}
