#if !defined(TABLEREADER_H)
#define TABLEREADER_H

#include <vector>
#include <utility>
#include <typeinfo>
#include <typeindex>

class TableReaderBase {
public:
  TableReaderBase () = default;
  virtual ~TableReaderBase() {}

  virtual std::vector<std::type_index> columnTypes() const = 0;

  virtual std::vector<std::pair<char const*, std::type_index>> columnDescriptions() const = 0;

  virtual size_t size() const = 0;

  virtual void * columnAddress(unsigned int iColumnIndex) const = 0;

  virtual const std::type_info* typeID() const = 0;
};


template <typename T>

class TableReader : public TableReaderBase {
public:

  TableReader( T const* iTable):
    m_table (iTable) {}

  size_t size() const override final { return m_table->size(); }
  void* columnAddress(unsigned int iColumnIndex) const override final {
    return m_table->columnAddressByIndex(iColumnIndex); 
  }

  std::vector<std::type_index> columnTypes() const override final {
    std::vector<std::type_index> returnValue;
    returnValue.reserve(T::kNColumns);
    using Layout = typename T::Layout;
    columnTypesImpl<0, T::kNColumns>(returnValue, static_cast<std::true_type*>(nullptr));
    return returnValue;
  }

  std::vector<std::pair<char const*, std::type_index>> 
  columnDescriptions() const override final {
    std::vector<std::pair<char const*, std::type_index>>  returnValue;
    returnValue.reserve(T::kNColumns);
    using Layout = typename T::Layout;
    columnDescImpl<0, T::kNColumns>(returnValue, static_cast<std::true_type*>(nullptr));
    return returnValue;
  } 

  const std::type_info* typeID() const override final {
    return &typeid(T);
  }

private:
  template <int I, int S>
  void columnTypesImpl(std::vector<std::type_index>& iV, std::true_type*) const {
    using Layout = typename T::Layout;
    iV.emplace_back( typeid( typename std::tuple_element<I,Layout>::type ) );
    columnTypesImpl<I+1, S>(iV,
			    static_cast<typename std::conditional< I+1 != S, 
			                                  std::true_type, 
 			                                  std::false_type>::type*>(nullptr));
  }

  template <int I, int S>
  void columnTypesImpl(std::vector<std::type_index>&, std::false_type*) const {};

  template <int I, int S>
  void columnDescImpl(std::vector<std::pair<char const*, std::type_index>>& iV, 
		      std::true_type*) const {
    using Layout = typename T::Layout;
    using ColumnType = typename std::tuple_element<I,Layout>::type;
    iV.emplace_back( ColumnType::label(), typeid( typename ColumnType::type ) );
    columnDescImpl<I+1, S>(iV,
			    static_cast<typename std::conditional< I+1 != S, 
			                                  std::true_type, 
 			                                  std::false_type>::type*>(nullptr));
  }

  template <int I, int S>
  void columnDescImpl(std::vector<std::pair<char const*, std::type_index>>&, 
		      std::false_type*) const {};

  T const* m_table;
};
#endif
