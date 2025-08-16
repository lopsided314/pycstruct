import numpy as np
from typing import Sequence

from catalog_filter import PartFilter, load_catalog

T0 = 290 # K
k = 1.380649e-23 # Boltzmann

def dB(x):
    return 10*np.log10(x)

def undB(x):
    return 10**(x/10)

def Te(F:float) -> float:
    return T0*(F-1)

def mismatch_loss_dB(VSWR:float) -> float:
    refl_coeff = (VSWR-1)/(VSWR+1)
    return dB(1 - refl_coeff**2)

class Component:
    def __init__(self, gain_dB:float, NF_dB:float, VSWR:float, Pin_limit_dBm:float, desc:str) -> None:
        self.G:float = undB(gain_dB)
        self.F:float = undB(NF_dB)
        self.VSWR:float = VSWR
        self.Pin_limit_dBm:float = Pin_limit_dBm
        self.desc:str = desc
        self.in_comp:bool = False
        self.Pout_dBm:float = -999 

    def set_Pin(self, Pin_dBm:float) -> None:
        self.Pout_dBm:float = Pin_dBm + dB(self.G)
        if Pin_dBm > self.Pin_limit_dBm:
            if self.G > 1:
                print(f'"{self.desc}" output too high: {self.Pout_dBm:.1f}, P1dB = {self.Pin_limit_dBm+dB(self.G):.1f}')
            else:
                print(f'"{self.desc}" input too high: {Pin_dBm:.1f}, {self.Pin_limit_dBm:.1f}')
            self.in_comp = True

class Amp(Component):
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

class Loss(Component):
    def __init__(self, loss_dB:float, VSWR:float=1, Pin_max:float = 999, desc="") -> None:
        # loss_dB += mismatch_loss_dB(VSWR)
        super().__init__(
            gain_dB=-loss_dB, 
            NF_dB=loss_dB, 
            VSWR=VSWR, 
            Pin_limit_dBm=Pin_max, 
            desc=desc
        )
PAD = [Loss(i) for i in range(20)]

class Mixer(Component):
    def __init__(self, loss_dB:float, NF_dB:float, VSWR:float, P1_dBm:float = 999, desc=''):
        # loss_dB += mismatch_loss_dB(VSWR)
        super().__init__(
            gain_dB=-loss_dB, 
            NF_dB=NF_dB, 
            VSWR=VSWR, 
            Pin_limit_dBm=P1_dBm, 
            desc=desc
        )

class Coax(Component):
    def __init__(self, length:float, dB_per:float, desc:str = '') -> None:
        loss_dB = length * dB_per
        super().__init__(
            gain_dB=-loss_dB, 
            NF_dB=loss_dB, 
            VSWR=1, 
            Pin_limit_dBm=9999, 
            desc=desc
        )


def cascade_G_F(components:list[Component]) -> tuple[float, float]:
    # G1, G1*G2, G1*G2*...*Gn
    G_cas = np.cumprod([c.G for c in components])
    Gtot:float = G_cas[-1]

    # F1 + (F2-1)/G1 + (F3-1)/(G1*G2) + ...
    F:list[float] = [c.F for c in components]
    Ftot:float = F[0]
    for g, f in zip(G_cas[:-1], F[1:]):
        Ftot += (f-1)/g

    return Gtot, Ftot


class Stage:
    def __init__(self, components:Sequence[Component], desc:str) -> None:
        self.components:list[Component] = list(components)
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
        ret += f'  G    = {self.G:5.2f}, {dB(self.G):6.2f} dB\n'
        ret += f'  F    = {self.F:5.2f}, {dB(self.F):6.2f} dB\n'
        ret += f'  Pout = {self.Pout_dBm:5.2f} dBm\n'
        if self.in_comp:
            ret += f'  Has component power issue\n'

        return ret

