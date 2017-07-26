#include <array>
#include <memory>
#include "TArrayD.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "Table.h"
#include "TableReader.h"
#include <iostream>
#include <unordered_map>

constexpr const char kEta[] = "eta";
using Eta = Column<kEta,double>;

constexpr const char kPhi[] = "phi";
using Phi = Column<kPhi, double>;

using JetTable = Table<Eta,Phi>;


class TableBranchWriter {
  int m_size;
  std::vector<TBranch*> m_branches;

  std::pair<std::string,std::string> createBranchInfo(std::string const& iTableName,
						      std::pair<char const*, std::type_index> const& iInfo) const;

  void createBranch(std::string const& iTableName,
		    TTree& iTree,
		    std::pair<char const*, std::type_index> const& iInfo) ;

  const char* typeLabel(std::type_index) const;
public:

  TableBranchWriter(std::string const& iTableName, TTree& iTree, TableReaderBase& iReader);

  void setAddresses(TableReaderBase& iReader) {
    m_size = iReader.size();
    for(unsigned int i=0; i<m_branches.size(); ++i) {
      m_branches[i]->SetAddress(iReader.columnAddress(i));
    }
  }
};


TableBranchWriter::TableBranchWriter(std::string const& iName, TTree& iTree, TableReaderBase& iReader):
  m_size(0)
{
  iTree.Branch( (iName+"_size").c_str(), &m_size);

  auto columns = iReader.columnDescriptions();
  m_branches.reserve(columns.size());
  for(auto const& c: columns ) {
    createBranch(iName, iTree,c);
  }
}

void 
TableBranchWriter::createBranch(std::string const& iTableName,
				TTree& iTree,
				std::pair<char const*, std::type_index> const& iInfo) {
  auto info = createBranchInfo(iTableName, iInfo);
  m_branches.push_back(iTree.Branch(info.first.c_str(), static_cast<void*>(nullptr), info.second.c_str()));
}


std::pair<std::string,std::string> 
TableBranchWriter::createBranchInfo(std::string const& iTableName,
				    std::pair<char const*, std::type_index> const& iInfo) const {
  std::string branchName = iTableName+"_"+iInfo.first;
  std::string branchDescription = branchName+"["+iTableName+"_size]/"+typeLabel(iInfo.second);
  return std::make_pair(branchName, branchDescription);
}

const char* 
TableBranchWriter::typeLabel(std::type_index iType) const {
  static const std::unordered_map<std::type_index,const char*> s_map = {
    {typeid(char),"B"},
    {typeid(unsigned char),"b"},
    {typeid(short),"S"},
    {typeid(unsigned short),"s"},
    {typeid(int),"I"},
    {typeid(unsigned int), "i"},
    {typeid(float),"F"},
    {typeid(double),"D"},
    {typeid(long), "L"},
    {typeid(unsigned long),"l"},
    {typeid(bool),"o"}
  };

  auto it = s_map.find(iType);
  if(it== s_map.end()) {
    return 0;
  }
  return it->second;
}


int main()
{

  std::array<double,3> eta = {1.,2.,4.};
  std::array<double,3> phi = {3.14,0.,1.3};

  JetTable jets{eta,phi};
  
  TFile f("test_branch.root", "new");

  TTree* tree = new TTree("Events","Events");

  TableReader<JetTable> reader(&jets);

  TableBranchWriter writer("Jets",*tree, reader);
  writer.setAddresses(reader);
  tree->Fill();

  {
    std::array<double,2> eta = {2.,2.};
    std::array<double,2> phi = {3.14,0.};

    JetTable jets{eta,phi};
    TableReader<JetTable> reader(&jets);
    writer.setAddresses(reader);
    tree->Fill();
  }
  {
    std::array<double,4> eta = {2.,3.,4.,5.};
    std::array<double,4> phi = {1.14,2.,3.3, 0.2};
    JetTable jets{eta,phi};
    TableReader<JetTable> reader(&jets);
    writer.setAddresses(reader);
    tree->Fill();
  }

  f.Write();
  
  return 0;
}

/*
build doing

g++ -std=c++14 -I $ROOTSYS/include branch_test.cc -L $ROOTSYS/lib -l RIO -l Tree -l Core
*/
