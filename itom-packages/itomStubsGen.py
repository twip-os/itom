# coding=iso-8859-15

"""Parses the stubs file for the entire itom module.

The pyi stubs file is put into itom-stubs/__init__.py,
such that Jedi is able to use this for auto completion,
calltips etc.
"""

import sys
import os
import inspect
from typing import List, Optional, Sequence, Tuple
import itom
import re

__version__ = "1.0.0"


class Arg:
    """Argument object for one parsed argument of a signature or docstring.
    
    One argument can either be a parameter or a return or yield value.
    """
    
    def __init__(self, name: str = "", dtype: Optional[str] = None, 
                 default: Optional[str] = None):
        self.name: str = name
        self.dtype: str = dtype
        self.default: str = default
        if default is None:
            self.optional: bool = False
        else:
            self.optional: bool = True
    
    def tostring(self) -> str:
        """Returns this argument as it would be written using the typing module."""
        text = self.name
        if self.dtype:
            text += ": " + self.dtype
        if self.default:
            text += " = " + self.default
        elif self.optional:
            text += " = \"???\""
        return text


class Signature:
    """This objects stands for the signature of one method.
    
    Args:
        name: is the name of the signature
        args: is a list of arguments, used to call the method
        rettype: is the potential return type (as string) or None if unknown or not given.
    """
    def __init__(self, name: str, args: List[Arg], rettype: Optional[str] = None):
        self.name = name
        self.args = args
        self.rettype = rettype
    
    def tostring(self) -> str:
        """Returns the full signature as it would be written in a script using the
        typing module."""
        sig = "def %s(%s)" % (
            self.name,
            ", ".join([a.tostring() for a in self.args]))
        if self.rettype:
            sig += " -> " + self.rettype
        return sig


def parse_stubs():
    """entry point method."""
    sys_version = sys.version_info
    py_version: str = "%i.%i" % (sys_version.major, sys_version.minor)
    
    base_folder: str = os.path.dirname(os.path.abspath(
        inspect.getfile(inspect.currentframe())))
    
    stubs_file: str = 'itom-stubs' + os.path.sep + '__init__.pyi'
    
    if py_version < "3.5":
        # Python < 3.5 does not know the typing module.
        # do not generate any stubs.
        if os.path.exists(stubs_file):
            os.remove(stubs_file)
    else:
        
        texts = [i for i in _parse_object(itom, indent=0)]
        
        # remove empty entries
        texts = [t for t in texts if t != ""]
        
        text = "\n\n".join(texts)
        
        text = "from typing import overload, Tuple, List, Dict, Optional\n\n" + text
        
        with open(os.path.join(base_folder, stubs_file), 'wt') as fp:
            fp.write(text)
        
        for t in text.split("\n"):
            print(t)


def _get_direct_members(obj) -> Tuple[str, object, object]:
    for itemname in dir(obj):
        if itemname in obj.__dict__:
            yield (itemname, getattr(obj, itemname), obj.__dict__[itemname])


def _parse_object(obj, indent: int = 0) -> str:
    
    itom_excludes: List[str] = []
    prefix: str = " " * indent
    
    try:
        if obj.__name__ == "itom":
            itom_excludes = [
                "gcStartTracking",
                "gcEndTracking",
                "autoReloader",
                "getDebugger",
                "proxy",
                "pythonStream"]
    except Exception:
        pass
    
    for childname, child, child2 in _get_direct_members(obj):
        if (childname not in itom_excludes and childname != "__init__"):
            if inspect.isclass(child):
                # class
                text = _get_class_signature(child, indent)
                
                subtext = [i for i in _parse_object(child, indent=indent+4)]
                
                # remove empty entries
                subtext = [t for t in subtext if t != ""]
                subtext = "\n\n".join(subtext)
                yield text + "\n" + " " * indent + subtext
            elif callable(child):
                if isinstance(child2, staticmethod):
                    # static method of a class
                    yield _parse_staticmeth_docstring(child, indent)
                else:
                    # method or function
                    yield _parse_meth_docstring(child, indent)
            elif type(child) is int:
                yield "%s#: constant int value.\n%s%s: int = %i" % \
                    (prefix, prefix, childname, child)
            elif type(child) is float:
                yield "%s#: constant float value.\n%s%s: float = %f" % \
                    (prefix, prefix, childname, child)
            elif inspect.ismodule(child):
                # ignore for now
                pass
            elif inspect.isgetsetdescriptor(child) or \
                    inspect.ismemberdescriptor(child):
                # property
                yield _parse_property_docstring(child, indent)
            else:
                # contant or something else
                yield "%s#: constant %s value.\n%s%s" % \
                    (prefix, type(child), prefix, childname)


