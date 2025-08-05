import re
import glob

# turn the user code into format easier for the parser to deal with
def deformat_source(struct_def:str) -> str:
    struct_def = struct_def.strip()

    # get rid of the comments manually since
    # the regex is doing weird shit
    i:int = struct_def.find('/*')
    while i >= 0:
        end = struct_def.find('*/', i)
        if end == -1:
            raise ValueError("Invalid comment")

        struct_def = struct_def[:i] + struct_def[end+2:]
        i = struct_def.find('/*')

    struct_def += '\n' # for // comments at end of file
    i = struct_def.find('//')
    while i >= 0:
        struct_def = struct_def[:i] + struct_def[struct_def.find('\n', i):]
        i = struct_def.find('//')
        
        
    # normalize whitespace around all important characters
    # remove default value initialization
    struct_def = re.sub(r'\s*{\s*', ' { ', struct_def, flags=re.DOTALL)
    struct_def = re.sub(r'\s*}\s*', ' } ', struct_def, flags=re.DOTALL)
    struct_def = re.sub(r'\s*\[\s*', ' [ ', struct_def, flags=re.DOTALL)
    struct_def = re.sub(r'\s*\]\s*', ' ] ', struct_def, flags=re.DOTALL)
    struct_def = re.sub(r'\s*;\s*', ' ; ', struct_def, flags=re.DOTALL)
    struct_def = re.sub(r'\s*\*\s*', ' * ', struct_def, flags=re.DOTALL)
    struct_def = re.sub(r'\s*:\s*', ':', struct_def, flags=re.DOTALL)
    struct_def = re.sub(r'\s*,\s*', ' , ', struct_def, flags=re.DOTALL)
    struct_def = re.sub(r'\s*struct[\s\{]+', ' struct ', struct_def, flags=re.DOTALL)
    struct_def = re.sub(r'\s*union[\s\{]+', ' union ', struct_def, flags=re.DOTALL)
    struct_def = re.sub(r'\s*=.+;', ';', struct_def, flags=re.DOTALL)
    struct_def = re.sub(r'\s+', ' ', struct_def, flags=re.DOTALL)
    return struct_def

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
#
#     brace_level:int = 0
#     lines:list[str] = []
#     for line in struct_def.split('\n'):
#         if '}' in line:
#             brace_level -= 1
#
#         lines.append('    ' * brace_level + line)
#
#         if '{' in line:
#             brace_level += 1
#
#     return '\n'.join(lines)

ctype_fmts:dict[str, tuple[str,str,str]] = {
    'bool':('u8', '%1X', 'stoul_0x'),
    'int8_t':('i8', '%3d', 'stol'),
    'char':('i8', '%3d', 'stol'),
    'signed char':('i8', '%3d', 'stol'),
    'uint8_t': ('u8', '%02X', 'stoul_0x'),
    'unsigned char': ('u8', '%02X', 'stoul_0x'),
    'int16_t':('i16', '%5d', 'stol'),
    'short': ('i16', '%5d', 'stol'),
    'short int': ('i16', '%5d', 'stol'),
    'signed short': ('i16', '%5d', 'stol'),
    'signed short int': ('i16', '%5d', 'stol'),
    'uint16_t':('u16', '%04X', 'stoul_0x'),
    'unsigned short': ('u16', '%04X', 'stoul_0x'),
    'unsigned short int': ('u16', '%04X', 'stoul_0x'),
    'int32_t': ('i32', '%9d', 'stol'),
    'int': ('i32', '%9d', 'stol'),
    'signed': ('i32', '%9d', 'stol'),
    'signed int': ('i32', '%9d', 'stol'),
    'uint32_t': ('u32', '%08X', 'stoul_0x'),
    'unsigned int': ('u32', '%08X', 'stoul_0x'),
    'int64_t': ('i64', '%20lld', 'stol'),
    'long': ('i64', '%20lld', 'stol'),
    'long int': ('i64', '%20lld', 'stol'),
    'signed long': ('i64', '%20lld', 'stol'),
    'signed long int': ('i64', '%20lld', 'stol'),
    'long long': ('i64', '%20lld', 'stol'),
    'long long int': ('i64', '%20lld', 'stol'),
    'signed long long': ('i64', '%20lld', 'stol'),
    'signed long long int': ('i64', '%20lld', 'stol'),
    'uint64_t': ('u64', '%016llX', 'stoul_0x'),
    'unsigned long': ('u64', '%016llX', 'stoul_0x'),
    'unsigned long int': ('u64', '%016llX', 'stoul_0x'),
    'unsigned long long': ('u64', '%016llX', 'stoul_0x'),
    'unsigned long long int': ('u64', '%016llX', 'stoul_0x'),
    'float': ('f32', '%14.4e', 'stod'),
    'double': ('f64','%14.4e', 'stod') ,
}


