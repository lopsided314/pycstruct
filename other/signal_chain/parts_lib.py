import json
from signal_chain import RFComponent, Amp, Loss, Mixer
from signal_chain import return_loss_to_VSWR


def get_VSWR_param(spec: dict[str, float]) -> float:
    """Find if any of the parameters can be turned into a VSWR."""
    match spec:
        case {"VSWR": VSWR} | {"Input VSWR": VSWR}:
            return VSWR
        case {"Input Return Loss dB": rl} | {"Return Loss dB": rl}:
            return return_loss_to_VSWR(rl)
        case _:
            return 1


def parse_amp(name: str, spec: dict[str, float]) -> dict[str, RFComponent]:
    """Take amplifier parameters from the catalog json and turn it into a Component object."""
    return {
        name: Amp(
            gain_dB=spec["Gain dB"],
            NF_dB=spec["Noise Figure dB"],
            VSWR=get_VSWR_param(spec),
            OP1dB_dBm=spec.get("OP1dB dBm", 999),
            desc=name,
        )
    }


def parse_mixer(name: str, spec: dict[str, float]) -> dict[str, RFComponent]:
    """Take mixer parameters from the catalog json and turn it into a Component object."""
    return {
        name: Mixer(
            loss_dB=spec["Conversion Loss dB"],
            NF_dB=spec["Noise Figure dB"],
            VSWR=get_VSWR_param(spec),
            IP1dB_dBm=spec.get("IP1dB dBm", 999),
            desc=name,
        )
    }


def parse_lossy(name: str, spec: dict[str, float]) -> dict[str, RFComponent]:
    """Take parameters of lossy component from the catalog json and turn it into a Component object."""
    return {
        name: Loss(
            loss_dB=spec["Insertion Loss dB"],
            VSWR=get_VSWR_param(spec),
            Pin_warn_dBm=spec.get("Pin Max dBm", 999),
            desc=name,
        )
    }


def read_catalog(filename: str) -> dict[str, RFComponent]:
    with open(filename, "r") as f:
        catalog = json.load(f)

    components: dict[str, RFComponent] = {}
    for category, parts_list in catalog.items():
        if category == "Amps":
            for name, spec in parts_list.items():
                components.update(parse_amp(name, spec))
        elif category == "Mixers":
            for name, spec in parts_list.items():
                components.update(parse_mixer(name, spec))
        else:  # pretend everything else is default lossy component
            for name, spec in parts_list.items():
                components.update(parse_lossy(name, spec))
    return components


if __name__ == "__main__":
    for name, component in read_catalog("./parts_catalog.json").items():
        print(name, component.VSWR)
