"""Cascaded RF Signal Chain Computations.

This module contains functionality to facilitate computing the cascaded gain
and noise figure of RF signal chains. The
"""

from copy import deepcopy
from math import log10, prod
from os.path import abspath
from sys import stdout
from typing import Sequence, TextIO

#
# constants
#

T0 = 290  # K
k = 1.380649e-23  # Boltzmann


#
# helper functions
#
def f72(f: float) -> str:
    """Shorthand for fstring floats."""
    return f"{f:7.2f}"


def dB(x: float) -> float:
    """Turn x into decibels."""
    return 10 * log10(x)


def dBm(x: float) -> float:
    """Turn x into dBm."""
    return 10 * log10(x) + 30


def undB(x: float) -> float:
    """Undo decibel operation."""
    return 10 ** (x / 10)


def undBm(x: float) -> float:
    """Undo dBm operation."""
    return 10 ** ((x - 30) / 10)


def Te(F: float) -> float:
    """Calculate effective noise temperature from noise figure."""
    return T0 * (F - 1)


def mismatch_loss_dB(VSWR: float) -> float:
    """Calculate the power loss from reflections due to VSWR."""
    refl_coeff = (VSWR - 1) / (VSWR + 1)
    return dB(1 - refl_coeff**2)


#
# cascaded gain and noise figure computation
#


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


# def cascade_G_F(stages: Sequence[NoiseFigureStage]) -> tuple[float, float]:
#     """Compute the Gain and Noise figure of ordered stages.
#
#     Args:
#         stages: ordered sequence of stages that define a gain and noise figure.
#
#     Returns:
#         (G, F): Total Gain and Noise Figure of the sequence (both linear)
#             G = G1*G2*...*Gn
#             F = F1 + (F2 - 1)/G1 + (F3  -1)/(G1*G2) + ... + (Fn - 1)/(G1*G2*...*Gn-1)
#     """
#     assert len(stages) > 0
#
#     # G1, G1*G2, G1*G2*...*Gn
#     Gees: list[float] = [s.G for s in stages]
#     Gees = [prod(Gees[: i + 1]) for i in range(len(Gees))]
#
#     Effs: list[float] = [s.F for s in stages]
#     F: float = Effs[0]
#
#     for g, f in zip(Gees[:-1], Effs[1:]):
#         F += (f - 1) / g
#
#     return Gees[-1], F
#


def cascade_G_F(stages: Sequence[NoiseFigureStage]) -> tuple[list[float], list[float]]:
    """Compute the Gain and Noise figure of ordered stages.

    Args:
        stages: ordered sequence of stages that define a gain and noise figure.

    Returns:
        (G, F): Total Gain and Noise Figure of the sequence (both linear)
            G = G1*G2*...*Gn
            F = F1 + (F2 - 1)/G1 + (F3  -1)/(G1*G2) + ... + (Fn - 1)/(G1*G2*...*Gn-1)
    """
    assert len(stages) > 0

    Gees: list[float] = [s.G for s in stages]
    Effs: list[float] = [s.F for s in stages]

    # G1, G1*G2, G1*G2*...*Gn
    G_accum: list[float] = [prod(Gees[: i + 1]) for i in range(len(Gees))]

    F_accum: list[float] = [Effs[0]]

    for g, f in zip(G_accum[:-1], Effs[1:]):
        F_accum.append(F_accum[-1] + (f - 1) / g)

    return G_accum, F_accum


class Component(NoiseFigureStage):
    """Model for an RF Component inside a signal chain.

    Create component with RF parameters and computes the components ouput
    for a given input.
    """

    def __init__(
        self, gain_dB: float, NF_dB: float, VSWR: float, Pin_warn_dBm: float, desc: str
    ) -> None:
        super().__init__()

        self.G: float = undB(gain_dB)
        self.F: float = undB(NF_dB)
        self.VSWR: float = VSWR
        self.desc: str = desc

        self.Si: float = 0
        self.So: float = 0
        self.Si_warn: float = undBm(Pin_warn_dBm)
        self.Ni: float = 0
        self.No: float = 0
        self.SNR_i: float = 0
        self.SNR_o: float = 0

        self.warning: str = ""

    def set_input(self, Si: float, Ni: float, BW: float) -> None:
        """Set the signal and noise inputs to the component and compute its output."""
        self.Si = Si
        self.Ni = Ni

        # signal
        self.So = self.Si * self.G

        self.warning = ""  # need to reset each time
        if self.Si > self.Si_warn:
            if self.G > 1:
                self.warning = f"'{self.desc}' output too high: {dBm(self.So):.1f}, {dBm(self.Si_warn * self.G):.1f}"
            else:
                self.warning = f"'{self.desc}' input too high: {dBm(self.Si):.1f}, {dBm(self.Si_warn):.1f}"

        # noise
        self.No = self.G * (self.Ni + k * Te(self.F))
        if self.No < k * T0:
            self.No = k * T0

        # SNR
        self.SNR_i = Si / (BW * Ni)
        self.SNR_o = self.So / (BW * self.No)