def _get_class_signature(classobj: type, indent: int) -> str:
    """
    """
    if not inspect.isclass(classobj):
        raise RuntimeError("%s must be a class object" % str(classobj))
    
    b = classobj.__bases__
    prefix = " " * indent
    
    if len(b) == 0 or (len(b) == 1 and b[0] is object):
        sig = prefix + "class %s:\n" % classobj.__name__
    else:
        sig = prefix + "class %s(%s):\n" % (
            classobj.__name__,
            ", ".join([item.__name__ for item in classobj.__bases__]))
    
    if classobj.__doc__ is not None:
        prefix += " " * 4
        
        signatures, docstrings = _get_signatures_and_docstring(classobj)
        
        sig += prefix +  "\"\"\"\n"
        sig += "\n".join([prefix + d for d in docstrings])
        sig += "\n" + prefix + "\"\"\"\n\n"
        
        overload_text: str = ""
        if len(signatures) > 1:
            overload_text: str = prefix + "@overload\n"
        
        # add the __init__ method here
        for signature in signatures:
            text = signature.tostring()
            text = text.replace("def %s" % classobj.__name__, "def __init__")
            sig += overload_text + prefix + text + ":\n"+ prefix + "    pass\n\n"
    else:
        # add the __init__ method
        sig += prefix + "def __init__(self, *args, **kwds):\n" + prefix + "    pass\n"
    
    return sig


def _parse_args_string(argstring: str) -> List[Arg]:
    
    # 1. search comma separated first-level sections
    opened_brackets: List[str] = []
    args: List[str] = []
    lastidx: int = 0
    
    for idx in range(len(argstring)):
        c = argstring[idx]
        
        if c == "," and len(opened_brackets) == 0:
            args.append(argstring[lastidx:idx])
            lastidx = idx + 1
        elif c in ["(", "[", "{"]:
            opened_brackets.append(c)
        elif len(opened_brackets) > 0:
            if c == ")" and opened_brackets[-1] == "(":
                opened_brackets = opened_brackets[0:-1]
            elif c == "]" and opened_brackets[-1] == "[":
                opened_brackets = opened_brackets[0:-1]
            if c == "}" and opened_brackets[-1] == "{":
                opened_brackets = opened_brackets[0:-1]
    
    args.append(argstring[lastidx:])
    args = [a for a in args if a != ""]
    
    result: List[Arg] = []
    
    for arg in args:
        idx_colon = arg.find(":")
        idx_equal = arg.find("=", max(0, idx_colon))
        
        if idx_colon == -1 and idx_equal == -1:
            result.append(Arg(arg.strip(), None, None))
        else:
            if idx_colon == -1:
                name = arg[0:idx_equal]
            else:
                name = arg[0:idx_colon]
            dtype = None
            default = None
            if idx_equal != -1:
                default = arg[idx_equal + 1:].strip()
                if idx_colon != -1:
                    dtype = arg[idx_colon + 1:idx_equal].strip()
            elif idx_colon != -1:
                dtype = arg[idx_colon + 1:].strip()
            result.append(Arg(name.strip(), dtype, default))
    
    if len(opened_brackets) > 0:
        print("invalid argument string: %s" % argstring, file=sys.stderr)
    
    return result


def _ismethod(obj):
    return inspect.ismethod(obj) or \
        inspect.ismethoddescriptor(obj)