class CStdVar:
    def __init__(self, var_def:str) -> None:
        var_def = var_def.strip(' ;')
        self.raw = var_def
        
        is_arr:bool = '[' in var_def and ']' in var_def
        
        def_split = var_def.split(' ')
        def_split = [s.strip() for s in def_split if (s != '[' and s != ']')]

        if is_arr and len(def_split) < 3:
            raise ValueError(f'Invalid array declaration: "{var_def}"')

        elif not is_arr and len(def_split) < 2:
            raise ValueError(f'Invalid member declaration: "{var_def}"')

        self.size:str = def_split.pop() if is_arr else'0'
        self.name:str = def_split.pop()
        self.ctype:str = ' '.join(def_split)

        if self.ctype == 'char' and self.size != '0':
            self.ctype = 's'

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
    @staticmethod
    def get_frame(text:str) -> tuple[str, str, str, str, str]:
        # frame, remainder, type (struct, union), typename, varname
        # return the first full brace enclosure and the remainder of the text
        # are there more than just structs and unions?

        END:int = 1 << 31
        pos_end = lambda i: i if i >= 0 else END

        indent_level:int = 0
        first_struct:int = pos_end(text.find('struct'))
        first_union:int = pos_end(text.find('union'))

        if first_struct == END and first_union == END:
            return '', text, '', '', ''

        text = text[min(first_struct, first_union):]

        # make sure this is a definition and not 
        # a variable declaration
        first_semi:int = pos_end(text.find(';'))
        first_open:int = pos_end(text.find("{"))

        if first_semi < first_open:
            return '__skipped__', text[first_semi+1:], '', '', ''

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

                s_or_u, typename = frame.split(' ')[:2]

                if typename == '{':
                    typename = '_'+s_or_u
                    
                var_name = frame[frame.rfind('}')+1 : frame.rfind(';')].strip()
                
                return frame, remainder, s_or_u, typename, var_name

        raise ValueError(f'Invalid syntax: failed to find brace enclosure:', text)

    def __init__(self, body_def:str, instance_name:str, parents:list) -> None:

        self.parents:list[ObjectFrame] = parents
        self.raw_full:str = body_def.strip()
        self.frame_type:str = body_def.split(' ')[0]
        self.instance_name:str = instance_name

        self.typename:str = body_def.split(' ')[1]
        if self.typename == '{':
            self.typename = '_' + self.frame_type.capitalize()

        # remove the opening and closing of the main body
        self.raw_body:str = self.raw_full[self.raw_full.find('{')+1:]
        self.raw_body = self.raw_body[:self.raw_body.rfind('}')]

        self.structs:list[CStruct] = []
        self.unions:list[CUnion] = []
        self.bitfields:list[CBitfield] = []
        self.vars:list[CStdVar] = []

        self._get_frames()
        self._get_bitfields()
        self._get_std_vars()


        combo_types:list[str] = [ p.typename for p in self.parents]
        combo_names:list[str] = [ p.instance_name for p in self.parents if p.instance_name]

        self.combo_type:str = '::'.join(combo_types + [self.typename])
        self.combo_name:str = '.'.join(combo_names + [self.instance_name])


        self.macros:list[str] = []

        for b in self.bitfields:
            for name, width in b.fields:
                print(f"{{bitfield}} {self.combo_type} {self.combo_name}: {b.ctype} {name}:{width};")

        for v in self.vars:
            if v.size == '0':
                print(f"{{var}} {self.combo_type} {self.combo_name}: {v.ctype} {v.name};")
            else:
                print(f"{{arrray}} {self.combo_type} {self.combo_name}: {v.ctype} {v.name}[{v.size}];")

        
    def reg_macros(self, base_name) -> list[str]:
        macros:list[str] = []

        for frame in self.structs + self.unions:
            macros += frame.reg_macros(base_name)

        if len(self.combo_name) > 0:
            self.combo_name += '.'

        for v in self.vars:
            if v.ctype in ctype_fmts:
                _, printf, stonum = ctype_fmts[v.ctype]
                if v.size == '0':
                    macros.append(f'REGISTER_VAR({base_name}, {self.combo_name}{v.name}, {v.ctype}, "{printf}", {stonum})')
                else:
                    macros.append(f'REGISTER_ARR({base_name}, {self.combo_name}{v.name}, {v.size}, {v.ctype}, "{printf}", {stonum})')

        
        for b in self.bitfields:
            if b.ctype in ctype_fmts:
                _, printf, stonum = ctype_fmts[b.ctype]
                for name, _ in b.fields:
                    macros.append(f'REGISTER_BITFIELD({base_name}, {self.combo_name}{name}, {b.ctype}, "{printf}", {stonum})')


        return macros

    def if_print_statements(self, base_name:str) -> list[str]:
        if_checks:list[str] = []

        for frame in self.structs + self.unions:
            if_checks += frame.if_print_statements(base_name)

        if len(self.combo_name) > 0:
            self.combo_name += '.'

        for v in self.vars:
            if_checks += [f'if (structname == "{base_name}" && varname == "{self.combo_name}{v.name}") {{}}']

        for b in self.bitfields:
            for f, _ in b.fields:
                if_checks += [f'if (structname == "{base_name}" && varname == "{self.combo_name}{f}") {{}}']
                
        return if_checks

    def _get_frames(self) -> None:
        remainder = self.raw_body

        while True:
            frame, remainder, s_or_u, _, instance_name = ObjectFrame.get_frame(remainder)
            
            if frame == '__skipped__':
                continue

            if len(frame) == 0:
                return

            if s_or_u == 'struct':
                self.structs.append(CStruct(frame, instance_name, self.parents + [self]))

            elif s_or_u == 'union':
                self.unions.append(CUnion(frame, instance_name, self.parents + [self]))

            else:
                raise ValueError(f"Invalid frame type: {s_or_u}")


    def _get_bitfields(self) -> None:
        only_members = self.raw_body

        for u in self.unions:
            only_members = only_members.replace(u.raw_full, '')

        for s in self.structs:
            only_members = only_members.replace(s.raw_full, '')

        for member in only_members.split(';'):
            if ':' in member and not '::' in member:
                self.bitfields.append(CBitfield(member))

    def _get_std_vars(self) -> None:
        only_members = self.raw_body

        for s in self.structs:
            only_members = only_members.replace(s.raw_full, '')

        for u in self.unions:
            only_members = only_members.replace(u.raw_full, '')

        for b in self.bitfields:
            only_members = only_members.replace(b.raw, '')

        for member in only_members.split(';'):
            if len(member.strip()) != 0:
                self.vars.append(CStdVar(member))



