#if !defined(Wrapper_h)
#define Wrapper_h

#include <memory>
#include "WrapperBase.h"

template<typename... Args>
class Table;

namespace edmtst {

  std::unique_ptr<TableReaderBase> make_table_reader_for(void const*) { return std::unique_ptr<TableReaderBase>{}; }

  template<typename... Args>
    std::unique_ptr<TableReaderBase> make_table_reader_for(Table<Args...> const* iObj) {return std::make_unique<TableReader<Table<Args...>>>(iObj);}

  template<typename T>
    class Wrapper : public WrapperBase {
  public:
    Wrapper() = default;

    explicit Wrapper(T&& iFrom): 
      obj{std::move(iFrom)},
      present{true} {}

    std::type_info const& heldType() const override final {
      return typeid(T);
    }

    std::unique_ptr<TableReaderBase> makeTableReader() const override final {
      return make_table_reader_for(&obj);
    }
    
    T const* object() const {
      if (present) {
        return &obj;
      }
      return nullptr;
    }
    
    bool isPresent() const {
      return present;
    }
    
    private:
      T obj;
      bool present = false;
  };
};

#endif
