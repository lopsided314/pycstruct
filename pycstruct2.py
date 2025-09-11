import sys, os
import re
import glob

'''
Basically what this does...

1. Go through all the files in the workspace to get all struct definitions and
   find the REGISTER_STRUCT macro calls.
2. Parse each struct definition to find all of its members and determine their 
   properties. This is a recursive process, allowing unions and other structs
   to be contained within a struct.
3. Generate the C++ code required to access the struct data for all the structs
   that got registered with the REGISTER_STRUCT macro.
   a. The register variable macros
   b. the 'private' static struct instance
4. Overwrite the pycstruct section of the structs.cpp source file with the 
   code generated from this script.

'''

DIR = os.path.dirname(os.path.abspath(__file__))
DEBUG_PRINT = False

# turn the user code into format easier for the parser to deal with
def deformat_source(src_text:str) -> str:
    src_text = src_text.strip()

    src_text = src_text.replace('\\', '')

    # get rid of the comments manually since
    # the regex is doing weird shit
    i:int = src_text.find('/*')
    while i >= 0:
        end = src_text.find('*/', i)
        if end == -1:
            raise ValueError("Comment wasn't closed")

        src_text = src_text[:i] + src_text[end+2:]
        i = src_text.find('/*')

    src_text += '\n' # for // comments at end of file
    i = src_text.find('//')
    while i >= 0:
        src_text = src_text[:i] + src_text[src_text.find('\n', i):]
        i = src_text.find('//')
        
    # remove default initialized values
    i = src_text.find('=')
    while i >= 0:
        src_text = src_text[:i] + src_text[src_text.find(';', i):]
        i = src_text.find('=')
        
    # normalize whitespace around all important characters
    src_text = re.sub(r'\s*{\s*', ' { ', src_text, flags=re.DOTALL)
    src_text = re.sub(r'\s*}\s*', ' } ', src_text, flags=re.DOTALL)
    src_text = re.sub(r'\s*\[\s*', ' [ ', src_text, flags=re.DOTALL)
    src_text = re.sub(r'\s*\]\s*', ' ] ', src_text, flags=re.DOTALL)
    src_text = re.sub(r'\s*;\s*', ' ; ', src_text, flags=re.DOTALL)
    src_text = re.sub(r'\s*\*\s*', ' * ', src_text, flags=re.DOTALL)
    src_text = re.sub(r'\s*:\s*', ':', src_text, flags=re.DOTALL)
    src_text = re.sub(r'\s*,\s*', ' , ', src_text, flags=re.DOTALL)
    src_text = src_text.replace('struct{', 'struct {')
    src_text = src_text.replace('union{', 'union {')
    src_text = re.sub(r'\s*struct\s+', ' struct ', src_text, flags=re.DOTALL)
    src_text = re.sub(r'\s*union\s+', ' union ', src_text, flags=re.DOTALL)
    src_text = re.sub(r'\s+', ' ', src_text, flags=re.DOTALL)
    return src_text

# put the struct back into user readable format
# def reformat_struct(struct_def:str) -> str:
#     struct_def = struct_def.strip()
#     struct_def = re.sub(r'\s*struct\s*', 'struct ', struct_def, flags=re.DOTALL)
#     struct_def = re.sub(r'\s*union\s*', 'union ', struct_def, flags=re.DOTALL)
#     struct_def = re.sub(r'\s*{\s*', '\n{\n', struct_def, flags=re.DOTALL)
#     struct_def = re.sub(r'\s*}\s*', '\n}', struct_def, flags=re.DOTALL)
#     struct_def = re.sub(r'\s*;\s*', ';\n', struct_def, flags=re.DOTALL)
#     struct_def = re.sub(r'\s*,\s*', ', ', struct_def, flags=re.DOTALL)
#     struct_def = re.sub(r'\s*\*\s*', ' *', struct_def, flags=re.DOTALL)
#     brace_level:int = 0
#     lines:list[str] = []
#     for line in struct_def.split('\n'):
#         if '}' in line:
#             brace_level -= 1
#         lines.append('    ' * brace_level + line)
#         if '{' in line:
#             brace_level += 1
#     return '\n'.join(lines)

