import sys, os
import re
import glob
from dataclasses import dataclass

"""TODO: module docstring"""

DIR = os.path.dirname(os.path.abspath(__file__))
INSTANCE_FILE = DIR + "/pycstruct_instances.txt"
MACRO_FILE = DIR + "/pycstruct_macros.txt"

OVERWRITE_WARNING = """
/*******************************************************************************\n
*\n
*\n
* EVERYTHING IN THIS FILE WILL BE OVERRITTEN\n
*\n
*\n
*******************************************************************************/\n
\n\n
"""

DEBUG_PRINT = False


def deformat_source(src_text: str) -> str:
    """Normalize user code into parsable format."""

    lines = src_text.replace("\\", "").strip().split("\n")
    # remove pre-processor bullshit
    src_text = "\n".join([line for line in lines if line and line[0] != "#"])

    # get rid of the comments manually since
    # the regex is doing weird shit
    i: int = src_text.find("/*")
    while i >= 0:
        end = src_text.find("*/", i)
        if end == -1:
            raise ValueError("Comment wasn't closed")

        src_text = src_text[:i] + src_text[end + 2 :]
        i = src_text.find("/*")

    src_text += "\n"  # for // comments as last line of file
    i = src_text.find("//")
    while i >= 0:
        src_text = src_text[:i] + src_text[src_text.find("\n", i) :]
        i = src_text.find("//")

    # remove default initialized values
    i = src_text.find("=")
    while i >= 0:
        src_text = src_text[:i] + src_text[src_text.find(";", i) :]
        i = src_text.find("=")

    # normalize whitespace around all important characters
    src_text = re.sub(r"\s*{\s*", " { ", src_text, flags=re.DOTALL)
    src_text = re.sub(r"\s*}\s*", " } ", src_text, flags=re.DOTALL)
    src_text = re.sub(r"\s*\[\s*", " [ ", src_text, flags=re.DOTALL)
    src_text = re.sub(r"\s*\]\s*", " ] ", src_text, flags=re.DOTALL)
    src_text = re.sub(r"\s*;\s*", " ; ", src_text, flags=re.DOTALL)
    src_text = re.sub(r"\s*\*\s*", " * ", src_text, flags=re.DOTALL)
    src_text = re.sub(r"\s*:\s*", ":", src_text, flags=re.DOTALL)
    src_text = re.sub(r"\s*,\s*", " , ", src_text, flags=re.DOTALL)
    src_text = src_text.replace("struct{", "struct {")
    src_text = src_text.replace("union{", "union {")
    src_text = re.sub(r"\s*struct\s+", " struct ", src_text, flags=re.DOTALL)
    src_text = re.sub(r"\s*union\s+", " union ", src_text, flags=re.DOTALL)
    src_text = re.sub(r"\s+", " ", src_text, flags=re.DOTALL)
    return src_text


def reformat_struct(struct_def: str) -> str:
    """Put the struct back into user readable format."""

    struct_def = struct_def.strip()
    struct_def = re.sub(r"\s*struct\s*", "struct ", struct_def, flags=re.DOTALL)
    struct_def = re.sub(r"\s*union\s*", "union ", struct_def, flags=re.DOTALL)
    struct_def = re.sub(r"\s*{\s*", "\n{\n", struct_def, flags=re.DOTALL)
    struct_def = re.sub(r"\s*}\s*", "\n} ", struct_def, flags=re.DOTALL)
    struct_def = re.sub(r"\s*;\s*", ";\n", struct_def, flags=re.DOTALL)
    struct_def = re.sub(r"\s*,\s*", ", ", struct_def, flags=re.DOTALL)
    struct_def = re.sub(r"\s*\*\s*", " *", struct_def, flags=re.DOTALL)
    brace_level: int = 0
    lines: list[str] = []
    for line in struct_def.split("\n"):
        if "}" in line:
            brace_level -= 1
        lines.append("    " * brace_level + line)
        if "{" in line:
            brace_level += 1
    return "\\n".join(lines)


