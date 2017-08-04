#if !defined(columns_h)
#define columns_h
#include "Table.h"

constexpr const char kEta[] = "eta";
using Eta = Column<kEta,double>;

constexpr const char kPhi[] = "phi";
using Phi = Column<kPhi, double>;

constexpr const char kPx[] = "p_x";
using Px = Column<kPx, double>;

constexpr const char kPy[] = "p_z";
using Py = Column<kPy, double>;

constexpr const char kPz[] = "p_z";
using Pz = Column<kPz, double>;

constexpr const char kEnergy[] = "energy";
using Energy = Column<kEnergy,double>;

#endif
