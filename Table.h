#if !defined(TABLE_H)
#define TABLE_H
#include <tuple>
#include <iterator>
#include <cassert>

/* Column is a type used to declare the purpose of what is stored */
template <const char* LABEL, typename T>
  struct Column{
    using type = T;
    static constexpr char const * const kLabel = LABEL;
  };

namespace tablehelp {

template<int I>
  struct FoundIndex {
    static constexpr int index = I; 
  };

template <int I, typename T, typename TPL>
  struct GetIndex {
    static constexpr int index = std::conditional<std::is_same<T, typename std::tuple_element<I,TPL>::type >::value,
      FoundIndex<I>, 
      GetIndex<I+1, T,TPL>>::type::index;
  };
 
}

/* ColumnValues: Give container like access to values in a column */
template<typename T>
struct ColumnValues {

ColumnValues(T const* iBegin, size_t iSize):
  m_begin(iBegin), m_end(iBegin+iSize) {}

  T const* m_begin;
  T const* m_end;

  T const* begin() const { return m_begin; }
  T const* end() const { return m_end; }
};


template <typename... Args>
class RowView {
  using Layout = std::tuple<Args...>;
  std::array<void*, sizeof...(Args)> m_values;

 public:
  explicit RowView( std::array<void*, sizeof...(Args)> const& iValues):
  m_values{iValues} {}

  template<typename U>
    typename U::type const& get() const {
    return *(static_cast<typename U::type*>(columnAddress<U>()));
  }

  template<typename U>
    void * columnAddress() const {
    return m_values[tablehelp::GetIndex<0,U,Layout>::index];
  }
  
};

template<int I, typename... Args>
  struct TableItrAdvance {
    using Layout = std::tuple<Args...>;
    static void advance(std::array<void*, sizeof...(Args)>& iArray, int iStep) {
      using Type = typename std::tuple_element<I,Layout>::type::type;
      TableItrAdvance<I-1,Args...>::advance(iArray,iStep);
      auto p = static_cast<Type*>( iArray[I]);
      iArray[I] = p+iStep;
    }
  };

template<typename... Args>
struct TableItrAdvance<0,Args...> {
    using Layout = std::tuple<Args...>;
    static void advance(std::array<void*, sizeof...(Args)>& iArray, int iStep) {
      using Type = typename std::tuple_element<0,Layout>::type::type;
      auto p = static_cast<Type*>( iArray[0]);
      iArray[0] = p+iStep;
    }
};

template <typename... Args>
class TableItr {
  using Layout = std::tuple<Args...>;
  std::array<void*, sizeof...(Args)> m_values;
 public:

  enum EndType { kEnd };

  using value_type = RowView<Args...>;

  explicit TableItr( std::array<void*, sizeof...(Args)> const& iValues):
  m_values{iValues} {}

  explicit TableItr( std::array<void*, sizeof...(Args)> const& iValues, int iOffset):
  m_values{iValues} {
    TableItrAdvance<sizeof...(Args)-1, Args...>::advance(m_values, iOffset);
  }

  explicit TableItr( std::array<void*, sizeof...(Args)> const& iValues, unsigned int iOffset, EndType):
  m_values{iValues} {
    TableItrAdvance<0, Args...>::advance(m_values, static_cast<int>(iOffset));
  }


  value_type operator*() const {
    return value_type{m_values};
  }

  TableItr& operator++() {
    TableItrAdvance<sizeof...(Args)-1, Args...>::advance(m_values,1);
    return *this;
  }

  bool operator==( const TableItr<Args...>& iOther) {
    return m_values[0] == iOther.m_values[0];
  }

  bool operator!=( const TableItr<Args...>& iOther) {
    return m_values[0] != iOther.m_values[0];
  }

};

template <int I, typename... Args>
  struct TableArrayDtr {
    static void dtr(std::array<void*, sizeof...(Args)>& iArray) {
      using Layout = std::tuple<Args...>;
      using Type = typename std::tuple_element<I,Layout>::type::type;
      delete [] static_cast<Type*>(iArray[I]);
      TableArrayDtr<I-1, Args...>::dtr(iArray);
  }
};

template <typename... Args>
struct TableArrayDtr<0, Args...> {
    static void dtr(std::array<void*, sizeof...(Args)>& iArray) {
      using Layout = std::tuple<Args...>;
      using Type = typename std::tuple_element<0,Layout>::type::type;
      delete [] static_cast<Type*>(iArray[0]);
  }
};


template <typename... Args>
class TableView;

template <typename... Args>
class Table {
  std::array<void *, sizeof...(Args)> m_values;
  unsigned int m_size;

  template<int I, typename T, typename... U>
    void ctrFiller(T const& iContainer, U... iU) {
    assert(iContainer.size() == m_size);
    using Type = typename std::tuple_element<I,Layout>::type::type;
    Type  * temp = new Type [m_size];
    unsigned int index = 0;
    for( auto const& v: iContainer) {
      temp[index] = v;
      ++index;
    }
    m_values[I] = temp;

    ctrFiller<I+1>(std::forward<U>(iU)... );
  }

  template<int I>
    static void ctrFiller() {}


