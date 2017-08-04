#include <array>
#include <memory>
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TBranchElement.h"
#include "TStreamerInfo.h"
#include "TStreamerElement.h"
#include "TLeaf.h"
#include "Table.h"
#include "JetTable.h"
#include "TInterpreter.h"
#include <iostream>


int main()
{

  //gDebug = 3;
  TFile f("test_struct_branch.root");


  TTree* tree = dynamic_cast<TTree*>(f.Get("Events"));
  assert(tree != nullptr);

  JetTable jets;
  JetTable* pJets = &jets;

  //std::cout <<"size "<<jets.size()<<std::endl;

  std::string strct = "namespace bw { struct ";

  const std::string branchName("Jets.");  
  auto jetBranch = tree->GetBranch(branchName.c_str());
  {
    auto l = static_cast<TLeaf*>(jetBranch->GetListOfLeaves()->At(0));
    strct += l->GetTypeName()+4;
    strct += " {\n";
  }

  /* Suggestion from Philippe:
     cast the top TBranch to TBranchElement and then get the
     StreamerInfo and use that to get the member data info

     To track which Table type went with which branch, add a
     comment to the size member data which is the type name.
     The comment can be obtained again via the GetTitle for the
     appropriate TStreamInfo branch entry for 'size'
   */
  {
    auto je = dynamic_cast<TBranchElement*>(jetBranch);
    assert(je != nullptr);
    auto si = je->GetInfo();
    for(int i = 0; i< si->GetNelement();++i) {
      auto e = si->GetElement(i);
      strct +=" ";
      strct += e->GetTypeName();
      strct +=" ";
      strct += e->GetName();
      strct += ";//";
      strct +=e->GetTitle();
      strct +="\n";
    }
  }

  strct += "}; }";
  std::cout <<strct<<std::endl;
  gInterpreter->Declare(strct.c_str());

  assert(0 != jetBranch);
  jetBranch->SetAddress(&pJets);
  
  std::cout <<"size "<<jets.size()<<std::endl;

  auto nEntries = tree->GetEntries();
  for(decltype(nEntries) i = 0; i< nEntries; ++i) {
    
    jetBranch->GetEntry(i);
    std::cout <<"size "<<jets.size()<<std::endl;
    for( auto const& r: jets) {
      std::cout <<"eta "<<r.get<Eta>()<<" phi "<<r.get<Phi>()<<std::endl;
    }
  }
  
  return 0;
}

/*
build doing

g++ -std=c++14 -I $ROOTSYS/include branch_test.cc -L $ROOTSYS/lib -l RIO -l Tree -l Core
*/
