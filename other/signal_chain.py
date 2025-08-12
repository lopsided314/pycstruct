import numpy as np
from typing import Sequence

T0 = 290 # K
k = 1.380649e-23 # Boltzmann

def Te(F:float) -> float:
    return T0*(F-1)

def mismatch_loss_dB(VSWR:float) -> float:
    refl_coeff = (VSWR-1)/(VSWR+1)
    return 10*np.log10(1 - refl_coeff**2)

class _Component:
    def __init__(self, gain_dB:float, NF_dB:float, VSWR:float, Pin_limit_dBm:float, desc:str) -> None:
        self.G:float = 10**(gain_dB/10)
        self.F:float = 10**(NF_dB/10)
        self.VSWR:float = VSWR
        self.Pin_limit_dBm:float = Pin_limit_dBm
        self.desc:str = desc
        self.in_comp:bool = False
        self.Pout_dBm:float = -999 

    def set_Pin(self, Pin_dBm:float) -> None:
        self.Pout_dBm:float = Pin_dBm + 10*np.log10(self.G)
        if Pin_dBm > self.Pin_limit_dBm:
            if self.G > 1:
                print(f'"{self.desc}" output too high: {self.Pout_dBm:.1f}, P1dB = {self.Pin_limit_dBm+10*np.log10(self.G):.1f}')
            else:
                print(f'"{self.desc}" input too high: {Pin_dBm:.1f}, {self.Pin_limit_dBm:.1f}')
            self.in_comp = True

class Amp(_Component):
    def  __init__(self, gain_dB: float, NF_dB: float, VSWR: float, P1_dBm: float, desc: str = "") -> None:
        # need to check the book for this one
        # mmloss = mismatch_loss_dB(VSWR)
        super().__init__(
            gain_dB=gain_dB, 
            NF_dB=NF_dB, 
            VSWR=VSWR, 
            Pin_limit_dBm=P1_dBm-gain_dB, 
            desc=desc
        )

class Loss(_Component):
    def __init__(self, loss_dB:float, VSWR:float=1, P1_dBm:float = 999, desc="") -> None:
        # loss_dB += mismatch_loss_dB(VSWR)
        super().__init__(
            gain_dB=-loss_dB, 
            NF_dB=loss_dB, 
            VSWR=VSWR, 
            Pin_limit_dBm=P1_dBm, 
            desc=desc
        )

class Mixer(_Component):
    def __init__(self, loss_dB:float, NF_dB:float, VSWR:float, P1_dBm:float = 999, desc=''):
        # loss_dB += mismatch_loss_dB(VSWR)
        super().__init__(
            gain_dB=-loss_dB, 
            NF_dB=NF_dB, 
            VSWR=VSWR, 
            Pin_limit_dBm=P1_dBm, 
            desc=desc
        )

class Coax(_Component):
    def __init__(self, length:float, dB_per:float, desc:str = '') -> None:
        loss_dB = length * dB_per
        super().__init__(
            gain_dB=-loss_dB, 
            NF_dB=loss_dB, 
            VSWR=1, 
            Pin_limit_dBm=9999, 
            desc=desc
        )


def cascade_G_F(components:list[_Component]) -> tuple[float, float]:
    # G1, G1*G2, G1*G2*...*Gn
    G_cas = np.cumulative_prod([c.G for c in components])
    Gtot:float = G_cas[-1]

    # F1 + (F2-1)/G1 + (F3-1)/(G1*G2) + ...
    F = [c.F for c in components]
    Ftot:float = F[0]
    for g, f in zip(G_cas[:-1], F[1:]):
        Ftot += (f-1)/g

    return Gtot, Ftot


class Stage:
    def __init__(self, components:Sequence[_Component], desc:str) -> None:
        self.components:list[_Component] = list(components)
        self.desc:str = desc

        self.G, self.F = cascade_G_F(self.components)
        self.Te = Te(self.F)
        
    def set_Pin(self, Pin_dBm:float) -> None:
        self.Pin_dBm = Pin_dBm

        self.components[0].set_Pin(Pin_dBm=self.Pin_dBm)
        for prev, next in zip(self.components[:-1], self.components[1:]):
            next.set_Pin(prev.Pout_dBm)

        self.Pout_dBm:float = self.components[-1].Pout_dBm
        self.in_comp:bool = any((c.in_comp for c in self.components))

    def __str__(self) -> str:
        ret:str = f'Stage "{self.desc}":\n'
        ret += f'  G    = {self.G:5.2f}, {10*np.log10(self.G):6.2f} dB\n'
        ret += f'  F    = {self.F:5.2f}, {10*np.log10(self.F):6.2f} dB\n'
        ret += f'  Pout = {self.Pout_dBm:5.2f} dBm\n'
        if self.in_comp:
            ret += f'  Has component power issue\n'

        return ret


def analyze_stages(stages:Sequence[Stage], Pin_dBm:float) -> None:
    stages[0].set_Pin(Pin_dBm=Pin_dBm)
    for prev, next in zip(stages[:-1], stages[1:]):
        next.set_Pin(prev.Pout_dBm)

    for stage in stages:
        print(stage)
    print()

    Pout_dBm = stages[-1].Pout_dBm

    components:list[_Component] = []
    for stage in stages:
        components += stage.components

    Gtot, Ftot = cascade_G_F(components)
    Teff = Te(Ftot)

    No_norm = k*Teff

    print(f'Totals:')
    print(f'  G    = {Gtot:6.1f}, {10*np.log10(Gtot):6.2f} dB')
    print(f'  F    = {Ftot:6.1f}, {10*np.log10(Ftot):6.2f} dB')
    print(f'  Teff = {Teff:6.0f} K')
    print(f'  Pout = {Pout_dBm:6.2f} dBm')
    print(f'  Noise Power = {10*np.log10()} dBm/Hz')


def test() -> None:
    PAD = [Loss(i) for i in range(20)]

    LNA = Amp(24, 3, 1.5, 17, 'CA812-281')
    ATTN1 = Loss(3.4, 1.8, 30)

    stage = Stage(
        (PAD[3], LNA, ATTN1),
        desc='test'
    )
    analyze_stages((stage, stage), 0)

if __name__ == "__main__":
    test()
