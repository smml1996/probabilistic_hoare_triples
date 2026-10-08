from enum import Enum

class ExpType(Enum):
    ipma2 = "ipma2"
    ghz = "ghz"
    reset = "reset"
    lphase = "lphase"
    cxh = "cxh"


class MethodType(Enum):
    linear = "linear"
    singular = "singular"
    any = "any"

def _tex_name_raw(exp_type, method_type: MethodType):
    if exp_type == ExpType.ipma2:
        temp = "IPMAP"
    elif exp_type == ExpType.ghz:
        temp = "GHZ"
    elif exp_type == ExpType.lphase:
        temp = "LPhase"
    elif exp_type == ExpType.cxh:
        temp = "CXH"
    elif exp_type == ExpType.reset:
        temp = "Reset"
    else:
        raise Exception("Unknown experiment type: ", exp_type)

    if method_type == MethodType.linear:
        temp += "Lin"
    else:
        assert method_type == MethodType.singular
    return f"{temp}"

def _tex_name(exp_type: ExpType, method_type: MethodType):
    return "\\" + f"{_tex_name_raw(exp_type, method_type)}"

def _tex_instruction_set(exp_type: ExpType):
    return _tex_name(exp_type, MethodType.singular) + "Ins"

def _tex_name_abbr(exp_type: ExpType, method_type: MethodType):
    return _tex_name(exp_type, MethodType.singular) + "Abbr"

def _tex_initial_states(exp_type: ExpType):
    if exp_type == ExpType.ipma2 or exp_type == ExpType.cxh:
        return "$\{\\ket{\\bstateS^{\pm}},~\\ket{\\bstateSX^{\pm}}\}$"
    if exp_type == ExpType.ghz:
        return "$\{\\ket{000}\}$"
    if exp_type == ExpType.reset:
        return "$\{\\ket{0},~\\ket{1}\}$"
    if exp_type == ExpType.lphase:
        return "$\{\\ket{{+}0},~\\ket{{+}1}\}$"

    assert False

def _tex_target_states(exp_type: ExpType):
    if exp_type == ExpType.ipma2 or exp_type == ExpType.cxh:
        return "$\{\\ket{\\bstateS^{\pm}}\}$"
    if exp_type == ExpType.ghz:
        return "$\{\\ket{\\text{GHZ}}\}$"
    if exp_type == ExpType.reset:
        return "$\{\\ket{0}\}$"
    if exp_type == ExpType.lphase:
        return "$\{\\ket{{-}0},~-\\ket{{-}1}\}$"

    assert False