cli_types:dict[str, tuple[str, str, str]] = {
    'i8': ('i8', '%4d', 'stol'),
    'u8': ('u8', '%02X', 'stoul_0x'),
    'i16': ('i16', '%6d', 'stol'),
    'u16': ('u16', '%04X', 'stoul_0x'),
    'i32': ('i32', '%11d', 'stol'),
    'u32': ('u32', '%08X', 'stoul_0x'),
    'i64': ('i64', '%20ld', 'stol'),
    'li64': ('i64', '%20lld', 'stol'),
    'u64': ('u64', '%016lX', 'stoul_0x'),
    'lu64': ('u64', '%016llX', 'stoul_0x'),
    'f32': ('f32', '%11.4e', 'stod'),
    'f64': ('f64', '%11.4e', 'stod') ,
}
ctype_fmts:dict[str, tuple[str,str,str]] = {
    'bool':('u8', '%1X', 'stoul_0x'),
    'int8_t': cli_types['i8'],
    'char': cli_types['i8'],
    'signed char': cli_types['i8'],
    'uint8_t': cli_types['u8'],
    'unsigned char': cli_types['u8'],
    'int16_t': cli_types['i16'],
    'short': cli_types['i16'],
    'short int': cli_types['i16'],
    'signed short': cli_types['i16'],
    'signed short int': cli_types['i16'],
    'uint16_t': cli_types['u16'],
    'unsigned short': cli_types['u16'],
    'unsigned short int': cli_types['u16'],
    'int32_t': cli_types['i32'],
    'int': cli_types['i32'],
    'signed': cli_types['i32'],
    'signed int': cli_types['i32'],
    'uint32_t': cli_types['u32'],
    'unsigned int': cli_types['u32'],
    'int64_t': cli_types['i64'],
    'long': cli_types['i64'],
    'long int': cli_types['i64'],
    'signed long': cli_types['i64'],
    'signed long int': cli_types['i64'],
    'long long': cli_types['li64'],
    'long long int': cli_types['li64'],
    'signed long long': cli_types['li64'],
    'signed long long int': cli_types['li64'],
    'uint64_t': cli_types['u64'],
    'unsigned long': cli_types['u64'],
    'unsigned long int': cli_types['u64'],
    'unsigned long long': cli_types['lu64'],
    'unsigned long long int': cli_types['lu64'],
    'float': cli_types['f32'],
    'double': cli_types['f64'],
}

#
# Regular variables and arrays
#
class CStdVar:
    def __init__(self, var_def:str) -> None:
        var_def = var_def.strip(' ;')
        self.raw = var_def
        
        is_arr:bool = '[' in var_def and ']' in var_def
        
        def_split = [s.strip() for s in var_def.split(' ') if (s != '[' and s != ']')]

        if is_arr and len(def_split) < 3:
            raise ValueError(f'Invalid array declaration: "{var_def}"')

        elif not is_arr and len(def_split) < 2:
            raise ValueError(f'Invalid member declaration: "{var_def}"')

        self.size:int = int(def_split.pop()) if is_arr else 0
        self.name:str = def_split.pop()
        self.ctype:str = ' '.join(def_split)


#
# Bitfields
#
class CBitfield:
    def __init__(self, bitfield_def:str) -> None:
        self.raw:str = bitfield_def.strip()
        self.ctype:str = ''
        self.fields:list[tuple[str, str]] = []

        for token in self.raw.split(' '):
            if ';' in token or ',' in token:
                continue;

            if ':' not in token:
                self.ctype += token + ' '
                continue

            try:
                name, size = token.split(':')
                if len(name) > 0:
                    self.fields.append((name.strip(), size))
            except ValueError:
                raise ValueError(f'Invalid bitfield declaration: "{bitfield_def}"')
            
        self.ctype = self.ctype.strip()

            