 public:
  using Layout = std::tuple<Args...>;
  using const_iterator = TableItr<Args...>;

  template <typename T, typename... CArgs>
    Table(T const& iContainer, CArgs... iArgs): m_size(iContainer.size()) {
    static_assert( sizeof...(Args) == sizeof...(CArgs)+1, "Wrong number of arguments passed to Table constructor");
    ctrFiller<0>(iContainer, std::forward<CArgs>(iArgs)...);
  }

 Table() : m_size(0) {
  }

  ~Table() {
    TableArrayDtr<sizeof...(Args)-1,Args...>::dtr(m_values);
  }
  unsigned int size() const {
    return m_size;
  }

  template<typename U>
    typename U::type const& get(size_t iRow) const {
    return *(static_cast<typename U::type*>(columnAddress<U>())+iRow);
  }

  template<typename U>
    ColumnValues<typename U::type> column() const {
    return ColumnValues<typename U::type>{static_cast<typename U::type*>(columnAddress<U>()), m_size};
  }

  template<typename U>
    void * columnAddress() const {
    return m_values[tablehelp::GetIndex<0,U,Layout>::index];
  }

  template<typename U>
    void * columnAddressWorkaround( U const*) const {
    return columnAddress<U>();
  }

  template<typename... U>
    TableView<U...> view() const;

  const_iterator begin() const { return const_iterator{m_values}; }
  const_iterator end() const { return const_iterator{m_values,size(),const_iterator::kEnd}; }

};


template <int I, typename TV, typename T>
  struct TableViewFiller {
    static void fillArray( std::array<void*, TV::kNColumns>& iArray,  T const& iTable) {
      using Layout = typename TV::Layout;
      using ElementType = typename std::tuple_element<I, Layout>::type;
      //iArray[I] = iTable.columnAddress<ElementType>();
      iArray[I] = iTable.columnAddressWorkaround(static_cast<ElementType*>(nullptr));
      TableViewFiller<I-1, TV, T>::fillArray(iArray, iTable);
    }
  };

template <typename TV, typename T>
  struct TableViewFiller<0,TV, T> {
  static void fillArray( std::array<void*, TV::kNColumns>& iArray, T const& iTable) {
    using Layout = typename TV::Layout;
    using ElementType = typename std::tuple_element<0, Layout>::type;
    //iArray[0] = iTable.columnAddress<ElementType>();
    iArray[0] = iTable.columnAddressWorkaround(static_cast<ElementType*>(nullptr));
  }
};


template <typename... Args>
class TableView {

  std::array<void*, sizeof...(Args)> m_values;
  unsigned int m_size;

 public:
  using Layout = std::tuple<Args...>;
  static constexpr const size_t kNColumns = sizeof...(Args);
  using const_iterator = TableItr<Args...>;

  template <typename... OArgs>
    TableView( Table<OArgs...> const& iTable):
  m_size(iTable.size()) {
    TableViewFiller<sizeof...(Args)-1, TableView<Args...>, Table<OArgs...>>::fillArray(m_values, iTable);
  }

  unsigned int size() const {
    return m_size;
  }

  template<typename U>
    typename U::type const& get(size_t iRow) const {
    return static_cast<typename U::type*>(columnAddress<U>())+iRow;
  }

  template<typename U>
    ColumnValues<typename U::type> column() const {
    return ColumnValues<typename U::type>{static_cast<typename U::type*>(columnAddress<U>()), m_size};
  }

  template<typename U>
    void * columnAddress() const {
    return m_values[tablehelp::GetIndex<0,U,Layout>::index];
  }

  const_iterator begin() const { return const_iterator{m_values}; }
  const_iterator end() const { return const_iterator{m_values,size(),const_iterator::kEnd}; }

};

template<typename... T>
  template<typename... U>
TableView<U...> Table<T...>::view() const {
  return TableView<U...>{*this};
};


/* Table Type Manipulation */
template <typename T1, typename T2> struct AddColumns;
template <typename... T1, typename... T2>
  struct AddColumns<Table<T1...>, std::tuple<T2...>> {
  using type = Table<T1...,T2...>;
};

template <typename T1, typename T2>
  using AddColumns_t = typename AddColumns<T1,T2>::type;


template <typename LHS, typename E, typename RHS> struct RemoveColumnCheck;
template <typename LHS, typename E, typename T, typename... U>
  struct RemoveColumnCheck<LHS, E, std::tuple<T,U...>> {
  using type =   typename std::conditional<std::is_same<E, T>::value,
    typename AddColumns<LHS,std::tuple<U...>>::type, 
    typename RemoveColumnCheck<typename AddColumns<LHS,std::tuple<T>>::type, E, std::tuple<U...>>::type>::type;
};

template <typename LHS, typename E>
  struct RemoveColumnCheck<LHS, E, std::tuple<>> {
  using type = LHS;
};

template <typename TABLE, typename E>
  struct RemoveColumn {
    using type = typename RemoveColumnCheck<Table<>,E, typename TABLE::Layout>::type;
  };

template <typename TABLE, typename E>
  using RemoveColumn_t = typename RemoveColumn<TABLE,E>::type;



#endif /*TABLE_H*/
