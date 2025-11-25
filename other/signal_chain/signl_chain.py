"""Library that allows for power and noise calculations for an
RF signal chain.

"""

import os, sys
from copy import deepcopy
from math import log10, prod
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


def sdB(x: float) -> str:
    """Turn x into a formatted string of decibels."""
    return f72(dB(x))


def dBm(x: float) -> float:
    """Turn x into dBm."""
    return 10 * log10(x) + 30


def sdBm(x: float) -> str:
    """Turn x into a formatted string of dBm."""
    return f72(dBm(x))


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


def return_loss_to_VSWR(return_loss_dB: float) -> float:
    """Calculate VSWR from returnn loss."""
    return 1 + 10 ** (-return_loss_dB / 20) / 1 - 10 ** (-return_loss_dB / 20)


"""Cascaded Noise figure computation."""


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


#
# initialization values for dB units that don't
# cause arithmetic errors when converting back
# to printable values
#
DB_INIT = undB(-999.99)
DBM_INIT = undBm(-999.99)


class RFComponent(NoiseFigureStage):
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

        self.Si: float = DBM_INIT
        self.So: float = DBM_INIT
        self.Si_warn: float = undBm(Pin_warn_dBm)
        self.Ni: float = DBM_INIT
        self.No: float = DBM_INIT
        self.SNR_i: float = DB_INIT
        self.SNR_o: float = DB_INIT

        self.warning: str = ""

    def set_input(self, Si: float, Ni: float, BW: float) -> None:
        """Set the signal and noise inputs to the component and compute its output."""
        self.Si = Si
        self.Ni = Ni

        # signal
        self.So = self.Si * self.G

        self.warning = ""
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
) -> RFComponent:
    """Generate a component representing typical amplifier parameters."""
    return RFComponent(gain_dB, NF_dB, VSWR, OP1dB_dBm - gain_dB, desc)


def Mixer(
    loss_dB: float,
    NF_dB: float,
    VSWR: float = 1,
    IP1dB_dBm: float = 999,
    desc: str = "",
) -> RFComponent:
    """Generate a component representing typical mixer parameters."""
    return RFComponent(-loss_dB, NF_dB, VSWR, IP1dB_dBm, desc)


def Loss(
    loss_dB: float, VSWR: float = 1, Pin_warn_dBm: float = 999, desc: str = ""
) -> RFComponent:
    """Generate a component representing generic loss parameters."""
    return RFComponent(-loss_dB, loss_dB, VSWR, Pin_warn_dBm, desc)


def AttnPad(loss_dB: float) -> RFComponent:
    """Generate a component representing an attenuator pad."""
    return RFComponent(-loss_dB, loss_dB, 1, 999, f"{loss_dB:.0f} dB pad")


class ComponentChain(NoiseFigureStage):
    """Container of RF Components.

    Hold ordered sequence of components and compute their cascaded
    gain and noise figure.
    """

    def __init__(self, components: Sequence[RFComponent], desc: str) -> None:
        super().__init__()
        if len(components) == 0:
            raise ValueError("Cannot have component chain with no components")

        self.components: list[RFComponent] = list(deepcopy(components))
        self.desc: str = desc

        self.G_accum, self.F_accum = cascade_G_F(self.components)
        self.G, self.F = self.G_accum[-1], self.F_accum[-1]
        self.Te = Te(self.F)

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
            ret += "\n".join("  Warning: " + w for w in self.warnings) + "\n"
        return ret


