import re
import json
from typing import Self



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

def load_catalog() -> dict:
    with open("rf_catalog.json", "r") as f:
        catalog = json.load(f)
    return catalog