def _parse_npdoc_argsection(doc_str: str, section_name: str) -> Optional[List[Arg]]:
    """."""
    assert(section_name in ["Parameters", "Returns", "Yields"])
    
    doc_lines = [line.rstrip() for line in doc_str.split("\n")]
    
    try:
        # there must be at least one line after the section title
        first_idx: int = doc_lines[0:-1].index(section_name)
    except ValueError:
        return None
    
    if not doc_lines[first_idx + 1].startswith("-" * len(section_name)):
        # there is no ---------------- line after the section title.
        print("The section title in the docstring is not followed by --------")
        return None
    
    lines: List[str] = []
    
    # search the end of the section, defined by section_name
    # the section ends, if one lines exists, that starts at column 0, followed
    # by a next line, starting with ----.
    
    for line, next_line in zip(doc_lines[first_idx+2:], doc_lines[first_idx+3:-1]):
        if line != "" and line[0] != "" and next_line.startswith("----"):
            break
        else:
            lines.append(line)
    
    # split the lines into different arguments. Each argument starts at column 0.
    arguments: List[List[str]] = []
    current_arg: Optional[List[str]] = None
    
    for idx, text in enumerate(lines):
        if not text.startswith(" ") and text.strip() != "":
            if current_arg is not None:
                arguments.append(current_arg)
            current_arg = [text, ]
        else:
            current_arg.append(text)
    
    if current_arg is not None:
        empty: bool = len([c for c in current_arg if c.strip() != ""]) == 0
        if not empty:
            arguments.append(current_arg)
    
    # parse the return list
    args: List[Arg] = []
    
    for a in arguments:
        sig = a[0]  # only the first line is relevant
        optional: bool = False
        name: str = ""
        
        # check if there is an ', optional' string at the end
        if sig.strip().endswith(", optional"):
            optional = True
            sig = sig[0:-len(", optional")]
        
        colon_idx: int = sig.find(": ")
        if colon_idx == -1:
            # the first line does not contain a : -> it is only the type
            if section_name == "Parameters":
                # a single value is the name only
                name = sig.strip()
                type_str = None
            else:
                # a single value in returns or yields is the type
                type_str: str = _nptype2typing(sig)
        else:
            name = sig[0:colon_idx].strip()
            type_str: str = _nptype2typing(sig[colon_idx + 1:])
        
        if name.startswith("*"):
            # *args or **kwds are always optional
            optional = True
        
        args.append(Arg(name, type_str, default=None))
        args[-1].optional = optional
    
    return args


def _get_rettype_from_npdocstring(doc_str: str) -> Optional[str]:
    """Try to parse a return type from a numpy docstring in doc_str.
    
    The return type (following the rules of the Python typing module)
    is either found in a Returns or a Yields section.
    
    For more information see: https://numpydoc.readthedocs.io/en/latest/format.html
    """
    args: Optional[List[Arg]] = _parse_npdoc_argsection(doc_str, "Returns")
    
    if args is None:
        args = _parse_npdoc_argsection(doc_str, "Yields")
    
    if args is None:
        return None
    
    if len(args) == 0:
        return None
    elif len(args) == 1:
        return args[0].dtype
    else:
        return "Tuple[%s]" % ", ".join([a.dtype for a in args])


def _get_parameters_from_npdocstring(doc_str: str) -> Optional[List[Arg]]:
    return _parse_npdoc_argsection(doc_str, "Parameters")


def _parse_signature_from_first_line(obj, first_line: str) -> Signature:
    
    # find arrow
    comps = first_line.split("->")
    
    sig = Signature(obj.__name__, args=[], rettype="")
    
    if _ismethod(obj) or inspect.isclass(obj):  # bound method
        sig.args += [Arg("self", None, None),]
    
    if len(comps) > 2:
        # more than one arrow
        comps2 = first_line.split(") ->")
        comps3 = first_line.split(")->")
        
        if len(comps2) == 2:
            comps = comps2
            comps[0] += ")"
        elif len(comps3) == 2:
            comps = comps3
            comps[0] += ")"
        else:
            comps = [comps[0], "->".join(comps[1:])]
    
    if len(comps) == 2:  # arrow with rettype
        sig.rettype = comps[1].strip()
    
    if len(comps) >= 1:  # parse args
        regexp = re.compile("(\w+)\((.*)\)")
        m = regexp.match(comps[0])
        
        if m is None:
            raise ValueError("invalid signature in docstring of obj. %s: %s" % 
                             (obj.__qualname__, first_line))
        else:
            g = m.groups()
            if g[0] != obj.__name__:
                print("name in signature of obj. %s does not match: %s" % 
                      (obj.__qualname__, comps[0]),
                      file=sys.stderr)
            
            args = g[1]
            sig.args += _parse_args_string(args)
    
    if obj.__doc__ is not None:
        doc_str = obj.__doc__
        
        # check if all arguments and the rettype have typehints already. If not,
        # try to get them from a numpy docstyle:
        if sig.rettype is None:
            # check for Returns sections
            sig.rettype = _get_rettype_from_npdocstring(doc_str)
        
        missing_argtypes = [a for a in sig.args if a.dtype is None]
        if len(missing_argtypes) > 0:
            # check for Parameters section
            np_args = _get_parameters_from_npdocstring(doc_str)
            
            if np_args is not None:
                arg_names: List[Arg] = [a.name for a in np_args]
                
                for a in sig.args:
                    if a.dtype is None and a.name in arg_names:
                        a.dtype = np_args[arg_names.index(a.name)].dtype
                        a.optional = np_args[arg_names.index(a.name)].optional
    
    return sig


