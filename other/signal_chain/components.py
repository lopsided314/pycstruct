from .noise_figure import NoiseFigureStage
from .util import *


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
