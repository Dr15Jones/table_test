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
#include "TInterpreter.h"

constexpr const char kEta[] = "eta";
using Eta = Column<kEta,double>;

constexpr const char kPhi[] = "phi";
using Phi = Column<kPhi, double>;

using JetTable = Table<Eta,Phi>;


class TableBranchWriter {
  TBranch* m_branch;

  const char* typeName(std::type_index) const;

  std::string columnAsMemberData( std::pair<char const*, std::type_index> const& iInfo) const;
public:

  TableBranchWriter(std::string const& iTableName, TTree& iTree, TableReaderBase& iReader, void* iAddress);

  void setAddress(void* iAddress) {
    m_branch->SetAddress(iAddress);
  }
};


TableBranchWriter::TableBranchWriter(std::string const& iName, TTree& iTree, TableReaderBase& iReader, void* iAddress)
{
  //create a description of a struct with the same memory layout
  std::string strct = "namespace bw { struct "+iName+" { \n";

  //NOTE: size must be an `int` and must be before the arrays
  // else readback will fail
  strct += "int size;\n";

  auto columns = iReader.columnDescriptions();  
  for(auto const& c: columns ) {
    strct += columnAsMemberData(c);
    strct += "\n";
  }
  strct +=" }; }";

  std::cout <<strct<<std::endl;

  //gROOT->ProcessLine(strct.c_str());

  if (!gInterpreter->Declare(strct.c_str())) {
    std::cout <<"Could not declare"<<std::endl;
    return;
  }
  auto tupleCl = TClass::GetClass( std::string("bw::"+iName).c_str() );
  if (!tupleCl) {
    std::cout <<"Could not find class"<<std::endl;
    return;
  }


  m_branch = iTree.Branch((iName+".").c_str(), (std::string("bw::")+iName).c_str(),iAddress);
  //iTree.Branch( (iName+"_size").c_str(), &m_size);


}

std::string
TableBranchWriter::columnAsMemberData( std::pair<char const*, std::type_index> const& iInfo) const {
  std::string returnValue = typeName(iInfo.second) ;
  returnValue +="* ";
  returnValue +=iInfo.first;
  returnValue +="; //[size]";
  return returnValue;
}

const char* 
TableBranchWriter::typeName(std::type_index iType) const {
  static const std::unordered_map<std::type_index,const char*> s_map = {
    {typeid(char),"char"},
    {typeid(unsigned char),"unsigned char"},
    {typeid(short),"short"},
    {typeid(unsigned short),"unsigned short"},
    {typeid(int),"int"},
    {typeid(unsigned int), "unsigned int"},
    {typeid(float),"float"},
    {typeid(double),"double"},
    {typeid(long), "long"},
    {typeid(unsigned long),"unsigned long"},
    {typeid(bool),"bool"}
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
  
  TFile f("test_struct_branch.root", "new");

  TTree* tree = new TTree("Events","Events");

  TableReader<JetTable> reader(&jets);
  auto pJets = &jets;

  TableBranchWriter writer("Jets",*tree, reader, &pJets);

  tree->Fill();

  {
    std::array<double,2> eta = {2.,2.};
    std::array<double,2> phi = {3.14,0.};

    JetTable jets{eta,phi};
    auto pJets = &jets;

    writer.setAddress(&pJets);
    tree->Fill();
  }
  {
    std::array<double,4> eta = {2.,3.,4.,5.};
    std::array<double,4> phi = {1.14,2.,3.3, 0.2};
    JetTable jets{eta,phi};
    auto pJets = &jets;

    writer.setAddress(&pJets);
    tree->Fill();
  }

  f.Write();
  
  return 0;
}

/*
build doing

g++ -std=c++14 -I $ROOTSYS/include branch_test.cc -L $ROOTSYS/lib -l RIO -l Tree -l Core
*/
