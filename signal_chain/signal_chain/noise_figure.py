"""Cascaded Noise figure computation."""

from math import prod
from typing import Sequence


class NoiseFigureStage:
    """Base class for stage used in cascaded noise figure calculations"""

    def __init__(self, G: float = -1, F: float = -1) -> None:
        self.G: float = G
        self.F: float = F

    def valid(self) -> bool:
        """Determine if the stage has valid parameters.

        Power gain cannot be negative and Noise figure
        cannot be less than one.
        """
        return self.G > 0 and self.F > 1


def cascade_G_F(stages: Sequence[NoiseFigureStage]) -> tuple[list[float], list[float]]:
    """Compute the Gain and Noise figure of ordered stages.

    Args:
        stages: ordered sequence of stages that define a gain and noise figure.

    Returns:
        (G, F): Total Gain and Noise Figure of the sequence (both linear)
            G = [G1,
                 G1*G2,
                 G1*G2*G3,
                 ...
                 G1*G2*...*Gn]
            F = [F1,
                 F1 + (F2 - 1)/G1,
                 F1 + (F2 - 1)/G1 + (F3  -1)/(G1*G2),
                 F1 + (F2 - 1)/G1 + (F3  -1)/(G1*G2) + ... + (Fn - 1)/(G1*G2*...*Gn-1)]
    """
    if len(stages) == 0:
        raise ValueError("Calculations require one or more stages")
    if not all(s.valid() for s in stages):
        raise ValueError("Stage has invalid or uninitialized parameters")

    Gees: list[float] = [s.G for s in stages]
    Effs: list[float] = [s.F for s in stages]

    G_accum: list[float] = [prod(Gees[: i + 1]) for i in range(len(Gees))]

    F_accum: list[float] = [Effs[0]]

    for g, f in zip(G_accum[:-1], Effs[1:]):
        F_accum.append(F_accum[-1] + (f - 1) / g)

    return G_accum, F_accum
