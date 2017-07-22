#include <array>
#include <memory>
#include "TArrayD.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "Table.h"
#include <iostream>

constexpr const char kEta[] = "eta";
using Eta = Column<kEta,double>;

constexpr const char kPhi[] = "phi";
using Phi = Column<kPhi, double>;

using JetTable = Table<Eta,Phi>;

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
  

  TFile f("test_branch.root", "new");

  TTree* tree = new TTree("Events","Events");

  tree->Branch("Jets_size",&size);
  tree->Branch("Jets_eta",eta.data(),"Jets_eta[Jets_size]/D");
  tree->Branch("Jets_phi",phi.data(),"Jets_phi[Jets_size]/D");


  tree->Fill();
  size =1;
  eta[0] = 2.;
  tree->Fill();
  size=2;
  eta[1] = 3.;
  tree->Fill();


  f.Write();
  
  return 0;
}

/*
build doing

g++ -std=c++14 -I $ROOTSYS/include branch_test.cc -L $ROOTSYS/lib -l RIO -l Tree -l Core
*/