def Amp(
    gain_dB: float,
    NF_dB: float,
    VSWR: float = 1,
    OP1dB_dBm: float = 999,
    desc: str = "",
) -> Component:
    """Generate a component representing typical amplifier parameters."""
    return Component(gain_dB, NF_dB, VSWR, OP1dB_dBm - gain_dB, desc)


def Mixer(
    loss_dB: float,
    NF_dB: float,
    VSWR: float = 1,
    IP1dB_dBm: float = 999,
    desc: str = "",
) -> Component:
    """Generate a component representing typical mixer parameters."""
    return Component(-loss_dB, NF_dB, VSWR, IP1dB_dBm, desc)


def Loss(
    loss_dB: float, VSWR: float = 1, Pin_warn_dBm: float = 999, desc: str = ""
) -> Component:
    """Generate a component representing generic loss parameters."""
    return Component(-loss_dB, loss_dB, VSWR, Pin_warn_dBm, desc)


def AttnPad(loss_dB: float) -> Component:
    """Generate a component representing an attenuator pad."""
    return Component(-loss_dB, loss_dB, 1, 999, f"{loss_dB:.0f} dB pad")


#
# RF Signal chain sections
#


class Section(NoiseFigureStage):
    """Container of RF Components.

    Hold ordered sequence of components and compute their cascaded
    gain and noise figure.
    """

    def __init__(self, components: Sequence[Component], desc: str) -> None:
        super().__init__()
        if len(components) == 0:
            raise ValueError("Cannot have section with no components")

        self.components: list[Component] = list(deepcopy(components))
        self.desc: str = desc

        self.G_accum, self.F_accum = cascade_G_F(self.components)
        self.G, self.F = self.G_accum[-1], self.F_accum[-1]
        # self.Te = Te(self.F)

        self.Si: float = 0
        self.So: float = 0
        self.Ni: float = 0
        self.No: float = 0
        self.SNR_i: float = 0
        self.SNR_o: float = 0

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
        ret += f"  G     = {f72(dB(self.G))} dB\n"
        ret += f"  NF    = {f72(dB(self.F))} dB\n"
        ret += f"  Si    = {f72(dBm(first.Si))} dBm\n"
        ret += f"  So    = {f72(dBm(last.So))} dBm\n"
        ret += f"  Ni    = {f72(dBm(first.Ni))} dBm/Hz\n"
        ret += f"  No    = {f72(dBm(last.No))} dBm/Hz\n"
        ret += f"  SNR_i = {f72(dB(first.SNR_i))} dB\n"
        ret += f"  SNR_o = {f72(dB(last.SNR_o))} dB\n"
        if self.warnings:
            ret += (
                "\n".join("  Warning: " + warning for warning in self.warnings) + "\n"
            )
        return ret

    def param_strs(self, SNR_0: float) -> dict[str, str]:
        """Return the section parameters as formatted strings."""
        first, last = self.components[0], self.components[-1]
        ret = {
            "Name": f"{self.desc}",
            "G": f"{f72(dB(self.G))}",
            "NF": f"{f72(dB(self.F))}",
            "Si": f"{f72(dBm(first.Si))}",
            "So": f"{f72(dBm(last.So))}",
            "Ni": f"{f72(dBm(first.Ni))}",
            "No": f"{f72(dBm(last.No))}",
            "SNR_i": f"{f72(dB(first.SNR_i))}",
            "SNR_o": f"{f72(dB(last.SNR_o))}",
            "SNR_loss": f"{f72(dB(SNR_0/last.SNR_o))}",
        }
        # normalize the length of every entry for this section so
        # the spacing works out when printing the inline thing
        vl = max(len(val) for val in ret.values())
        return {key: val.rjust(vl) for key, val in ret.items()}


class Coax(Section):
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