class Coax(ComponentChain):
    """Cable loss as a signal chain section.

    Overload of so cable loss can easily
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
    sections: Sequence[ComponentChain], Pin_dBm: float, BW_MHz: float, **kwargs
) -> None:
    """
    Perform cascade analysis on signal chain sections.

    By default the input noise is thermal (k*T0). Optionally, the input
    noise power density (NPSD) can be set in one of several ways.
    **kwargs:
        Ni (float): set NPSD directly, in dBm/Hz
        Ni_dBm (float): Converts power to NPSD using the input bandwidth.
        Tin (float): set NPSD from noise temperature, k*Tin.
        SNR (float): Calculates the NPSD required to get SNR using input power and bandwidth.

        file (TextIO | str): Redirect script output to a file / filename.
    """
    file: TextIO = sys.stdout
    if "file" in kwargs:
        if isinstance(kwargs["file"], TextIO) and kwargs["file"] != sys.stdout:
            file = kwargs["file"]
            if file.closed:
                file = open(file.name, "w")
        elif isinstance(kwargs["file"], str):
            output_file = os.path.abspath(kwargs["file"])
            file = open(output_file, "w")
        else:
            raise TypeError("Ouput file invalid type")

    # less typing
    fprint = lambda s: print(s, file=file)
    fprint(f"{kwargs = }\n")

    # input param unit conversion
    Si: float = undBm(Pin_dBm)
    BW: float = BW_MHz * 1e6

    # determine the input noise
    match kwargs:
        case {"Ni": _Ni}:
            Ni = undBm(_Ni)
        case {"Ni_dBm": Ni_dBm}:
            Ni = undBm(Ni_dBm) / BW
            if Ni < k * T0:
                fprint(
                    f"WARNING: Requested noise power requires noise density below thermal ({sdBm(Ni)} dBm/Hz)"
                )
        case {"Tin": Tin}:
            Ni = k * Tin
        case {"SNR": SNR_in}:
            Ni = undBm(Pin_dBm - SNR_in) / BW
            if Ni < k * T0:
                fprint(
                    f"WARNING: Requested SNR requires noise density below thermal ({sdBm(Ni)} dBm/Hz)"
                )
        case _:
            Ni = k * T0
            fprint(f"Default Ni = {sdBm(Ni)} dBm/Hz\n")

    #
    # calculations
    #
    sections = deepcopy(sections)

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

    for section in sections:
        fprint(section)

    fprint(_generate_inline_text(sections, Gees, Effs))

    # print system totals
    fprint("")
    fprint("System Totals:")
    fprint(f"  G    = {sdB(Gees[-1])} dB")
    fprint(f"  NF   = {sdB(Effs[-1])} dB")
    fprint(f"  Si   = {sdBm(start.Si)} dBm")
    fprint(f"  So   = {sdBm(end.So)} dBm")
    fprint(f"  Ni   = {sdBm(start.Ni)} dBm/Hz ({sdBm(start.Ni * BW)} dBm/{BW_MHz} MHz)")
    fprint(f"  No   = {sdBm(end.No)} dBm/Hz ({sdBm(end.No * BW)} dBm/{BW_MHz} MHz)")
    fprint(f"  SNR_i = {sdB(start.Si/(start.Ni * BW))} dB ({BW_MHz} MHz)")
    fprint(f"  SNR_o = {sdB(end.So/(end.No * BW))} dB ({BW_MHz} MHz)")

    if file != sys.stdout:
        file.close()


def _generate_inline_text(
    sections: Sequence[ComponentChain], Gees: list[float], Effs: list[float]
) -> str:
    """Format stuff into lines"""

    stage_params: dict[str, list[str]] = {
        "Name": [],
        "G": [],
        "NF": [],
        "Si": [],
        "So": [],
        "Ni": [],
        "No": [],
        "SNRi": [],
        "SNRo": [],
        "SNR loss": [],
    }

    accum_params: dict[str, list[str]] = {
        "G tot": [],
        "NF tot": [],
        "SNR loss": [],
    }

    # figure out the longest parameter name
    width: int = max(
        max(len(k) for k in stage_params.keys()),
        max(len(k) for k in accum_params.keys()),
    )

    # make each line start with its paramater, padded to the correct length
    stage_params = {s: [s.ljust(width)] for s in stage_params.keys()}
    accum_params = {s: [s.ljust(width)] for s in accum_params.keys()}

    # more functions to make string formatting shorter
    # right-justified string decibels
    rsdB = lambda x: sdB(x).rjust(width)
    rsdBm = lambda x: sdBm(x).rjust(width)

    for section, G, F in zip(sections, Gees, Effs):
        width = max(len(section.desc), len(sdB(1)))

        stage_params["Name"].append(section.desc.rjust(width))
        stage_params["G"].append(rsdB(section.G))
        stage_params["NF"].append(rsdB(section.F))
        # stage_params["Si"].append(lsdBm(section.Si))
        stage_params["So"].append(rsdBm(section.So))
        # stage_params["Ni"].append(lsdBm(section.Ni))
        stage_params["No"].append(rsdBm(section.No))
        # stage_params["SNRi"].append(lsdBm(section.SNR_i))
        # stage_params["SNRo"].append(lsdB(section.SNR_o))
        # stage_params["SNR loss"].append(rsdB(section.SNR_i / section.SNR_o))

        accum_params["G tot"].append(rsdB(G))
        accum_params["NF tot"].append(rsdB(F))
        accum_params["SNR loss"].append(rsdB(sections[0].SNR_i / section.SNR_o))

    lines: list[str] = []
    for param_list in stage_params.values():
        if len(param_list) > 1:
            lines.append("| " + " | ".join(param_list) + " |")

    lines.append("-" * len(lines[-1]))

    for param_list in accum_params.values():
        if len(param_list) > 1:
            lines.append("| " + " | ".join(param_list) + " |")

    return "\n".join(lines)


def example() -> None:
    """Example 10.2 in Pozar's Microwave Engineering, 4e."""

    # make a list of one or more components
    components: list[RFComponent] = [
        Amp(10, 2, OP1dB_dBm=0, desc="lna"),
        Loss(1, desc="bpf"),
        Mixer(3, 4, desc="mixer"),
    ]

    # make a list of one or more sections
    sections: list[ComponentChain] = [
        ComponentChain(desc="Example 10.2", components=components),
    ]

    # perform analysis with desired inputs
    analyze_sections(
        sections=sections,
        Pin_dBm=-82.8,
        BW_MHz=10,
        Tin=150,
        file="sgc_example.txt",
    )
