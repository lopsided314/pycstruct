"""Signal chain computation example."""

from .components import *
from .component_chain import *
from .sgc_analyzer import analyze_sections


# from signal_chain import *


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