cli_types: dict[str, tuple[str, str]] = {
    "i8": ("4d", "stol"),
    "u8": ("02X", "stoul_0x"),
    "i16": ("6d", "stol"),
    "u16": ("04X", "stoul_0x"),
    "i32": ("11d", "stol"),
    "u32": ("08X", "stoul_0x"),
    "i64": ("20ld", "stol"),
    "li64": ("20lld", "stol"),
    "u64": ("016lX", "stoul_0x"),
    "lu64": ("016llX", "stoul_0x"),
    "f32": ("11.4e", "stod"),
    "f64": ("11.4e", "stod"),
}
ctype_fmts: dict[str, tuple[str, str]] = {
    "bool": ("%1X", "stoul_0x"),
    "int8_t": cli_types["i8"],
    "char": cli_types["i8"],
    "signed char": cli_types["i8"],
    "uint8_t": cli_types["u8"],
    "unsigned char": cli_types["u8"],
    "int16_t": cli_types["i16"],
    "short": cli_types["i16"],
    "short int": cli_types["i16"],
    "signed short": cli_types["i16"],
    "signed short int": cli_types["i16"],
    "uint16_t": cli_types["u16"],
    "unsigned short": cli_types["u16"],
    "unsigned short int": cli_types["u16"],
    "int32_t": cli_types["i32"],
    "int": cli_types["i32"],
    "signed": cli_types["i32"],
    "signed int": cli_types["i32"],
    "uint32_t": cli_types["u32"],
    "unsigned int": cli_types["u32"],
    "int64_t": cli_types["i64"],
    "long": cli_types["i64"],
    "long int": cli_types["i64"],
    "signed long": cli_types["i64"],
    "signed long int": cli_types["i64"],
    "ssize_t": cli_types["i64"],
    "long long": cli_types["li64"],
    "long long int": cli_types["li64"],
    "signed long long": cli_types["li64"],
    "signed long long int": cli_types["li64"],
    "uint64_t": cli_types["u64"],
    "unsigned long": cli_types["u64"],
    "unsigned long int": cli_types["u64"],
    "unsigned long long": cli_types["lu64"],
    "unsigned long long int": cli_types["lu64"],
    "size_t": cli_types["u64"],
    "float": cli_types["f32"],
    "double": cli_types["f64"],
}


#
# Regular variables and arrays
#
class CStdVar:
    def __init__(self, var_def: str) -> None:
        """Parse the type and name of a c-style variable or array."""
        var_def = var_def.strip(" ;")
        self.raw = var_def

        is_arr: bool = "[" in var_def and "]" in var_def

        def_split = [s.strip() for s in var_def.split(" ") if (s != "[" and s != "]")]

        if is_arr and len(def_split) < 3:
            raise ValueError(f'Invalid array declaration: "{var_def}"')

        elif not is_arr and len(def_split) < 2:
            raise ValueError(f'Invalid member declaration: "{var_def}"')

        # The type can be any number of words but the size and name are only 1,
        # e.g. unsigned long long int
        self.ctype: str = ""
        self.size: int = 0
        if is_arr:
            size_arg = def_split.pop()
            base = 16 if "0x" in size_arg else 10
            try:
                self.size: int = int(size_arg, base)
            except ValueError:
                print(
                    f"Pycstruct: [Skipping] Array size not integer literal: {var_def}"
                )
                return

        self.name: str = def_split.pop()
        self.ctype = " ".join(def_split)

    def macro(self, parent: str, name_base: str) -> str:
        """Generate the pycstruct macro call for this variable."""

        if self.ctype not in ctype_fmts:
            return ""

        printf, stonum = ctype_fmts[self.ctype]
        if self.size == 0:
            return f'REGISTER_VAR({parent}, {name_base}{self.name}, {self.ctype}, "{printf}", {stonum});'

        elif self.ctype == "char":
            return f"REGISTER_CHAR_ARR({parent}, {name_base}{self.name})"

        else:
            return f'REGISTER_ARR({parent}, {name_base}{self.name}, {self.size}, {self.ctype}, "{printf}", {stonum});'


