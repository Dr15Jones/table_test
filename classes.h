#if !defined(classes_h)
#define classes_h

#include "JetTable.h"
#include "Wrapper.h"

namespace test_class {
  struct dictionary {
    JetTable a1;
    edmtst::Wrapper<JetTable> wa1;
  };
}

#endif
