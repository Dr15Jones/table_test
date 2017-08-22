#include <array>
#include <memory>
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TBranchElement.h"
#include "TStreamerInfo.h"
#include "TStreamerElement.h"
#include "Table.h"
#include "JetTable.h"
#include "TInterpreter.h"
#include <iostream>

bool buildShadowClassFor(TBranch* iBranch) {
  /* Suggestion from Philippe:
     cast the top TBranch to TBranchElement and then get the
     StreamerInfo and use that to get the member data info

     To track which Table type went with which branch, add a
     comment to the size member data which is the type name.
     The comment can be obtained again via the GetTitle for the
     appropriate TStreamInfo branch entry for 'size'
   */

  auto be = dynamic_cast<TBranchElement*>(iBranch);
  assert(be != nullptr);
  auto si = be->GetInfo();

  std::string className = si->GetName();
  size_t pos;
  int nNamespaces = 0;

  std::string strct;
  while(std::string::npos != (pos = className.find_first_of(':')) ) {
    ++nNamespaces;
    strct +="namespace ";
    strct+=className.substr(0,pos)+" { ";
    className = className.substr(pos+2);
  }
  strct +=" struct ";
  strct += className +" {\n";

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
  strct +="};";
  while(nNamespaces > 0) {
    --nNamespaces;
    strct +="}";
  }
  std::cout <<strct<<std::endl;

  return gInterpreter->Declare(strct.c_str());
}

std::string
storedTableClassName(TBranch* iBranch) {
  auto be = dynamic_cast<TBranchElement*>(iBranch);
  assert(be != nullptr);
  auto si = be->GetInfo();

  const std::string kSize("size");

  for(int i = 0; i< si->GetNelement();++i) {
    auto e = si->GetElement(i);
    if (kSize == e->GetName()) {
      return e->GetTitle();
    }
  }

  return std::string{};
}

int main()
{

  //gDebug = 3;
  TFile f("test_struct_branch.root");


  TTree* tree = dynamic_cast<TTree*>(f.Get("Events"));
  assert(tree != nullptr);

  const std::string branchName("Jets.");  
  auto jetBranch = tree->GetBranch(branchName.c_str());
  assert(nullptr != jetBranch);
  buildShadowClassFor(jetBranch);

  auto cls = TClass::GetClass(storedTableClassName(jetBranch).c_str());
  assert(nullptr != jetBranch);

  void* pJets = cls->New();
  assert(nullptr != pJet);

  jetBranch->SetAddress(&pJets);
  
  JetTable const& jets = *(reinterpret_cast<JetTable*>(pJets));

  auto nEntries = tree->GetEntries();
  for(decltype(nEntries) i = 0; i< nEntries; ++i) {
    
    jetBranch->GetEntry(i);
    std::cout <<"size "<<jets.size()<<std::endl;
    for( auto const& r: jets) {
      std::cout <<"eta "<<r.get<Eta>()<<" phi "<<r.get<Phi>()<<std::endl;
    }
  }
  
  cls->Destructor(pJets);
  return 0;
}