def _get_signatures_and_docstring(obj) -> Tuple[List[Signature], List[str]]:
    
    docstring: Optional[str] = obj.__doc__
    
    if docstring is None or docstring.strip() == "":
        print("Docstring missing for method %s" % obj.__qualname__, file=sys.stderr)
        return ([], "")
    
    docstrings = docstring.split("\n")
    
    signatures: List[Signature] = []
    
    while len(docstrings) > 0:
        first_line = docstrings[0]
        docstrings = docstrings[1:]
        
        sig = first_line
        
        if sig.strip() == "":
            print("Signature missing in first lines of docstring for method %s" %
                  obj.__qualname__, file=sys.stderr)
            break
        
        if sig[-1] == "\\":
            sig = sig[0:-1]
        
        try:
            signature = _parse_signature_from_first_line(obj, sig)
            signatures.append(signature)
        except ValueError as ex:
            if len(signatures) == 0:
                # add an empty signature
                if _ismethod(obj):  # bound method
                    signatures.append(
                        Signature(
                            obj.__name__, 
                            [Arg("self"), Arg("*args"), Arg("**kwds")], 
                            None
                        )
                    )
                else:
                    signatures.append(Signature(obj.__name__, [], None))
            break
        
        if first_line[-1] != "\\":  # no backslash at the end, all signatures found
            break
    
    # remove the first empty lines from the remaining docstring
    while len(docstrings) > 0 and docstrings[0].strip() == "":
        docstrings = docstrings[1:]
            
    return (signatures, docstrings)


def _parse_meth_docstring(obj, indent: int):
    prefix = " " * indent
    prefix2 = " " * (indent + 4)
    name = obj.__name__
    
    signatures, docstrings = _get_signatures_and_docstring(obj)
    
    # parse dochstring + body
    docstring = ""
    if len(docstrings) == 1:
        docstring += prefix2 + "\"\"\"%s\"\"\"\n" % docstrings[0]
    elif len(docstrings) > 1:
        docstring += (prefix2 + "\"\"\"\n" + prefix2 + docstrings[0] + "\n")
        for d in docstrings[1:-1]:
            docstring += (prefix2 + d + "\n")
        docstring += (prefix2 + docstrings[-1] + "\n" + prefix2 + "\"\"\"\n")
    
    docstring += prefix2 + "pass"
    
    texts: List[str] = []
    overload_text = ""
    if len(signatures) > 1:
        overload_text = prefix + "@overload\n"
    
    for signature in signatures:
        text = overload_text
        text += prefix + signature.tostring() + ":\n"
        text += docstring
        texts.append(text)
    
    return "\n\n".join(texts)


def _parse_staticmeth_docstring(obj, indent: int):
    prefix = " " * indent
    prefix2 = " " * (indent + 4)
    name = obj.__name__
    
    signatures, docstrings = _get_signatures_and_docstring(obj)
    
    # parse dochstring + body
    docstring = ""
    if len(docstrings) == 1:
        docstring += prefix2 + "\"\"\"%s\"\"\"\n" % docstrings[0]
    elif len(docstrings) > 1:
        docstring += (prefix2 + "\"\"\"\n" + prefix2 + docstrings[0] + "\n")
        for d in docstrings[1:-1]:
            docstring += (prefix2 + d + "\n")
        docstring += (prefix2 + docstrings[-1] + "\n" + prefix2 + "\"\"\"\n")
    
    docstring += prefix2 + "pass"
    
    texts: List[str] = []
    overload_text = ""
    if len(signatures) > 1:
        overload_text = prefix + "@overload\n"
    
    for signature in signatures:
        text = overload_text
        text += prefix + "@staticmethod\n"
        text += prefix + signature.tostring() + ":\n"
        text += docstring
        texts.append(text)
    
    return "\n\n".join(texts)


