#include <array>
#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <Table.h>
#include <cmath>
#include <sstream>

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

/* Create a new table that is an extension of an existing table*/
constexpr const char kLabel[] = "label";
using Label = Column<kLabel,std::string>;

using MyJetTable = AddColumns_t<JetTable, std::tuple<Label>>;

/* Creat a table that is a sub table of an existing one */
using MyOtherJetTable = RemoveColumn_t<MyJetTable, Phi>;


std::vector<double> pTs( TableView<Px,Py> tv) {
  std::vector<double> results;
  results.reserve(tv.size());

  for(auto const& r: tv) {
    std::cout <<"loop"<<std::endl;
    auto px = r.get<Px>();
    auto py = r.get<Py>();
    results.push_back(std::sqrt(px*px+py*py));
  }
  
  return results;
}  

void printEta( TableView<Eta> iEtas) {
  for(auto v: iEtas.column<Eta>() ) {
    std::cout <<v <<" ";
  }
  std::cout <<std::endl;
}

struct JetType {
  double eta_;
  double phi_;
};

double value_for_column(JetType const& iJ, Eta*) {
  return iJ.eta_;
}

double value_for_column(JetType const& iJ, Phi*) {
  return iJ.phi_;
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

  for(auto const& v: jets) {
    std::cout <<"eta "<<v.get<Eta>()<<" phi "<<v.get<Phi>()<<std::endl;
  }

  std::vector<double> px = { 0.1, 0.9, 1.3 };
  std::vector<double> py = { 0.8, 1.7, 2.1 };
  std::vector<double> pz = { 0.4, 1.0, 0.7 };
  std::vector<double> energy = { 1.4, 3.7, 4.1};

  ParticleTable particles{px,py,pz,energy};

  for( auto v : pTs( particles ) ) {
    std::cout <<v <<" ";
  }
  std::cout <<std::endl;

  {
    std::vector<JetType> j = {{1.,3.14},{2.,0.},{4.,1.3}};

    int index=0;
    MyJetTable jt{ j, make_columns_filler(filler_for<Label>([&index](JetType const&) 
							    { std::ostringstream s;
							      s<<"jet"<<index++;
							      return s.str();}) ) };

    for(auto const& v: jt) {
      std::cout <<"eta "<<v.get<Eta>()<<" phi "<<v.get<Phi>()<<" "<<v.get<Label>()<<std::endl;
    }

  }

  {
    ParticleTable copyTable{particles};

    auto compare = [](const ParticleTable& iLHS, const ParticleTable& iRHS) {
      if(iLHS.size() != iRHS.size()) {
	std::cout <<"copy size wrong "<<iLHS.size()<<" "<<iRHS.size()<<std::endl;
      }
      for(size_t i = 0; i< iRHS.size(); ++i) {
	if(iLHS.get<Px>(i) != iRHS.get<Px>(i)) {
	  std::cout <<"copy px not the same "<<i<<" "<<iLHS.get<Px>(i)<< " "<<iRHS.get<Px>(i)<<std::endl;
	}
	if(iLHS.get<Py>(i) != iRHS.get<Py>(i)) {
	  std::cout <<"copy py not the same "<<i<<" "<<iLHS.get<Py>(i)<< " "<<iRHS.get<Py>(i)<<std::endl;
	}
	if(iLHS.get<Pz>(i) != iRHS.get<Pz>(i)) {
	  std::cout <<"copy pz not the same "<<i<<" "<<iLHS.get<Pz>(i)<< " "<<iRHS.get<Pz>(i)<<std::endl;
	}
	if(iLHS.get<Energy>(i) != iRHS.get<Energy>(i)) {
	  std::cout <<"copy energy not the same "<<i<<" "<<iLHS.get<Energy>(i)<< " "<<iRHS.get<Energy>(i)<<std::endl;
	}
      }
    };
    compare(copyTable,particles);

    std::cout <<"move ctr"<<std::endl;
    ParticleTable moveTable(std::move(copyTable));
    compare(moveTable,particles);

    std::cout <<"op="<<std::endl;
    ParticleTable opEqTable;
    opEqTable = particles;
    compare(opEqTable,particles);

    std::cout <<"op=&&"<<std::endl;
    ParticleTable opEqMvTable;
    opEqMvTable=std::move(moveTable);
    compare(opEqMvTable,particles);
  }

  return 0;
}

