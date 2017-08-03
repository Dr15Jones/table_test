#include <array>
#include <memory>
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TLeaf.h"
#include "TDataType.h"
#include "Table.h"
#include "TableReader.h"
#include "TInterpreter.h"
#include <iostream>
#include <unordered_map>

constexpr const char kEta[] = "eta";
using Eta = Column<kEta,double>;

constexpr const char kPhi[] = "phi";
using Phi = Column<kPhi, double>;

using JetTable = Table<Eta,Phi>;


template<typename... Args>
class TableBranchReader {
  int m_size = 0;
  TBranch* m_sizeBranch = nullptr;
  std::array<TBranch*, sizeof...(Args)> m_branches;
  std::array<void*, sizeof...(Args)> m_addresses;
  unsigned int m_capacity = 0;

  using Layout = std::tuple<Args...>;

  template<int I>
  void setupBranch(std::string const& iName, TTree* iTree, std::true_type) {
    using Element = typename std::tuple_element<I, Layout>::type;
    m_branches[I] = iTree->GetBranch((iName+"_"+Element::label()).c_str());
    m_addresses[I] = nullptr;
    m_branches[I]->SetAddress(&m_addresses[I]);
    assert(nullptr != m_branches[I]);
    TClass* cls;
    EDataType dtype;
    auto check = m_branches[I]->GetExpectedType(cls,dtype);
    assert(0 == check);
    assert(dtype == typeToEDataType(typeid(typename Element::type)));
    setupBranch<I+1>( iName, iTree,
		      std::conditional_t<I+1<sizeof...(Args),
		      std::true_type,
		      std::false_type>{} );
  }
  template<int I>
  void setupBranch(std::string const& iName, TTree*, std::false_type) {}

  static EDataType typeToEDataType(std::type_index iType);
  
  void resizeIfNeeded() {
    if (m_size > m_capacity) {
      m_capacity = m_size;
      resizeElement<0>(std::true_type{});
    }
  }
  template <int I>
  void resizeElement(std::true_type) {
    using ElementType = typename std::tuple_element<I,Layout>::type;
    using Type = typename ElementType::type;
    delete [] static_cast<Type*>(m_addresses[I]);

    m_addresses[I] = new Type[m_capacity];
    resizeElement<I+1>(std::conditional_t<I+1<sizeof...(Args),
		       std::true_type,
		       std::false_type>{} );

  }
  template<int I>
  void resizeElement(std::false_type) {}


  template <int I>
  void deleteElement(std::true_type) {
    using ElementType = typename std::tuple_element<I,Layout>::type;
    using Type = typename ElementType::type;
    delete [] static_cast<Type*>(m_addresses[I]);

    deleteElement<I+1>(std::conditional_t<I+1<sizeof...(Args),
		       std::true_type,
		       std::false_type>{} );

  }
  template<int I>
  void deleteElement(std::false_type) {}

public:
  TableBranchReader(std::string const& iName, TTree*);

  ~TableBranchReader() {
    deleteElement<0>(std::true_type{});
  }

  TableView<Args...> getEntry(unsigned long iEntry) {
    m_sizeBranch->GetEntry(iEntry);

    resizeIfNeeded();
    int i=0;
    for(auto b: m_branches) {
      b->SetAddress(m_addresses[i]);
      b->GetEntry(iEntry);
      ++i;
    }
    return TableView<Args...>{static_cast<unsigned int>(m_size), m_addresses};
  }
};

template< typename... Args>
TableBranchReader<Args...>::TableBranchReader(std::string const& iName, TTree* iTree) {
  m_sizeBranch = iTree->GetBranch( (iName+"_size").c_str() );
  assert(m_sizeBranch != nullptr);
  m_sizeBranch->SetAddress(&m_size);

  setupBranch<0>(iName, iTree, std::true_type{} );
}


template<typename... Args>
EDataType
TableBranchReader<Args...>::typeToEDataType(std::type_index iType) {
  static const std::unordered_map<std::type_index,EDataType> s_map = {
    {typeid(char),kChar_t},
    {typeid(unsigned char),kUChar_t},
    {typeid(short),kShort_t},
    {typeid(unsigned short),kUShort_t},
    {typeid(int),kInt_t},
    {typeid(unsigned int), kUInt_t},
    {typeid(float),kFloat_t},
    {typeid(double),kDouble_t},
    {typeid(long), kLong_t},
    {typeid(unsigned long),kULong_t},
    {typeid(bool),kBool_t},
    {typeid(const char*),kCharStar} //is the address passed, or the address to the address?
  };

  auto it = s_map.find(iType);
  if(it== s_map.end()) {
    return kVoid_t;
  }
  return it->second;
}

using Jets = Table<Eta,Phi>;

int main()
{

  
  
  //gDebug = 3;
  TFile f("test_struct_branch.root");


  TTree* tree = dynamic_cast<TTree*>(f.Get("Events"));
  assert(tree != nullptr);

  Jets jets;
  Jets* pJets = &jets;

  //std::cout <<"size "<<jets.size()<<std::endl;

  std::string strct = "namespace bw { struct ";

  const std::string branchName("Jets.");  
  auto jetBranch = tree->GetBranch(branchName.c_str());
  {
    auto l = static_cast<TLeaf*>(jetBranch->GetListOfLeaves()->At(0));
    strct += l->GetTypeName()+4;
    strct += " {\n int size;\n";
  }

  auto branches = jetBranch->GetListOfBranches();
  for( int i = 1; i< branches->GetEntries(); ++i) {
    auto b = static_cast<TBranch*>(branches->At(i));
    auto leaves = b->GetListOfLeaves();
    //std::cout <<b->GetName()<<std::endl;
    for( int j=0; j< leaves->GetEntries(); ++j)  {
      auto l = static_cast<TLeaf*>(leaves->At(j));
      strct += " ";
      strct += l->GetTypeName();
      strct += "* ";
      strct += b->GetName()+branchName.size();
      strct += "; //[size]\n";
      //std::cout << l->GetName()<<" "<<l->GetTypeName()<<std::endl;
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
