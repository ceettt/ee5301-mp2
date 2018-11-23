#ifndef UTIL_H
#define UTIL_H

// at location n, exchange with last element and pop it
template <typename T>
T remove_at(std::vector<T>& v,typename std::vector<T>::size_type n)
{
  T ans;
  if (n != v.size()) {
    ans = std::move_if_noexcept(v[n]);
    v[n] = std::move_if_noexcept(v.back());
  } else
    ans = std::move_if_noexcept(v.back());
  v.pop_back();
  return ans;
}


bool random_placement(const std::vector<node*>& nodes,
		      std::vector<row*>& rows,
		      int dlWidth, int lHeight);

void destroy(std::vector<row*>& rows);

double layoutHPWL(std::vector<row*>& rows);
double kboltz(std::vector<row*>& rows, double initHPWL);

void setCoordinate(std::vector<row*>& rows);

void annealing(std::vector<row*>& rows,
	       const double k,
	       const double initHPWL,
	       const int num_moves,
	       std::ofstream& outFile);

void annealingStatistics(std::ofstream& outFile,
			 std::vector<row*>& rows,
			 std::vector<node*>& nodes,
			 double initHPWL);
#endif
