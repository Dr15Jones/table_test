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
#include "Wrapper.h"
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
  auto top_si = be->GetInfo();

  std::string kObj("obj");
  for(int i = 0; i< top_si->GetNelement(); ++i) {
    auto e = top_si->GetElement(i);
    if (e->GetName() != kObj) {
      continue;
    } else {
      std::cout <<" found obj"<<std::endl;
      const std::string fullClassName = e->GetTypeName();
      size_t pos;
      int nNamespaces = 0;

      auto shadowTClass = TClass::GetClass(fullClassName.c_str());
      assert(shadowTClass);
      
      auto si = shadowTClass->GetCurrentStreamerInfo();
      assert(si);
      
      auto className = fullClassName;
      std::string strct;
      while(std::string::npos != (pos = className.find_first_of(':')) ) {
        ++nNamespaces;
        strct +="namespace ";
        strct+=className.substr(0,pos)+" { ";
        className = className.substr(pos+2);
      }
      strct +=" struct ";
      strct += className +" {\n";

      const auto sze = si->GetElements()->GetEntries();
      for(int i = 0; i< sze;++i) {
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
      if( not gInterpreter->Declare(strct.c_str()) ) {
        std::cout <<"Failed to declare shadow type"<<std::endl;
        return false;
      }
      //instantiate the wrapper to the shadow class
      std::string wrapperTypeName ="edmtst::Wrapper<"+fullClassName+">";
      std::string wrapperExpression = "#include \"Wrapper.h\"\n template class "+wrapperTypeName+";";
      if(!gInterpreter->Declare(wrapperExpression.c_str())) {
        std::cout <<"Could not declare wrapper"<<std::endl;
        return false;
      } else {
        return true;
      }
    }
  }
  return false;
}

std::string
storedTableClassName(TBranch* iBranch) {
  auto be = dynamic_cast<TBranchElement*>(iBranch);
  assert(be != nullptr);
  auto si = be->GetInfo();

  const std::string kSize("size");
  const std::string kObj("obj");

  for(int i = 0; i< si->GetNelement();++i) {
    auto e = si->GetElement(i);
    if (kObj != e->GetName()) {
      continue;
    }
    
    const std::string fullClassName = e->GetTypeName();
    auto shadowTClass = TClass::GetClass(fullClassName.c_str());
    assert(shadowTClass);

    auto si_shadow = shadowTClass->GetCurrentStreamerInfo();
    assert(si_shadow);

    auto sze = si_shadow->GetElements()->GetEntries();
    for(int j=0; j<sze; ++j) {
      auto e_shadow = si_shadow->GetElement(j);
      if (kSize == e_shadow->GetName()) {
        return e_shadow->GetTitle();
      }
    }
  }

  return std::string{};
}

int main()
{

  //gDebug = 3;
  TFile f("test_wrapper_branch.root");


  TTree* tree = dynamic_cast<TTree*>(f.Get("Events"));
  assert(tree != nullptr);

  const std::string branchName("Jets.");  
  auto jetBranch = tree->GetBranch(branchName.c_str());
  assert(nullptr != jetBranch);
  auto success = buildShadowClassFor(jetBranch);
  assert(success);

  auto wrapperClassName = std::string("edmtst::Wrapper<")+storedTableClassName(jetBranch)+">";
  std::cout <<"wrapper "<<wrapperClassName<<std::endl;
  auto cls = TClass::GetClass( wrapperClassName.c_str());
  assert(nullptr != cls);

  void* pJets = cls->New();
  assert(nullptr != pJets);

  jetBranch->SetAddress(&pJets);
  
  edmtst::Wrapper<JetTable> const& jetsW = *(reinterpret_cast<edmtst::Wrapper<JetTable>*>(pJets));

  auto nEntries = tree->GetEntries();
  for(decltype(nEntries) i = 0; i< nEntries; ++i) {
    
    jetBranch->GetEntry(i);
    auto jets = *(jetsW.object());
    std::cout <<"size "<<jets.size()<<std::endl;
    for( auto const& r: jets) {
      std::cout <<"eta "<<r.get<Eta>()<<" phi "<<r.get<Phi>()<<std::endl;
    }
  }
  
  cls->Destructor(pJets);
  return 0;
}