class CStruct(ObjectFrame):
    def __init__(self, struct_def:str, instance_name:str, parents:list[ObjectFrame] = []) -> None:
        super().__init__(struct_def, instance_name, parents)


class CUnion(ObjectFrame):
    def __init__(self, union_def:str, instance_name:str, parents:list[ObjectFrame] = []) -> None:
        super().__init__(union_def, instance_name, parents)
    



def find_registrations(filenames:list[str]) -> list[tuple[str, str]]:
    registers:list[tuple[str, str]] = []

    for filename in filenames:
        if 'structs.h' in filename:
            continue

        with open(filename, 'r') as f:
            text = f.read()

        text = text.replace('#define REGISTER_STRUCT', '')
        while text.find('REGISTER_STRUCT') != -1:

            text = text[text.find('REGISTER_STRUCT'):]

            name_reg = text[text.find("(")+1 : text.find(")")]

            if ';' in name_reg or '{' in name_reg or '}' in name_reg or name_reg.count(',') != 1:
                ValueError("Invalid struct registration")

            struct, name = (s.strip() for s in name_reg.split(',')) 

            if len(struct) == 0 or len(name) == 0:
                ValueError("Invalid struct registration")

            registers.append((struct, name))
            text = text[1:] # shit

    return registers


# todo: chase out provided include paths
def find_bases(filenames:list[str]) -> list[CStruct]:

    sdefs:list[CStruct] = [] 
    for filename in filenames:
        if 'structs.h' in filename:
            continue

        with open(filename, "r") as f:
            raw = f.read()
            
        text = '\n'.join([line for line in raw.split('\n') if len(line) > 0 and line[0] != '#'])
        text = deformat_source(text)
        
        while text.find('struct') != -1:
            sdef, text, _, _, _ = ObjectFrame.get_frame(text)
            if sdef == '__skipped__':
                continue
            sdefs.append(CStruct(sdef, ''))
     
    return sdefs


def main() -> None:
    source_files:list[str] = glob.glob('./*.h') + glob.glob('./*.cpp')

    source_files.remove('./structs.cpp')
    source_files.remove('./structs.h')

    defined_structs:dict[str, CStruct] = { s.typename:s for s in find_bases(source_files) }

    with open('structs.cpp') as f:
        struct_cpp:str = f.read()

    struct_cpp = re.sub(r'//\s*pycstruct_shit.*', '//pycstruct_shit\n', struct_cpp, flags=re.DOTALL)

    registers:str = ''
    static_instances:str = ''

    for struct_type, struct_name in find_registrations(source_files):
        if struct_type not in defined_structs:
            ValueError("Registered a struct that doesn't have a definition")

        registers += f'    REGISTER_INTERNAL_STRUCT(_{struct_type}, {struct_name})\n'
        registers += '    ' + '\n    '.join(defined_structs[struct_type].reg_macros(struct_name))

        static_instances += f'static struct _{struct_type} {{{defined_structs[struct_type].raw_body}}} {struct_name};\n'


    instance_def = f'{static_instances}\n\n'
    init_def = f'void init_structs()\n{{\n{registers}\n}}\n\n'

    struct_cpp += instance_def + init_def

    with open('structs.cpp', 'w') as f:
        f.write(struct_cpp)




if __name__ == "__main__":
    main()
    


