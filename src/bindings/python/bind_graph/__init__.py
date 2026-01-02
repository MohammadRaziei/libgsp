"""Python bindings for libgsp graph library."""

from ._graph import *

__all__ = [
    # Enums
    'ShiftType',
    
    # Structs
    'Coord', 'Edge', 'Node',
    
    # Classes
    'VertexGraph', 'BaseGraph',
    'SparseGraph', 'DenseGraph',
    'SignalMask', 'SignalDouble', 'SignalFloat',
    'GraphSignalSparseDouble', 'GraphSignalDenseDouble',
    'EdgeGeneratorSparse', 'EdgeGeneratorDense',
    
    # Functions
    'readFile', 'writeFile',
]

# Create aliases for convenience
Signal = SignalDouble
GraphSignal = GraphSignalSparseDouble
EdgeGenerator = EdgeGeneratorSparse
Graph = SparseGraph