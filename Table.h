#include <tuple>
#include <iterator>
#include <cassert>

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

template<typename T>
struct ColumnValues {

ColumnValues(T const* iBegin, size_t iSize):
  m_begin(iBegin), m_end(iBegin+iSize) {}

  T const* m_begin;
  T const* m_end;

  T const* begin() const { return m_begin; }
  T const* end() const { return m_end; }
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
class Table {
  using Layout = std::tuple<Args...>;

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

    ctrFiller<I-1>( std::forward<U>(iU)... );
  }

  template<int I>
    static void ctrFiller() {}


 public:
  template <typename T, typename... CArgs>
    Table(T const& iContainer, CArgs... iArgs): m_size(iContainer.size()) {
    static_assert( sizeof...(Args) == sizeof...(CArgs)+1, "Wrong number of arguments passed to Table constructor");
    ctrFiller<sizeof...(Args)-1>(iContainer, std::forward<CArgs>(iArgs)...);
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

};

template <typename... Args>
class TableView;

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

};




