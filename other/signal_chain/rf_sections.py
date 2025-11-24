from copy import deepcopy
from typing import Sequence

from .components import RFComponent, Loss
from .noise_figure import *
from .util import *


class ComponentChain(NoiseFigureStage):
    """Container of RF Components.

    Hold ordered sequence of components and compute their cascaded
    gain and noise figure.
    """

    def __init__(self, components: Sequence[RFComponent], desc: str) -> None:
        super().__init__()
        if len(components) == 0:
            raise ValueError("Cannot have section with no components")

        self.components: list[RFComponent] = list(deepcopy(components))
        self.desc: str = desc

        self.G_accum, self.F_accum = cascade_G_F(self.components)
        self.G, self.F = self.G_accum[-1], self.F_accum[-1]
        self.Te = Te(self.F)

        # initialize these to values that won't cause arithmetic
        # errors when an input hasn't been specified
        self.Si: float = DBM_INIT
        self.So: float = DBM_INIT
        self.Ni: float = DBM_INIT
        self.No: float = DBM_INIT
        self.SNR_i: float = DB_INIT
        self.SNR_o: float = DB_INIT

        self.warnings: list[str] = []

    def set_input(self, Si: float, Ni: float, BW: float) -> None:
        """Set the signal and noise inputs to the section and compute its output."""
        self.Si = Si
        self.Ni = Ni
        self.SNR_i = Si / (BW * Ni)

        self.components[0].set_input(Si, Ni, BW)
        for prev, next in zip(self.components[:-1], self.components[1:]):
            next.set_input(prev.So, prev.No, BW)

        self.So = self.components[-1].So
        self.No = self.components[-1].No
        self.SNR_o = self.So / (BW * self.No)

        self.warnings = [c.warning for c in self.components if c.warning]

    def __str__(self) -> str:
        """Printable output."""
        first, last = self.components[0], self.components[-1]
        ret = f"'{self.desc}' totals:\n"
        ret += f"  G     = {sdB(self.G)} dB\n"
        ret += f"  NF    = {sdB(self.F)} dB\n"
        ret += f"  Si    = {sdBm(first.Si)} dBm\n"
        ret += f"  So    = {sdBm(last.So)} dBm\n"
        ret += f"  Ni    = {sdBm(first.Ni)} dBm/Hz\n"
        ret += f"  No    = {sdBm(last.No)} dBm/Hz\n"
        ret += f"  SNR_i = {sdB(first.SNR_i)} dB\n"
        ret += f"  SNR_o = {sdB(last.SNR_o)} dB\n"
        if self.warnings:
            ret += (
                "\n".join("  Warning: " + warning for warning in self.warnings) + "\n"
            )
        return ret


class Coax(ComponentChain):
    """Cable loss as a signal chain section.

    Overload of signal chain section so cable loss can easily
    be integrated into model.
    """

    def __init__(self, loss_per_dB: float, length: float, desc: str = "") -> None:
        self.loss_per_dB: float = loss_per_dB
        self.length: float = length
        if not desc:
            desc = "coax"
        super().__init__((Loss(loss_per_dB * length),), desc)

    def __str__(self) -> str:
        """Printable output."""
        return f"{dB(self.G):.2f} ({self.length}*{self.loss_per_dB}) dB from coax '{self.desc}'\n"