def analyze_sections(
    sections: Sequence[Section], Pin_dBm: float, BW_MHz: float, **kwargs
) -> None:
    """
    Perform cascade analysis on signal chain sections.

    By default the input noise is thermal (k*T0). Optionally, the input
    noise power density (NPSD) can be set in one of several ways.
    **kwargs:
        Ni (float): set NPSD directly.
        Ni_dBm (float): Converts power to NPSD using the input bandwidth.
        Tin (float): set NPSD from noise temperature, k*Tin.
        SNR (float): Calculates the NPSD required to get SNR using input power and bandwidth.

        file (TextIO | str): Redirect script output to a file / filename.
    """
    if "file" in kwargs:
        if isinstance(kwargs["file"], TextIO) and kwargs["file"] != stdout:
            file = kwargs["file"]
            if file.closed:
                file = open(file.name, "w")
        elif isinstance(kwargs["file"], str):
            output_file = abspath(kwargs["file"])
            file = open(output_file, "w")
        else:
            raise TypeError("Ouput file invalid type")
    else:
        file: TextIO = stdout

    # less typing
    fprint = lambda s: print(s, file=file)
    fprint(f"{kwargs = }\n")

    # input param unit conversion
    Si: float = undBm(Pin_dBm)
    BW: float = BW_MHz * 1e6

    # determine the input noise
    match kwargs:
        case {"Ni": _Ni}:
            Ni = _Ni
        case {"Ni_dBm": Ni_dBm}:
            Ni = undBm(Ni_dBm) / BW
            if Ni < k * T0:
                Ni = k * T0
                fprint(
                    f"WARNING: Requested noise power requires noise density below thermal; setting to default Ni = {f72(dBm(Ni))} dBm/Hz"
                )
        case {"Tin": Tin}:
            Ni = k * Tin
        case {"SNR": SNR_in}:
            Ni = undBm(Pin_dBm - SNR_in) / BW
            if Ni < k * T0:
                Ni = k * T0
                fprint(
                    f"WARNING: Requested SNR requires noise density below thermal; setting to default Ni = {f72(dBm(Ni))} dBm/Hz"
                )
        case _:
            Ni = k * T0
            fprint(f"Default Ni = {f72(dBm(Ni))} dBm/Hz")

    #
    # calculations
    #

    # run calculations for individual stages
    sections[0].set_input(Si, Ni, BW)
    for prev, next in zip(sections[:-1], sections[1:]):
        next.set_input(prev.components[-1].So, prev.components[-1].No, BW)

    # run calculation for full signal chain
    Gees, Effs = cascade_G_F(sections)

    #
    # printing
    #
    start, end = sections[0], sections[-1]

    # printing individual stages
    PARAM_NAME_WIDTH = 9
    inline_lines: dict[str, list[str]] = {}
    for section, Gcas, Fcas in zip(sections, Gees, Effs):
        fprint(section)
        section_width: int = 0
        for param, val in section.param_strs(start.Si / (start.Ni * BW)).items():
            section_width = len(val)
            if param not in inline_lines:
                inline_lines[param] = [param.ljust(PARAM_NAME_WIDTH)]
            inline_lines[param].append(val)

        if "G total" not in inline_lines:
            inline_lines["G total"] = ["G total".ljust(PARAM_NAME_WIDTH)]
        if "NF total" not in inline_lines:
            inline_lines["NF total"] = ["NF total".ljust(PARAM_NAME_WIDTH)]

        inline_lines["G total"].append(f72(dB(Gcas)).rjust(section_width))
        inline_lines["NF total"].append(f72(dB(Fcas)).rjust(section_width))

    for line in inline_lines.values():
        fprint("| " + " | ".join(line) + " |")

    # print system totals
    fprint("")
    fprint("System Totals:")
    fprint(f"  G     = {f72(dB(Gees[-1]))} dB")
    fprint(f"  NF    = {f72(dB(Effs[-1]))} dB")
    fprint(f"  Si    = {f72(dBm(start.Si))} dBm")
    fprint(f"  So    = {f72(dBm(end.So))} dBm")
    fprint(
        f"  Ni    = {f72(dBm(start.Ni))} dBm/Hz ({f72(dBm(start.Ni * BW))} dBm/{BW_MHz} MHz)"
    )
    fprint(
        f"  No    = {f72(dBm(end.No))} dBm/Hz ({f72(dBm(end.No * BW))} dBm/{BW_MHz} MHz)"
    )
    fprint(f"  SNR_i = {f72(dB(start.Si/(start.Ni * BW)))} dB ({BW_MHz} MHz)")
    fprint(f"  SNR_o = {f72(dB(end.So/(end.No * BW)))} dB ({BW_MHz} MHz)")

    if file != stdout:
        file.close()


def example() -> None:
    """Example 10.2 in Pozar's Microwave Engineering, 4e."""

    # make a list of one or more components
    components: list[Component] = [
        Amp(10, 2, OP1dB_dBm=0, desc="lna"),
        Loss(1, desc="bpf"),
        Mixer(3, 4, desc="mixer"),
    ]

    # make a list of one or more sections
    sections: list[Section] = [
        Section(desc="Example 10.2", components=components),
        Section(desc="Example 10.2", components=components),
    ]

    # perform analysis with desired inputs
    analyze_sections(
        sections=sections,
        Pin_dBm=-82.8,
        BW_MHz=10,
        Tin=150,
        # SNR=70,
        file="./sgc_example2.txt",
    )


if __name__ == "__main__":
    example()
