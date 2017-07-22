#include <array>
#include <memory>
#include <iostream>
#include <vector>
#include <Table.h>
#include <cmath>

constexpr const char kEta[] = "eta";
using Eta = Column<kEta,double>;

constexpr const char kPhi[] = "phi";
using Phi = Column<kPhi, double>;

using JetTable = Table<Eta,Phi>;

constexpr const char kPx[] = "p_x";
using Px = Column<kPx, double>;

constexpr const char kPy[] = "p_z";
using Py = Column<kPy, double>;

constexpr const char kPz[] = "p_z";
using Pz = Column<kPz, double>;

constexpr const char kEnergy[] = "energy";
using Energy = Column<kEnergy,double>;

using ParticleTable = Table<Px, Py, Pz, Energy>;

std::vector<double> pTs( TableView<Px,Py> tv) {
  std::vector<double> results;
  results.reserve(tv.size());
  
  auto py_s = tv.column<Py>();
  auto itPy = py_s.begin();
  for(auto px : tv.column<Px>()) {
    auto py = *itPy;
    results.push_back( std::sqrt(px*px + py*py) );
    ++itPy;
  }
  return results;
}  

void printEta( TableView<Eta> iEtas) {
  for(auto v: iEtas.column<Eta>() ) {
    std::cout <<v <<" ";
  }
  std::cout <<std::endl;
}

int main()
{

  std::array<double,3> eta = {1.,2.,4.};
  std::array<double,3> phi = {3.14,0.,1.3};
  int size = eta.size();

  JetTable jets{eta,phi};
  
  printEta(jets);

  for(auto v: jets.column<Phi>() ) {
    std::cout <<v <<" ";
  }
  std::cout <<std::endl;  

  std::vector<double> px = { 0.1, 0.9, 1.3 };
  std::vector<double> py = { 0.8, 1.7, 2.1 };
  std::vector<double> pz = { 0.4, 1.0, 0.7 };
  std::vector<double> energy = { 1.4, 3.7, 4.1};

  ParticleTable particles{px,py,pz,energy};

  for( auto v : pTs( particles ) ) {
    std::cout <<v <<" ";
  }
  std::cout <<std::endl;

  return 0;
}

