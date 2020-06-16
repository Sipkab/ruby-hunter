#ifndef RHFW_GEN_TYPES_H_
#define RHFW_GEN_TYPES_H_

#include <gen/fwd/types.h>

namespace rhfw {
@foreach type in this.typeDeclarations.values() :@opt type.toSourceDefinition()@
@
}  // namespace rhfw

#endif /* RHFW_GEN_TYPES_H_ */
