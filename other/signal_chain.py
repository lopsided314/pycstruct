import re
import numpy as np
from typing import Sequence, Self
import json

T0 = 290 # K
k = 1.380649e-23 # Boltzmann

def Te(F:float) -> float:
    return T0*(F-1)

def mismatch_loss_dB(VSWR:float) -> float:
    refl_coeff = (VSWR-1)/(VSWR+1)
    return 10*np.log10(1 - refl_coeff**2)

class Component:
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

    components:list[Component] = []
    for stage in stages:
        components += stage.components

    Gtot, Ftot = cascade_G_F(components)
    Teff = Te(Ftot)

    No_norm = k*Teff

    print(f'Totals:')
    print(f'  G    = {Gtot:6.1f} / {10*np.log10(Gtot):6.2f} dB')
    print(f'  F    = {Ftot:6.1f} / {10*np.log10(Ftot):6.2f} dB')
    print(f'  Teff = {Teff:6.0f} K')
    print(f'  Pout = {Pout_dBm:6.2f} dBm')
    print(f'  No   = {10*np.log10(No_norm) + 30:6.1f} dBm/Hz')


class PartFilter:
    # needs to already be narrowed down to the component list level
    def __init__(self, catalog:dict):
        self._catalog = catalog
        self._name_re = '.*'
        self._freq = ((0, 100), (0, 100))
        self._gain = (-999, 999)
        self._VSWR = 999
        self._Pin = -999
        self._NF = (0, 999)

    def _update_catalog(self) -> None:
        catalog:dict = {}
        for name, spec in self._catalog.items():

            name_ok:bool = re.fullmatch(self._name_re, name) is not None

            freq_ok:bool = True
            if 'Freq Start GHz' in spec:
                freq_ok &= self._freq[0][0] <= spec['Freq Start GHz'] and self._freq[0][1] >= spec['Freq Start GHz']
            if 'Freq Stop GHz' in spec:
                freq_ok &= self._freq[1][0] <= spec['Freq Stop GHz'] and self._freq[1][1] >= spec['Freq Stop GHz']

            gain_ok:bool = True
            if 'Gain dB' in spec:
                gain_ok = (self._gain[0] <= spec['Gain dB'] and self._gain[1] >= spec['Gain dB'])
            elif 'IL dB'in spec:
                gain_ok = self._gain[0] >= spec['IL dB']

            VSWR_ok:bool = True
            if 'VSWR' in spec:
                VSWR_ok = self._VSWR >= spec['VSWR']

            Pin_ok:bool = True
            if 'P1 dB' in spec:
                Pin_ok = self._Pin <= spec['P1 dB']
            elif 'Pin Max dBm' in spec:
                Pin_ok = self._Pin <= spec['Pin Max dBm']

            # print(name_ok, freq_ok, gain_ok, VSWR_ok, Pin_ok
            if name_ok and freq_ok and gain_ok and VSWR_ok and Pin_ok:
                catalog[name] = spec

        self._catalog = catalog

    def name_re(self, name_match:str) -> Self:
        self._name_re = '.*'+name_match+'.*'
        self._name_re = self._name_re.replace('.*.*', '.*')
        self._update_catalog()
        return self

    def freq(self, low:tuple[float, float], high:tuple[float, float]) -> Self:
        self._freq = (low, high)
        self._update_catalog()
        return self

    def gain(self, lim:tuple[float, float]) -> Self:
        self._gain = lim
        self._update_catalog()
        return self

    def insertion_loss(self, max:float) -> Self:
        self._gain = (-max, 999)
        self._update_catalog()
        return self

    def VSWR(self, lim:float) -> Self:
        self._VSWR = lim
        self._update_catalog()
        return self
    
    def Pin(self, lim:float) -> Self:
        self._Pin = lim
        self._update_catalog()
        return self
    
    def __str__(self) -> str:
        return json.dumps(self._catalog, indent=4)


def load_catalog() -> tuple[dict, dict[str, Component]]:
    with open("rf_catalog.json", "r") as f:
        catalog = json.load(f)

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
    catalog, parts = load_catalog()

    print(PartFilter(catalog['Amplifiers']).freq((7, 9), (10, 100)).gain((20,30)).VSWR(1.5).Pin(10).name_re('812'))
    return


    LNA = parts['CA812-281B']
    ATTN1 = Loss(3.4, 1.8, 30)

    stage = Stage(
        (PAD[3], LNA, ATTN1),
        desc='test'
    )
    analyze_stages((stage, stage), 0)




if __name__ == "__main__":
    test()