#
# Bitfields
#
class CBitfield:
    def __init__(self, bitfield_def: str) -> None:
        """Parse the type and name of a bitfield definition."""
        self.raw: str = bitfield_def.strip()
        self.ctype: str = ""
        self.fields: list[tuple[str, str]] = []

        for token in self.raw.split(" "):
            if ";" in token or "," in token:
                continue

            if ":" not in token:
                self.ctype += token + " "
                continue

            try:
                name, size = token.split(":")
                if len(name) > 0:
                    self.fields.append((name.strip(), size))
            except ValueError:
                raise ValueError(f'Invalid bitfield declaration: "{bitfield_def}"')

        self.ctype = self.ctype.strip()

    def macros(self, parent: str, name_base: str) -> list[str]:
        """Generate the pycstruct macro call for this bitfield."""

        if self.ctype not in ctype_fmts:
            return []

        macros: list[str] = []
        printf, stonum = ctype_fmts[self.ctype]

        # The unsigned prints have leading zeros that don't accurately
        # represent a bitfield size, e.g.
        #    unsigned long u1 : 5, :3, u2 : 5, :51;
        # probably shouldn't print
        #   u1 = 000000000000001f,
        #   u2 = 000000000000001f,
        # although this is just my opinion.
        if printf[0] == "0":
            printf = printf[1:]

        for field_name, _ in self.fields:
            macros.append(
                f'REGISTER_BITFIELD({parent}, {name_base}{field_name}, {self.ctype}, "{printf}", {stonum});'
            )

        return macros


class ObjectFrame:
    """
    This is a name I made up that basically just includes structs and
    unions. In theory this includes classes as well, but... I wan't no part
    of that.
    """

    def __init__(self, body_def: str, parents: list) -> None:
        """Parse the definition of an 'Object Frame'.

        Get the names and types of all members of the object. This call is
        recursive so structs declared within structs or unions declared within
        structs is all fair game.
        """

        self.raw_full: str = body_def.strip()
        self.parents: list[ObjectFrame] = parents
        self.src_file: str = ""
        self.src_file_mtime: int = 0

        self.instance_name: str = self.raw_full[
            self.raw_full.rfind("}") + 1 : self.raw_full.rfind(";")
        ].strip()  # struct S {...} _name_;

        # For a nameless struct, default to _Struct for typename
        self.frame_type, self.typename = body_def.split(" ")[:2]
        if self.typename == "{":
            self.typename = "_" + self.frame_type.capitalize()

        # isolate the contents of the struct
        self.raw_body: str = self.raw_full[
            self.raw_full.find("{") + 1 : self.raw_full.rfind("}")
        ]

        self.frames: list["ObjectFrame"] = []
        self.bitfields: list[CBitfield] = []
        self.vars: list[CStdVar] = []

        parent_types: list[str] = [p.typename for p in self.parents]
        parent_names: list[str] = [
            p.instance_name for p in self.parents if p.instance_name
        ]

        self.combo_type: str = "::".join(parent_types + [self.typename])
        self.combo_name: str = ".".join(parent_names + [self.instance_name])
        if self.combo_name:
            self.combo_name += "."

        self._get_members()

        if DEBUG_PRINT:
            self.print()

    def reg_macros(self, base_name) -> list[str]:
        """Generate all the c++ macro calls to register all the members of
        this struct. This call is recursive as well."""
        macros: list[str] = []

        for frame in self.frames:
            macros += frame.reg_macros(base_name)

        for v in self.vars:
            mac = v.macro(base_name, self.combo_name)
            if mac:
                macros.append(mac)

        for b in self.bitfields:
            macros += b.macros(base_name, self.combo_name)

        return macros

    def _get_members(self) -> None:
        """Parse the body of this struct to get lists of all its members and
        struct sub-types.

        Recursively parses all sub-struct frames.
        """

        remainder = self.raw_body
        only_members = self.raw_body

        while remainder:
            frame, remainder = pop_object_frame(remainder)

            if not frame:
                continue

            self.frames.append(ObjectFrame(frame, self.parents + [self]))
            only_members = only_members.replace(frame, "")

        for member in only_members.split(";"):
            if ":" in member and not "::" in member:
                self.bitfields.append(CBitfield(member))
            elif len(member.strip()) != 0:
                self.vars.append(CStdVar(member))

    def print(self) -> None:
        """Print out all the things it found about this struct."""
        header = f"Pycstruct: struct {self.combo_type} {self.combo_name}... "
        for b in self.bitfields:
            if not b.ctype:
                continue

            for name, width in b.fields:
                print(f"{header} {b.ctype} {name}:{width};")

        for v in self.vars:
            if not v.ctype:
                continue

            var_info = f"{header} {v.ctype} {v.name}"

            if v.size == 0:
                print(f"{var_info};")
            else:
                print(f"{var_info}[{v.size}];")


