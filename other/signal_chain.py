import numpy as np
from typing import Sequence

T0 = 290 # K
k = 1.380649e-23 # Boltzmann

class Stage:
    def __init__(self, components:Sequence['Component'], desc:str) -> None:
        self.components = list(components)
        self.desc = desc

        G_cas = np.cumprod([1] + [c.G for c in self.components])

        F_stage = np.array([c.F for c in self.components])
        F_cas = F_stage[:-1] / G_cas[:-1]

        self.G = G_cas[-1]
        self.F = np.sum(F_cas)
        self.Te = 
        self.in_comp:bool = any((c.in_comp for c in self.components))

    def __str__(self) -> str:
        ret:str = f'Stage "{self.desc}":\n'
        ret += f'  G = {self.G:5.1f}, {10*np.log10(self.G):6.2f} dB'
        ret += f'  F = {self.F:5.1f}, {10*np.log10(self.F):6.2f} dB'
        if self.in_comp:
            ret += f'  Has component in compression'

        return ret

    class Component:
        def __init__(self, gain_dB:float, NF_dB:float, VSWR:float, P1_dBm:float, Pin_dBm:float, desc:str) -> None:
            self.G:float = 10**np.log10(gain_dB/10)
            self.F:float = 10**np.log10(NF_dB/10)
            self.VSWR:float = VSWR
            self.P1_dBm:float = P1_dBm
            self.Pin_dBm:float = Pin_dBm
            self.desc:str = desc

            self.Pout_dBm:float = self.Pin_dBm + gain_dB
            self.in_comp:bool = False
            if self.Pout_dBm > self.P1_dBm:
                print(f'Component "{self.desc}" over power limit: {self.Pout_dBm:.1f}, {self.P1_dBm:.1f}')
                self.Pout_dBm = self.P1_dBm
                self.in_comp = True

