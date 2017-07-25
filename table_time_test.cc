#include <array>
#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <Table.h>
#include <cmath>

using FType = double;
//using FType = float;

constexpr const char kPx[] = "p_x";
using Px = Column<kPx, FType>;

constexpr const char kPy[] = "p_z";
using Py = Column<kPy, FType>;

constexpr const char kPz[] = "p_z";
using Pz = Column<kPz, FType>;

constexpr const char kEnergy[] = "energy";
using Energy = Column<kEnergy,FType>;

constexpr const char kTag[] = "tag";
using Tag = Column<kTag,int>;

using ParticleTable = Table<Px, Py, Pz, Energy, Tag>;

std::vector<FType> pTs( TableView<Px,Py> tv) {
  std::vector<FType> results;
  results.reserve(tv.size());

  for(auto const& r: tv) {
    std::cout <<"loop"<<std::endl;
    auto px = r.get<Px>();
    auto py = r.get<Py>();
    results.push_back(std::sqrt(px*px+py*py));
  }
  
  return results;
}  

struct Particle {
  Particle(FType x, FType y, FType z, FType e, int tag):
    px_(x),py_(y),pz_(z),energy_(e), tag_(tag) {}
  FType px_;
  FType py_;
  FType pz_;
  FType energy_;
  int tag_;
};


template <typename F>
void timeThis( const char * iTitle, F iF) {
  using clock = std::chrono::high_resolution_clock;

  constexpr const int kTimes = 10;
  FType ave = 0.;
  FType aveV = 0.;
  for(int i=0; i<kTimes; ++i) {
    auto start = clock::now();
    FType v= iF();
    auto stop = clock::now();
    aveV += v;
    std::chrono::duration<double,std::ratio<1,1000>> diff = stop-start;
    ave += diff.count();

    //std::cout << iTitle<<" " <<v<<" time:"<<diff.count()<<std::endl;
  }
  std::cout << iTitle<<" " <<" ave time:"<<ave/kTimes<<" value:"<<aveV/kTimes <<std::endl;

}

int main()
{

  const size_t kSize = 10000000;
  std::vector<Particle> particles;

  
  std::vector<FType> px;
  std::vector<FType> py;
  std::vector<FType> pz;
  std::vector<FType> energy;
  std::vector<int> tag;
  particles.reserve(kSize);
  px.reserve(kSize);
  py.reserve(kSize);
  pz.reserve(kSize);
  energy.reserve(kSize);
  tag.reserve(kSize);

  for(unsigned int i=0; i<kSize; ++i) {
    px.push_back(i*0.001);
    py.push_back(i*0.001);
    pz.push_back(i*0.001);
    energy.push_back(3.*i*0.001);
    tag.push_back(i);
    particles.emplace_back(px.back(),py.back(),pz.back(),energy.back(),tag.back());
  }

  ParticleTable particleTable{px,py,pz,energy,tag};
  {
    std::vector<FType> empty;
    px = std::move(empty);
    py = std::move(empty);
    pz = std::move(empty);
    energy = std::move(empty);
    tag = std::move(std::vector<int>());
  }
 


  std::cout <<"sum px"<<std::endl;
  timeThis("vector", [&particles](){
      FType v=0.;
      for( auto const& p: particles) {
	v += p.px_;
      }
      return v;
    });

  timeThis("full table",[&particleTable]() {
      FType v=0.;
      for(auto const& p: particleTable) {
	v += p.get<Px>();
      }
      return v;
    });

  timeThis("column",[&particleTable]() {
      FType v=0.;
      for(auto x: particleTable.column<Px>()) {
	v += x;
      }
      return v;
    });

  TableView<Px> pxTable(particleTable);
  timeThis("view", [&pxTable]() {
      FType v=0.;
      for(auto const& p: pxTable) {
	v += p.get<Px>();
      }
      return v;
    });



  {
    std::cout <<"sum pt2"<<std::endl;
    timeThis("vector", [&particles](){
	FType v=0.;
	for( auto const& p: particles) {
	  v += p.px_*p.px_ + p.py_*p.py_;
	}
	return v;
      });
    
    timeThis("full table",[&particleTable]() {
	FType v=0.;
	for(auto const& p: particleTable) {
	  v += p.get<Px>()*p.get<Px>()+p.get<Py>()*p.get<Py>();
	}
	return v;
      });
    
    TableView<Px,Py> pxTable(particleTable);
    timeThis("view", [&pxTable]() {
	FType v=0.;
	for(auto const& p: pxTable) {
	  v += p.get<Px>()*p.get<Px>()+p.get<Py>()*p.get<Py>();
	}
	return v;
      });
  }

  return 0;
}