def pop_object_frame(text: str) -> tuple[str, str]:
    """Return the first full object frame definition and the remainder of the
    text."""

    END: int = 1 << 31
    pos_end = lambda i: i if i >= 0 else END

    first_frame: int = min(
        (
            pos_end(text.find("struct ")),
            pos_end(text.find("union ")),
        )
    )

    if first_frame == END:
        return "", ""

    text = text[first_frame:]

    first_open: int = pos_end(text.find("{"))
    first_close: int = pos_end(text.find("}"))
    first_semi: int = pos_end(text.find(";"))

    # Characters that signify features that we cannot deal with.
    # functions, templates, constructors, inheritiance, ...
    #
    # std::unique_ptr<struct S>{new struct S}
    bad_chars: str = "()<>"
    first_bad_char: int = min(pos_end(text.find(c)) for c in bad_chars)

    if min(first_bad_char, first_semi, first_close) < first_open:
        return "", text[1:]

    # We need to track how many nested frames we have entered to make sure we
    # find the matchcing pair of the opening brace instead of some member
    # struct.
    indent_level: int = 0
    for i, c in enumerate(text):
        # wait for the start of the first body to count indents
        if indent_level == 0:
            if c == "{":
                indent_level = 1
            continue

        if c == "{":
            indent_level += 1
        elif c == "}":
            indent_level -= 1

        if indent_level == 0:
            # we found the matching pair for the starting brace
            def_end: int = text.find(";", i) + 1
            frame = text[:def_end].strip()
            remainder = text[def_end + 1 :].strip()

            if any(c in frame for c in bad_chars) or frame == "\\":
                return "", remainder

            s_or_u = frame.split(" ")[0]

            if s_or_u != "struct" and s_or_u != "union":
                raise ValueError(
                    f"Invalid syntax: frame is neither struct nor union: {frame}"
                )

            return frame, remainder

    raise ValueError(f"Invalid syntax: failed to find closing brace:", text)


@dataclass(frozen=True, eq=True)
class StructRegisterRequest:
    src_filename: str
    typename: str
    instance_name: str
    pragma_pack: str = ""


def find_requests(text: str, filename: str) -> set[StructRegisterRequest]:
    """
    Go through the file and find all empty macro calls
    that indicate to this program which structs we need to handle
    """
    registers: set[StructRegisterRequest] = set()

    text = text.replace("#define REGISTER_STRUCT", "")  # ignore the macro definition
    i: int = text.find("REGISTER_STRUCT")

    bad_chars: str = "()<>{}-+|/;!@#$%^&*~`.?\\"
    while i != -1:

        macro_args = text[text.find("(", i) + 1 : text.find(")", i)]

        if any(c in macro_args for c in bad_chars):
            ValueError(f"Invalid struct registration request: {macro_args}")

        args = [s.strip() for s in macro_args.split(",") if s.strip()]
        registers.add(StructRegisterRequest(filename, *args))

        i = text.find("REGISTER_STRUCT", i + 1)

    return registers


