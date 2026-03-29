from math import log10

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


#
# initialization values for dB units that don't
# cause arithmetic errors when converting back
# to printable values
#
DB_INIT = undB(-999.99)
DBM_INIT = undBm(-999.99)
