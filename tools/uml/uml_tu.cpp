#include "libgsp/Graph.h"

// Force template instantiation so clang-uml sees full definitions/relationships
template class gsp::Graph<gsp::densematrix>;
template class gsp::Graph<gsp::sparsematrix>;