class ObjectFrame:
    def __init__(self, body_def:str, parents:list) -> None:

        self.parents:list[ObjectFrame] = parents
        self.raw_full:str = body_def.strip()
        self.instance_name:str = body_def[body_def.rfind('}')+1 : body_def.rfind(';')].strip() # struct S {} s;

        self.frame_type:str = body_def.split(' ')[0] # struct, union
        self.typename:str = body_def.split(' ')[1]
        self.frame_type, self.typename = body_def.split(' ')[:2]
        if self.typename == '{': # nameless struct
            self.typename = '_' + self.frame_type.capitalize()

        # isolate the contents of the struct
        self.raw_body:str = self.raw_full[self.raw_full.find('{')+1:self.raw_full.rfind('}')]

        self.frames:list['ObjectFrame'] = []
        self.bitfields:list[CBitfield] = []
        self.vars:list[CStdVar] = []

        self._get_frames()
        self._get_vars()

        parent_types:list[str] = [ p.typename for p in self.parents ]
        parent_names:list[str] = [ p.instance_name for p in self.parents if p.instance_name ]

        self.combo_type:str = '::'.join(parent_types + [self.typename])
        self.combo_name:str = '.'.join(parent_names + [self.instance_name])

        if DEBUG_PRINT:
            self.print()


    def print(self) -> None:
        header = f'Pycstruct: struct {self.combo_type} {self.combo_name}... '
        for b in self.bitfields:
            for name, width in b.fields:
                print(f"{header} {b.ctype} {name}:{width};")

        for v in self.vars:
            var_info = f'{header} {v.ctype} {v.name}'
            if v.size == 0:
                print(f"{var_info};")
            else:
                print(f"{var_info}[{v.size}];")


    def reg_macros(self, base_name) -> list[str]:
        macros:list[str] = []

        for frame in self.frames:
            macros += frame.reg_macros(base_name)

        for v in self.vars:
            if v.ctype not in ctype_fmts:
                continue

            var_name:str = f'{self.combo_name+"." if self.combo_name else ""}{v.name}'
            print(var_name)

            _, printf, stonum = ctype_fmts[v.ctype]
            if v.size == 0:
                macros.append(f'REGISTER_VAR({base_name}, {var_name}, {v.ctype}, "{printf}", {stonum});')
            elif v.ctype == 'char':
                macros.append(f'REGISTER_CHAR_ARR({base_name}, {var_name})')
            else:
                macros.append(f'REGISTER_ARR({base_name}, {var_name}, {v.size}, {v.ctype}, "{printf}", {stonum});')


        for b in self.bitfields:
            if b.ctype in ctype_fmts:
                _, printf, stonum = ctype_fmts[b.ctype]
                printf = printf.replace("0","")
                for var_name, _ in b.fields:
                    macros.append(f'REGISTER_BITFIELD({base_name}, {self.combo_name}{var_name}, {b.ctype}, "{printf}", {stonum});')

        return macros


    def _get_frames(self) -> None:
        remainder = self.raw_body

        while True:
            frame, remainder = get_object_frame(remainder)

            if frame == '__skipped__':
                continue

            if len(frame) == 0:
                return

            self.frames.append(ObjectFrame(frame, self.parents + [self]))


    def _get_vars(self) -> None:
        only_members = self.raw_body

        for f in self.frames:
            only_members = only_members.replace(f.raw_full, '')

        for member in only_members.split(';'):
            if ':' in member and not '::' in member:
                self.bitfields.append(CBitfield(member))
            elif len(member.strip()) != 0:
                self.vars.append(CStdVar(member))


def get_object_frame(text:str) -> tuple[str, str]:
    # return the first full brace enclosure and the remainder of the text

    END:int = 1 << 31
    pos_end = lambda i: i if i >= 0 else END

    #MAYBE TODO: struct can be sep by any whitespace
    first_frame:int = min((
        pos_end(text.find('struct ')),
        pos_end(text.find('union ')),
    ))
    if first_frame == END:
        return '', text

    text = text[first_frame:]

    first_open:int = pos_end(text.find("{"))
    first_close:int = pos_end(text.find("}"))
    first_semi:int = pos_end(text.find(";"))

    # characters that signify features that we cannot deal with
    # functions, templates, constructors, ...
    #
    # std::unique_ptr<struct S>{new struct S}
    bad_chars:str = '()<>' 
    first_bad_char:int = min(pos_end(text.find(c)) for c in bad_chars)

    if min(first_bad_char, first_semi, first_close) < first_open:
        return '__skipped__', text[5:]

    indent_level:int = 0
    for i, c in enumerate(text):
        # wait for the start of the first
        # body to count indents
        if indent_level == 0: 
            if c == '{':
                indent_level = 1
            continue

        if c == '{':
            indent_level += 1
        elif c == '}':
            indent_level -= 1
        
        if indent_level == 0:
            # we found the matching pair for the starting brace
            def_end:int = text.find(';', i)+1
            frame = text[:def_end].strip()
            remainder = text[def_end+1:].strip()

            if any(c in frame for c in bad_chars) or frame == '\\':
                return '__skipped__', remainder

            s_or_u = frame.split(' ')[0]

            if s_or_u != 'struct' and s_or_u != 'union':
                raise ValueError(f'Invalid syntax: frame is neither struct nor union: {frame}')

            return frame, remainder

    raise ValueError(f'Invalid syntax: failed to find closing brace:', text)