def find_top_struct_defs(text: str, filename: str) -> dict[str, ObjectFrame]:
    """Find all the top level struct definitions in the file."""

    frames: list[ObjectFrame] = []

    remainder = deformat_source(text)

    while remainder.find("struct ") != -1:
        frame, remainder = pop_object_frame(remainder)
        if not frame:
            continue

        obj = ObjectFrame(frame, [])
        obj.src_file = filename
        obj.src_file_mtime = int(os.path.getmtime(filename))
        frames.append(obj)

    return {s.typename: s for s in frames if s.frame_type == "struct"}


def get_filename_list(includes: list[str]) -> set[str]:
    """
    Based on the include paths generate a list of filenames to check for
    struct definitions and register requests.

    Apply some filtering of the name list based on extesion and manual
    limiters. Resolve all symbolic links and normalize path formatting.
    """

    # Do we want anything to do with this file
    def valid_file(filename: str) -> bool:
        if not os.path.isfile(filename):
            return False

        valid_exts: tuple[str, ...] = (".cpp", ".c", ".hpp", ".h")
        if not any(filename[-len(ext) :] == ext for ext in valid_exts):
            return False

        patterns_to_skip: tuple[str, ...] = ()
        if any(skip in filename for skip in patterns_to_skip):
            return False

        files_to_skip: tuple[str, ...] = ("json.hpp",)
        if any(os.path.basename(filename) == skip for skip in files_to_skip):
            return False

        return True

    source_files: set[str] = set(glob.glob(DIR + "/**", recursive=True))

    for arg in includes:
        arg = arg.replace("-I", "")
        if os.path.isdir(arg):
            source_files.update(set(glob.glob(arg + "/**", recursive=True)))
        elif os.path.isfile(arg):
            source_files.add(arg)

    source_files = {os.path.realpath(file) for file in source_files if valid_file(file)}
    print(f"Pycstruct: Scanning {len(source_files)} files...")

    return source_files


def main(make_includes: list[str]) -> None:
    defined_structs: dict[str, ObjectFrame] = {}
    requests: set[StructRegisterRequest] = set()

    source_files = get_filename_list(make_includes)

    for filename in source_files:
        try:
            with open(filename, "r") as f:
                filetext = f.read()

            requests.update(find_requests(filetext, filename))
            defined_structs.update(find_top_struct_defs(filetext, filename))

        except Exception as e:
            print("Pycstruct:", filename, e)
            exit(1)

    instances: list[str] = []
    macros: list[str] = []

    # Generate a static instance and the required macros for everything the
    # user has asked for.
    for request in requests:
        if request.typename not in defined_structs:
            ValueError(
                f"Registered a struct that doesn't have a definition: {request.typename} [Request from {request.src_filename}]"
            )

        requested_struct = defined_structs[request.typename]

        print(
            f'Pycstruct: Generating macros for: {request.typename} "{request.instance_name}"'
        )

        if request.pragma_pack:
            instances.append(f"#pragma pack(push, {request.pragma_pack})")

        instances.append(
            f"static struct _{request.typename} {{{requested_struct.raw_body}}} {request.instance_name};"
        )

        if request.pragma_pack:
            instances.append(f"#pragma pack(pop)")

        macros.append(
            f'REGISTER_INTERNAL_STRUCT({request.typename}, {request.instance_name}, "{requested_struct.src_file}", "{reformat_struct(requested_struct.raw_full)}", {requested_struct.src_file_mtime});'
        )
        macros += defined_structs[request.typename].reg_macros(request.instance_name)
        macros.append("")

    with open(INSTANCE_FILE, "w") as f:
        f.write(OVERWRITE_WARNING)
        f.write("\n".join(instances) + "\n")

    with open(MACRO_FILE, "w") as f:
        f.write(OVERWRITE_WARNING)
        f.write("\n".join(macros))


def clean() -> None:
    with open(INSTANCE_FILE, "w") as f:
        f.write(OVERWRITE_WARNING)

    with open(MACRO_FILE, "w") as f:
        f.write(OVERWRITE_WARNING)


if __name__ == "__main__":
    clean()
    if "clean" not in sys.argv:
        main(sys.argv[1:])