import copy
def analyze_stages(stages:Sequence[Stage], Pin_dBm:float, BW_MHz:float, **kwargs) -> None:
    stages[0].set_Pin(Pin_dBm=Pin_dBm)
    for prev, next in zip(stages[:-1], stages[1:]):
        next.set_Pin(prev.Pout_dBm)

    for stage in stages:
        print(stage)
    print()

    Pout_dBm = stages[-1].Pout_dBm

    components:list[Component] = []
    for stage in stages:
        components += stage.components

    Gtot, Ftot = cascade_G_F(components)
    Teff = Te(Ftot)

    print(f'Totals:')
    print(f'  G    = {Gtot:6.1f} / {dB(Gtot):6.2f} dB')
    print(f'  F    = {Ftot:6.1f} / {dB(Ftot):6.2f} dB')
    print(f'  Teff = {Teff:6.0f} K')
    print(f'  Pout = {Pout_dBm:6.2f} dBm')
    print(f'  No+  = {dB(k*Teff) + 30:6.1f} dBm/Hz ({dB(k*Teff*BW_MHz*1e6) + 30:6.1f} dBm/{BW_MHz:.0f} MHz)')
    print()
    
    Tin:float = -1
    if 'Tin' in kwargs:
        Tin = float(kwargs['Tin'])
    elif 'Ni_dBm' in kwargs:
        Tin = 10**(float(kwargs['Ni_dBm']-30)/10) / (k*BW_MHz*1e6)
    elif 'Ni_dBm_Hz' in kwargs:
        Tin = 10**(float(kwargs['Ni_dBm_Hz']-30)/10)/k
    elif 'SNR_in' in kwargs:
        Ni_dBm = Pin_dBm - float(kwargs['SNR_in'])
        Tin = 10**((Ni_dBm-30)/10) / (k*BW_MHz*1e6)

    if Tin > 0:
        To = Teff + Gtot*Tin
        print(f'  Tin  = {Tin:10.0f} K')
        print(f'  Ni   = {dB(k*Tin) + 30:6.2f} dBm/Hz ({dB(k*Tin*BW_MHz*1e6) + 30:6.1f} dBm/{BW_MHz:.0f} MHz)')
        print(f'  No   = {dB(k*To) + 30:6.2f} dBm/Hz ({dB(k*To*BW_MHz*1e6) + 30:6.1f} dBm/{BW_MHz:.0f} MHz)')
        print(f'  SNR  = {Pout_dBm - (dB(k*To*BW_MHz*1e6) + 30):.0f} dB/{BW_MHz:.0f} MHz')

#--------------------------------------------------------------------------------
#--------------------------------------------------------------------------------

def catalog_components() -> tuple[dict, dict[str, Component]]:
    catalog = load_catalog()

    components:dict[str, Component] = {}
    for name, spec in catalog.get('Amplifiers', {}).items():
        spec.update(spec.pop('Specs'))
        try:
            components[name] = Amp(
                gain_dB = spec['Gain dB'],
                NF_dB = spec['Noise Figure'],
                VSWR = spec['VSWR'],
                P1_dBm = spec['P1 dB'],
                desc = name
            )
        except KeyError as e:
            print(f'Amplifier {name} {e}')
            continue

    for i in catalog.get('Dividers', {}).values():
        for name, spec in i.items():
            spec.update(spec.pop('Specs'))
            try:
                components[name] = Loss(
                    loss_dB = spec['IL dB'],
                    VSWR = spec['VSWR'],
                    Pin_max = spec['Pin Max dBm'],
                    desc = name,
                )
            except KeyError as e:
                print(f'Divider {name} {e}')
                continue


    return catalog, components

def test() -> None:
    catalog, parts = catalog_components()

    LNA = parts['CA812-281B']
    ATTN1 = Loss(3.4, 1.8, 30)

    stage = Stage(
        (PAD[3], LNA, ATTN1),
        desc='test'
    )
    stage2 = copy.deepcopy(stage)
    analyze_stages((stage, stage2), 0, 10, SNR_in = 50)




if __name__ == "__main__":
    test()
