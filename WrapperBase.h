#if !defined(WrapperBase_h)
#define WrapperBase_h
#include <typeinfo>
#include <memory>
#include "TableReader.h"

namespace edmtst {
  class WrapperBase {
  public:
    virtual std::type_info const& heldType() const = 0;
    virtual std::unique_ptr<TableReaderBase> makeTableReader() const = 0;
  };
}

#endif