def _nptype2typing(nptypestr: str) -> str:
    """tries to convert a nptypestr to a valid typestring with the typing module.
    
    E.g. `list of int` should become List[int]."""
    nptypestr = nptypestr.strip()  # trim spaces etc.
    
    # remove any references like :obj:`...` etc.
    def remref(match):
        """from :py:obj:`str` return str."""
        return match.group(3)
    
    def removeRefs(type_str: str) -> str:
        return re.sub(r"((\:[\w\.]+)+\:)`([\w\.]+)`", remref, type_str)
    
    # Check if alternative types are specified with 'or'
    alternatives = re.split(r"\bor\b", nptypestr)
    
    def parseval(val: str) -> str:
        comps = re.split(r"\bof\b", val.strip())
        if len(comps) == 1:
            # remove any references like :obj:`...` etc.
            type_str: str = removeRefs(comps[0])
            return type_str
        else:
            comps[1] = parseval(" of ".join(comps[1:]))
            comps[0] = removeRefs(comps[0].strip())
            # turn first letter into upper case
            comps[0] = comps[0][0].upper() + comps[0][1:]
            return "%s[%s]" % (comps[0], comps[1])
            
    alternatives = [parseval(a) for a in alternatives if a.strip() != ""]
    
    has_none: bool = False
    try:
        none_idx = alternatives.index("None")
    except ValueError:
        none_idx = -1
    if none_idx >= 0:
        has_none = True
        del alternatives[none_idx]
    
    type_str: str
    
    if len(alternatives) == 0:
        if has_none:
            return "None"
        else:
            return nptypestr  # any kind of conversion failed
    elif len(alternatives) == 1:
        if has_none:
            return "Optional[%s]" % alternatives[0]
        else:
            return alternatives[0]
    else:
        type_str: str = "Union[%s]" % ", ".join(alternatives)
        if has_none:
            type_str = "Optional[%s]" % type_str
        return type_str
    
    return nptypestr


def _parse_property_docstring(obj, indent: int):
    prefix = " " * indent
    prefix2 = " " * (indent + 4)
    
    if type(obj) is property:
        if obj.fget is not None:
            name: str = obj.fget.__name__
        elif obj.fset is not None:
            name: str = obj.fset.__name__
        else:
            name = "unknown"
    else:
        name: str = obj.__name__
    docstring: Optional[str] = obj.__doc__
    
    if docstring is None:
        print("Docstring missing for property %s" % obj.__qualname__, file=sys.stderr)
        text = "%s@property\n" % prefix
        text += "%sdef %s(self):\n%s    pass" % (prefix, name, prefix)
        return text
    
    docstrings = docstring.split("\n")
    
    # see https://sphinxcontrib-napoleon.readthedocs.io/en/latest/example_numpy.html
    # the return value of a property can be given in the docstring as:
    # str: docstring of the property
    search_str: str = ": "
    colon_idx: int = docstrings[0].find(search_str)
    
    if colon_idx == -1:
        print("Rettype missing in docstring of property %s" % obj.__qualname__,
              file=sys.stderr)
        rettype: str = ""
    else:
        
        rettype: str = " -> " + _nptype2typing(docstrings[0][0:colon_idx])
        docstrings[0] = docstrings[0][colon_idx + len(search_str):].strip()
    
    text = "%s@property\n" % prefix
    text += prefix + "def %s(self)%s:\n" % (name, rettype)
    
    while len(docstrings) > 0 and docstrings[0].strip() == "":
        docstrings = docstrings[1:]  # remove the first empty lines
    
    if len(docstrings) == 1:
        text += prefix2 + "\"\"\"%s\"\"\"\n" % docstrings[0]
    elif len(docstrings) > 1:
        text += (prefix2 + "\"\"\"\n" + prefix2 + docstrings[0] + "\n")
        for d in docstrings[1:-1]:
            text += (prefix2 + d + "\n")
        text += (prefix2 + docstrings[-1] + "\n" + prefix2 + "\"\"\"\n")
    
    text += prefix2 + "pass"
    
    return text


if __name__ == "__main__":
    parse_stubs()