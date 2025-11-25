import os, sys
from copy import deepcopy
from io import TextIOBase
from typing import Sequence

from .noise_figure import cascade_G_F
from .component_chain import ComponentChain
from .util import undBm, sdB, sdBm, k, T0


def analyze_sections(
    sections: Sequence[ComponentChain], Pin_dBm: float, BW_MHz: float, **kwargs
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
    file = sys.stdout
    if "file" in kwargs:
        if isinstance(kwargs["file"], TextIOBase):
            file = kwargs["file"]
            if file.closed:
                raise ValueError("Cannot output to closed file")
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
            Ni = _Ni
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

    fprint(generate_inlined_text(sections, Gees, Effs))

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


def generate_inlined_text(
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