#
# Go through the file and find all empty macro calls
# that indicate to this program which structs we need to handle
#
def find_registrations(text:str) -> set[tuple[str, str, str]]:
    registers:set[tuple[str, str, str]] = set()

    text = text.replace('#define REGISTER_STRUCT', '') # ignore the macro definition
    while text.find('REGISTER_STRUCT') != -1:

        text = text[text.find('REGISTER_STRUCT'):]

        name_reg = text[text.find("(")+1 : text.find(")")]

        if ';' in name_reg or '{' in name_reg or '}' in name_reg:
            ValueError("Invalid struct registration")

        struct, name, pragma_pack = '', '', '-1'
        if name_reg.count(',') == 1:
            struct, name = (s.strip() for s in name_reg.split(',')) 
        elif name_reg.count(',') == 2:
            struct, name, pragma_pack = (s.strip() for s in name_reg.split(',')) 
        else:
            ValueError("Invalid struct registration")

        if len(struct) == 0 or len(name) == 0:
            ValueError("Invalid struct registration")

        registers.add((struct, name, pragma_pack))
        text = text[1:] # shit

    return registers

#
# Find all the struct definitions in the file
#
def find_struct_defs(text:str) -> dict[str, ObjectFrame]:

    sdefs:list[ObjectFrame] = [] 
        
    # remove all the pre-processor bullshit
    text = '\n'.join([line for line in text.split('\n') if len(line) > 0 and line[0] != '#'])
    text = deformat_source(text)
    
    #TODO: should check against any whitespace
    while text.find('struct ') != -1:
        sdef, text = get_object_frame(text)
        if sdef == '__skipped__':
            continue
        sdefs.append(ObjectFrame(sdef, []))

    return { s.typename:s for s in sdefs if s.frame_type == 'struct'}





# Do we want anything to do with this file
def valid_file(filename:str) -> bool:
    if not os.path.isfile(filename):
        return False
    
    valid_exts:tuple[str, ...] = ('.cpp', '.c', '.hpp', '.h')
    if not any(filename[-len(ext):] == ext for ext in valid_exts):
        return False

    files_to_skip:tuple[str, ...] = ()
    if any(os.path.basename(filename) == skip for skip in files_to_skip):
        return False

    return True

def main(make_includes:list[str]) -> None:

    source_files:set[str] = set(glob.glob('./**', recursive=True))
    
    for arg in make_includes:
        arg = arg.replace('-I', '')
        if os.path.isdir(arg):
            source_files.update(set(glob.glob(arg+'/**', recursive=True)))
        elif os.path.isfile(arg):
            source_files.add(arg)

    source_files = {os.path.realpath(file) for file in source_files if valid_file(file)}
    print(f'Pycstruct: Scanning {len(source_files)} files...')

    defined_structs:dict[str, ObjectFrame] = {}
    registered_structs:set[tuple[str, str, str]] = set()

    for filename in source_files:
        try:
            with open(filename, "r") as f:
                filetext = f.read()
                registered_structs.update(find_registrations(filetext))
                defined_structs.update(find_struct_defs(filetext))
        except Exception as e:
            print('Pycstruct:', filename, e)
            exit(1)


    static_instances:str = ''
    registers:str = ''

    for registered_type, registered_name, pragma_pack in registered_structs:
        if registered_type not in defined_structs:
            ValueError("Registered a struct that doesn't have a definition")

        print(f'Pycstruct: Generating macros for: {registered_type} "{registered_name}"')

        if pragma_pack != '-1':
            static_instances += f'#pragma pack(push, {pragma_pack})\n'
        static_instances += f'static struct _{registered_type} {{{defined_structs[registered_type].raw_body}}} {registered_name};\n'
        if pragma_pack != '-1':
            static_instances += f'#pragma pack(pop)\n'

        registers += f'    REGISTER_INTERNAL_STRUCT(_{registered_type}, {registered_name});\n'
        registers += '    ' + '\n    '.join(defined_structs[registered_type].reg_macros(registered_name)) + '\n\n'



    instance_def = f'{static_instances}\n\n'
    init_def = f'void init_structs()\n{{\n    {registers.strip()}\n}}\n\n'

    with open(DIR+'/structs.cpp') as f:
        struct_cpp:str = f.read()

    struct_cpp = re.sub(r'//\s*pycstruct_shit.*', '//pycstruct_shit\n', struct_cpp, flags=re.DOTALL)
    struct_cpp += instance_def + init_def

    with open(DIR+'/structs.cpp', 'w') as f:
        f.write(struct_cpp)





if __name__ == "__main__":
    main(sys.argv[1:])

    


