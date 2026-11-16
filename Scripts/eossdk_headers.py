#!/usr/bin/env python3
"""Reads the EOSPRINT API headers in Libraries/EOSSDK/<version>/include/EosprintApi.

  compare   Writes a JSON and an HTML report of the differences between two header versions,
            and between the headers and Lib3MFToolpath4EOS's hand-written declarations.
  generate  Writes Lib3MFToolpath4EOS's declarations for one version into Implementation/SDK:
            libeos_eossdk_decl_<v>.hpp   types, enums, structs, callbacks and entry point table
            libeos_eossdk_decl_<v>.cpp   entry point loading
            libeos_eossdk_layout_<v>.cpp compile-time checks against the vendor headers

Only declarations are taken from the headers; comments are not carried over.
Layouts are computed for Windows x64 (MSVC): pointers 8 bytes, wchar_t 2 bytes, enums 4 bytes.

Usage:
  python Scripts/eossdk_headers.py compare 2.18 2.26
  python Scripts/eossdk_headers.py generate 2.18
"""

import argparse
import ast
import html
import json
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
SDK_DIR = REPO / "Libraries" / "EOSSDK"
IMPL_SDK_DIR = REPO / "Implementation" / "SDK"
LIBEOS_DECL_FILE = REPO / "Implementation" / "libeos_eossdk.hpp"

# Include order of the headers, so that declarations are emitted before they are used.
HEADER_ORDER = ["Types.h", "ToolpathTypes.h", "Constants.h", "EventIds.h", "EosprintApi.h", "ToolpathApi.h"]

BASE_TYPES = {
    "int8_t": (1, 1), "uint8_t": (1, 1), "char": (1, 1), "bool": (1, 1),
    "int16_t": (2, 2), "uint16_t": (2, 2), "short": (2, 2), "wchar_t": (2, 2),
    "int32_t": (4, 4), "uint32_t": (4, 4), "int": (4, 4), "unsigned": (4, 4), "float": (4, 4), "long": (4, 4),
    "int64_t": (8, 8), "uint64_t": (8, 8), "double": (8, 8), "size_t": (8, 8),
}
POINTER = (8, 8)
ENUM = (4, 4)


# ----------------------------------------------------------------------------------------------
# Parsing
# ----------------------------------------------------------------------------------------------

def strip_comments(text):
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c == '"' or (c == 'L' and i + 1 < n and text[i + 1] == '"'):
            j = i + (2 if c == 'L' else 1)
            while j < n and text[j] != '"':
                j += 2 if text[j] == '\\' else 1
            out.append(text[i:j + 1])
            i = j + 1
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("\n" * text.count("\n", i, j))
            i = j
        elif text.startswith("//", i):
            j = text.find("\n", i)
            i = n if j < 0 else j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def remove_preprocessor(text, empty_macros):
    lines = []
    pending = ""
    for line in text.split("\n"):
        line = pending + line
        pending = ""
        if line.rstrip().endswith("\\"):
            pending = line.rstrip()[:-1]
            continue
        stripped = line.strip()
        if stripped.startswith("#"):
            m = re.match(r"#\s*define\s+(\w+)\s*$", stripped)
            if m:
                empty_macros.add(m.group(1))
            lines.append("")
        else:
            lines.append(line)
    return "\n".join(lines)


def split_statements(text):
    """Splits top-level statements at ';', skipping the braces of extern "C" blocks."""
    text = re.sub(r'extern\s+"C"\s*\{', " ", text)
    statements = []
    current = []
    depth = 0
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c == '"':
            j = i + 1
            while j < n and text[j] != '"':
                j += 2 if text[j] == '\\' else 1
            current.append(text[i:j + 1])
            i = j + 1
            continue
        if c == "{":
            depth += 1
        elif c == "}":
            if depth == 0:
                i += 1
                continue
            depth -= 1
        if c == ";" and depth == 0:
            statement = " ".join("".join(current).split())
            if statement:
                statements.append(statement)
            current = []
        else:
            current.append(c)
        i += 1
    return statements


def normalize_type(t):
    t = " ".join(t.replace("*", " * ").split())
    return t.replace("* *", "**")


def split_top_level(body, sep):
    parts, depth, cur = [], 0, []
    for c in body:
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        if c == sep and depth == 0:
            parts.append("".join(cur))
            cur = []
        else:
            cur.append(c)
    parts.append("".join(cur))
    return [p.strip() for p in parts if p.strip()]


class SafeIntEvaluator:
    """Evaluates integer constant expressions without eval: literals, known names and basic operators."""

    OPS = {
        ast.Add: lambda a, b: a + b, ast.Sub: lambda a, b: a - b, ast.Mult: lambda a, b: a * b,
        ast.LShift: lambda a, b: a << b, ast.RShift: lambda a, b: a >> b,
        ast.BitOr: lambda a, b: a | b, ast.BitAnd: lambda a, b: a & b, ast.BitXor: lambda a, b: a ^ b,
    }

    def __init__(self, names):
        self.names = names

    def evaluate(self, expr):
        expr = re.sub(r"\b(0[xX][0-9a-fA-F]+|\d+)[uUlL]+\b", r"\1", expr.strip())
        expr = re.sub(r"\(\s*\w+\s*\)\s*(?=[\w(-])", "", expr)  # drop C casts such as (EOS_UINT)
        try:
            tree = ast.parse(expr, mode="eval")
        except SyntaxError:
            return None
        return self._node(tree.body)

    def _node(self, node):
        if isinstance(node, ast.Constant) and isinstance(node.value, int):
            return node.value
        if isinstance(node, ast.Name):
            return self.names.get(node.id)
        if isinstance(node, ast.UnaryOp) and isinstance(node.op, (ast.USub, ast.Invert, ast.UAdd)):
            v = self._node(node.operand)
            if v is None:
                return None
            return -v if isinstance(node.op, ast.USub) else (~v if isinstance(node.op, ast.Invert) else v)
        if isinstance(node, ast.BinOp) and type(node.op) in self.OPS:
            a, b = self._node(node.left), self._node(node.right)
            if a is None or b is None:
                return None
            return self.OPS[type(node.op)](a, b)
        return None


class HeaderSet:
    """All declarations of one header version (or of Lib3MFToolpath4EOS's own declaration file)."""

    def __init__(self, label):
        self.label = label
        self.order = []          # (kind, name) in declaration order
        self.scalars = {}        # name -> type
        self.enums = {}          # name -> [(entry, value)]
        self.records = {}        # name -> {"kind": struct|union, "members": [(type, name, array)]}
        self.callbacks = {}      # name -> (return, [params])
        self.functions = {}      # name -> (return, [params], api macro)
        self.constants = {}      # name -> (type, value text, scalar?)
        self.enum_values = {}    # entry -> value

    # -- declarations ---------------------------------------------------------------------------

    def add(self, kind, name, value):
        table = {"scalar": self.scalars, "enum": self.enums, "record": self.records, "callback": self.callbacks,
                 "function": self.functions, "constant": self.constants}[kind]
        if name not in table:
            self.order.append((kind, name))
        table[name] = value

    def parse_statement(self, s, empty_macros):
        for macro in empty_macros:
            s = re.sub(r"\b%s\b" % re.escape(macro), " ", s)
        s = " ".join(s.split())

        m = re.match(r"^typedef (struct|union)\s*\w*\s*\{(.*)\}\s*(\w+)$", s)
        if m:
            self.add("record", m.group(3), {"kind": m.group(1), "members": self.parse_members(m.group(2))})
            return
        m = re.match(r"^typedef enum\s*\w*\s*\{(.*)\}\s*(\w+)$", s)
        if m:
            self.add("enum", m.group(2), self.parse_enum(m.group(1)))
            return
        m = re.match(r"^enum (\w+)\s*\{(.*)\}$", s)
        if m:
            self.add("enum", m.group(1), self.parse_enum(m.group(2)))
            return
        m = re.match(r"^typedef ([\w\s\*]+?)\s*\(\s*\*\s*(\w+)\s*\)\s*\((.*)\)$", s)
        if m:
            self.add("callback", m.group(2), (normalize_type(m.group(1)), self.parse_params(m.group(3))))
            return
        m = re.match(r"^typedef ([\w\s\*]+?)\s*\b(\w+)$", s)
        if m:
            self.add("scalar", m.group(2), normalize_type(m.group(1)))
            return
        m = re.match(r"^(EOS_\w+_API) ([\w\s\*]+?)\s*\b(\w+)\s*\((.*)\)$", s)
        if m:
            self.add("function", m.group(3), (normalize_type(m.group(2)), self.parse_params(m.group(4)), m.group(1)))
            return
        m = re.match(r"^static ([\w\s\*]+?)\s+const\s+(\w+)\s*(\[[^\]]*\])?\s*=\s*(.*)$", s)
        if m:
            scalar = m.group(3) is None and not m.group(4).startswith("{")
            self.add("constant", m.group(2), (normalize_type(m.group(1)), m.group(4).strip(), scalar))
            return

    def parse_members(self, body):
        members = []
        for decl in split_top_level(body, ";"):
            declarators = split_top_level(decl, ",")
            first = re.match(r"^(.*?)(\w+)\s*(\[[^\]]*\])?$", declarators[0])
            if not first:
                raise ValueError("cannot parse member: " + decl)
            base = normalize_type(first.group(1))
            members.append((base, first.group(2), (first.group(3) or "")[1:-1].strip()))
            base_no_ptr = normalize_type(base.replace("*", ""))
            for extra in declarators[1:]:
                m = re.match(r"^(\**)\s*(\w+)\s*(\[[^\]]*\])?$", extra)
                members.append((normalize_type(base_no_ptr + " " + m.group(1)), m.group(2), (m.group(3) or "")[1:-1].strip()))
        return members

    def parse_enum(self, body):
        entries = []
        evaluator = SafeIntEvaluator(self.enum_values)
        last = -1
        for item in split_top_level(body, ","):
            m = re.match(r"^(\w+)\s*(?:=\s*(.+))?$", item)
            if not m:
                raise ValueError("cannot parse enum entry: " + item)
            if m.group(2) is not None:
                value = evaluator.evaluate(m.group(2))
                if value is None:
                    raise ValueError("cannot evaluate enum value: " + item)
            else:
                value = last + 1
            last = value
            self.enum_values[m.group(1)] = value
            entries.append((m.group(1), value))
        return entries

    @staticmethod
    def parse_params(text):
        text = text.strip()
        if text in ("", "void"):
            return []
        params = []
        for p in split_top_level(text, ","):
            m = re.match(r"^(.*?)\b(\w+)\s*(\[[^\]]*\])?$", p)
            if m and m.group(1).strip():
                ptype = normalize_type(m.group(1) + (" *" if m.group(3) is not None else ""))
                params.append((ptype, m.group(2)))
            else:
                params.append((normalize_type(p), ""))
        return params

    # -- layout -----------------------------------------------------------------------------------

    def resolve(self, t, seen=None):
        """Returns the underlying type of a type string: base name, 'enum', record name or 'pointer'."""
        if "*" in t:
            return "pointer"
        words = [w for w in t.split() if w not in ("const", "volatile", "struct", "union", "enum", "signed")]
        if words == ["unsigned", "int"] or words == ["unsigned"]:
            return "uint32_t"
        name = words[-1] if words else t
        seen = seen or set()
        if name in BASE_TYPES:
            return name
        if name in self.enums:
            return "enum"
        if name in self.records:
            return name
        if name in self.scalars and name not in seen:
            seen.add(name)
            return self.resolve(self.scalars[name], seen)
        return "?" + name

    def array_count(self, text):
        if not text:
            return 1
        names = dict(self.enum_values)
        for cname, (_, value, scalar) in self.constants.items():
            if scalar:
                v = SafeIntEvaluator(names).evaluate(value)
                if v is not None:
                    names[cname] = v
        v = SafeIntEvaluator(names).evaluate(text)
        if v is None:
            raise ValueError("cannot evaluate array size: " + text)
        return v

    def size_align(self, t):
        r = self.resolve(t)
        if r == "pointer":
            return POINTER
        if r == "enum":
            return ENUM
        if r in BASE_TYPES:
            return BASE_TYPES[r]
        if r in self.records:
            layout = self.layout(r)
            return layout["size"], layout["align"]
        raise ValueError("%s: unknown type %s" % (self.label, t))

    def layout(self, name):
        rec = self.records[name]
        offset, size, align = 0, 0, 1
        members = []
        for mtype, mname, arr in rec["members"]:
            msize, malign = self.size_align(mtype)
            total = msize * self.array_count(arr)
            align = max(align, malign)
            if rec["kind"] == "union":
                members.append({"name": mname, "type": mtype, "array": arr, "offset": 0, "size": total})
                size = max(size, total)
            else:
                offset = (offset + malign - 1) // malign * malign
                members.append({"name": mname, "type": mtype, "array": arr, "offset": offset, "size": total})
                offset += total
                size = offset
        size = (size + align - 1) // align * align
        return {"kind": rec["kind"], "size": size, "align": align, "members": members}


def load_version(version):
    folder = SDK_DIR / version / "include" / "EosprintApi"
    if not folder.is_dir():
        raise SystemExit("header folder not found: %s" % folder)
    hs = HeaderSet(version)
    empty_macros = set()
    files = sorted(folder.glob("*.h"), key=lambda p: HEADER_ORDER.index(p.name) if p.name in HEADER_ORDER else 99)
    for path in files:
        text = remove_preprocessor(strip_comments(path.read_text(encoding="utf-8-sig", errors="replace")), empty_macros)
        for statement in split_statements(text):
            hs.parse_statement(statement, empty_macros)
    return hs


def load_libeos_declarations():
    """Reads the typedefs of Lib3MFToolpath4EOS's current hand-written declarations."""
    hs = HeaderSet("Lib3MFToolpath4EOS")
    text = strip_comments(LIBEOS_DECL_FILE.read_text(encoding="utf-8-sig", errors="replace"))
    text = remove_preprocessor(text, set())
    patterns = [
        r"typedef\s+(?:struct|union)\s*\w*\s*\{[^{}]*\}\s*\w+\s*;",
        r"typedef\s+enum\s*\w*\s*\{[^{}]*\}\s*\w+\s*;",
        r"typedef\s+[\w\s\*]+?\(\s*\*\s*\w+\s*\)\s*\([^;]*\)\s*;",
        r"typedef\s+[\w\s\*]+?\s+\w+\s*;",
    ]
    found = []
    for pattern in patterns:
        for m in re.finditer(pattern, text):
            if not any(a <= m.start() < b for a, b, _ in found):
                found.append((m.start(), m.end(), m.group(0)))
    for _, _, statement in sorted(found):
        hs.parse_statement(statement.rstrip(";"), set())
    for name in [n for n in hs.callbacks if n.startswith("P")]:
        ret, params = hs.callbacks.pop(name)
        hs.order.remove(("callback", name))
        hs.add("function", name[1:], (ret, params, "Lib3MFToolpath4EOS"))
    return hs


# ----------------------------------------------------------------------------------------------
# Comparison
# ----------------------------------------------------------------------------------------------

def signature_types(sig):
    ret, params = sig[0], sig[1]
    return ret, [p[0] for p in params]


def compare_sets(old, new, by_position=False):
    """Differences between two header sets. by_position compares record members by index (for Lib3MFToolpath4EOS)."""
    result = {"functions": {"added": [], "removed": [], "changed": [], "renamed_parameters": []},
              "callbacks": {"added": [], "removed": [], "changed": []},
              "records": {"added": [], "removed": [], "changed": []},
              "enums": {"added": [], "removed": [], "changed": []},
              "scalars": {"added": [], "removed": [], "changed": []},
              "constants": {"added": [], "removed": [], "changed": []}}

    def resolved_sig(hs, sig):
        ret, types = signature_types(sig)
        return hs.resolve(ret) if ret != "void" else "void", [hs.resolve(t) for t in types]

    old_funcs = {k.lower(): k for k in old.functions}
    new_funcs = {k.lower(): k for k in new.functions}
    for key, name in sorted(new_funcs.items()):
        if key not in old_funcs:
            result["functions"]["added"].append({"name": name, "signature": format_signature(name, new.functions[name])})
    for key, name in sorted(old_funcs.items()):
        if key not in new_funcs:
            result["functions"]["removed"].append({"name": name, "signature": format_signature(name, old.functions[name])})
            continue
        oname, nname = name, new_funcs[key]
        osig, nsig = old.functions[oname], new.functions[nname]
        same_types = resolved_sig(old, osig) == resolved_sig(new, nsig) if by_position else signature_types(osig) == signature_types(nsig)
        if not same_types:
            result["functions"]["changed"].append({"name": nname, "old": format_signature(oname, osig), "new": format_signature(nname, nsig)})
        elif [p[1] for p in osig[1]] != [p[1] for p in nsig[1]] and not by_position:
            result["functions"]["renamed_parameters"].append({"name": nname, "old": format_signature(oname, osig), "new": format_signature(nname, nsig)})

    for name in sorted(set(new.callbacks) - set(old.callbacks)):
        result["callbacks"]["added"].append({"name": name, "signature": format_signature(name, new.callbacks[name])})
    for name in sorted(set(old.callbacks) - set(new.callbacks)):
        result["callbacks"]["removed"].append({"name": name, "signature": format_signature(name, old.callbacks[name])})
    for name in sorted(set(old.callbacks) & set(new.callbacks)):
        a = resolved_sig(old, old.callbacks[name]) if by_position else signature_types(old.callbacks[name])
        b = resolved_sig(new, new.callbacks[name]) if by_position else signature_types(new.callbacks[name])
        if a != b:
            result["callbacks"]["changed"].append({"name": name, "old": format_signature(name, old.callbacks[name]), "new": format_signature(name, new.callbacks[name])})

    for name in sorted(set(new.records) - set(old.records)):
        result["records"]["added"].append({"name": name, "layout": new.layout(name)})
    for name in sorted(set(old.records) - set(new.records)):
        result["records"]["removed"].append({"name": name, "layout": old.layout(name)})
    for name in sorted(set(old.records) & set(new.records)):
        lo, ln = old.layout(name), new.layout(name)
        if by_position:
            def key(hs, lay):
                return [(hs.resolve(m["type"]), m["size"], m["offset"]) for m in lay["members"]]
            differs = key(old, lo) != key(new, ln) or lo["size"] != ln["size"]
        else:
            def key2(lay):
                return [(m["type"], m["name"], m["array"]) for m in lay["members"]]
            differs = key2(lo) != key2(ln) or lo["size"] != ln["size"]
        if differs:
            result["records"]["changed"].append({"name": name, "old": lo, "new": ln, "size_changed": lo["size"] != ln["size"]})

    for name in sorted(set(new.enums) - set(old.enums)):
        result["enums"]["added"].append({"name": name, "entries": new.enums[name]})
    for name in sorted(set(old.enums) - set(new.enums)):
        result["enums"]["removed"].append({"name": name, "entries": old.enums[name]})
    for name in sorted(set(old.enums) & set(new.enums)):
        a, b = dict(old.enums[name]), dict(new.enums[name])
        if a != b:
            result["enums"]["changed"].append({
                "name": name,
                "added": sorted([(k, v) for k, v in b.items() if k not in a], key=lambda e: e[1]),
                "removed": sorted([(k, v) for k, v in a.items() if k not in b], key=lambda e: e[1]),
                "renumbered": sorted([(k, a[k], b[k]) for k in a if k in b and a[k] != b[k]], key=lambda e: e[2])})

    for name in sorted(set(new.scalars) - set(old.scalars)):
        result["scalars"]["added"].append({"name": name, "type": new.scalars[name]})
    for name in sorted(set(old.scalars) - set(new.scalars)):
        result["scalars"]["removed"].append({"name": name, "type": old.scalars[name]})
    for name in sorted(set(old.scalars) & set(new.scalars)):
        if old.resolve(name) != new.resolve(name):
            result["scalars"]["changed"].append({"name": name, "old": "%s (%s)" % (old.scalars[name], old.resolve(name)),
                                                 "new": "%s (%s)" % (new.scalars[name], new.resolve(name))})

    if not by_position:
        for name in sorted(set(new.constants) - set(old.constants)):
            result["constants"]["added"].append({"name": name, "value": short_value(new.constants[name])})
        for name in sorted(set(old.constants) - set(new.constants)):
            result["constants"]["removed"].append({"name": name, "value": short_value(old.constants[name])})
        for name in sorted(set(old.constants) & set(new.constants)):
            if old.constants[name] != new.constants[name]:
                result["constants"]["changed"].append({"name": name, "old": short_value(old.constants[name]), "new": short_value(new.constants[name])})
    return result


def short_value(c):
    ctype, value, scalar = c
    return "%s = %s" % (ctype, value) if scalar else "%s (aggregate)" % ctype


def format_signature(name, sig):
    ret, params = sig[0], sig[1]
    return "%s %s(%s)" % (ret, name, ", ".join(("%s %s" % p).strip() for p in params))


def libeos_vs_headers(libeos, hs):
    """Differences between Lib3MFToolpath4EOS's declarations and one header version, limited to what Lib3MFToolpath4EOS declares."""
    diff = compare_sets(hs, libeos, by_position=True)
    diff["functions"]["missing_in_libeos"] = [f["name"] for f in diff["functions"].pop("removed")]
    diff["functions"]["unknown_to_headers"] = [f["name"] for f in diff["functions"].pop("added")]
    for kind in ("records", "enums", "scalars", "callbacks"):
        diff[kind]["unknown_to_headers"] = [e["name"] for e in diff[kind].pop("added")]
        diff[kind]["missing_in_libeos"] = [e["name"] for e in diff[kind].pop("removed")]
    for e in diff["enums"]["changed"]:
        e["missing_in_libeos"] = e.pop("removed")
        e["unknown_to_headers"] = e.pop("added")
    diff.pop("constants")
    return diff


# ----------------------------------------------------------------------------------------------
# HTML report
# ----------------------------------------------------------------------------------------------

def esc(s):
    return html.escape(str(s))


def code(s):
    return "<code>%s</code>" % esc(s)


def member_text(m):
    return "%s %s%s" % (m["type"], m["name"], "[%s]" % m["array"] if m["array"] else "")


def html_record_change(change, old_label, new_label):
    lo, ln = change["old"], change["new"]
    rows = []
    old_by_name = {m["name"]: m for m in lo["members"]}
    new_by_name = {m["name"]: m for m in ln["members"]}
    names = [m["name"] for m in lo["members"]]
    for m in ln["members"]:
        if m["name"] not in old_by_name:
            names.append(m["name"])
    for n in names:
        a, b = old_by_name.get(n), new_by_name.get(n)
        status = ""
        if a and not b:
            status = "removed"
        elif b and not a:
            status = "added"
        elif (a["type"], a["array"]) != (b["type"], b["array"]):
            status = "type"
        elif a["offset"] != b["offset"]:
            status = "moved"
        cls = ' class="diff"' if status else ""
        rows.append("<tr%s><td>%s</td><td>%s</td><td class=\"num\">%s</td><td>%s</td><td class=\"num\">%s</td><td>%s</td></tr>" % (
            cls, code(n), code(member_text(a)) if a else "", a["offset"] if a else "",
            code(member_text(b)) if b else "", b["offset"] if b else "", status))
    size_note = '<span class="tag brk">size %d &rarr; %d</span>' % (lo["size"], ln["size"]) if lo["size"] != ln["size"] else '<span class="tag same">size %d unchanged</span>' % lo["size"]
    return ("<h4>%s %s</h4>\n<table class=\"layout\"><tr><th>Member</th><th>%s</th><th>Offset</th><th>%s</th><th>Offset</th><th></th></tr>\n%s\n</table>\n"
            % (code(change["name"]), size_note, esc(old_label), esc(new_label), "\n".join(rows)))


def html_list(items, fmt):
    if not items:
        return "<p class=\"small\">None.</p>\n"
    return "<ul class=\"tight\">\n%s\n</ul>\n" % "\n".join("<li>%s</li>" % fmt(i) for i in items)


def html_version_diff(diff, a, b):
    out = []
    f = diff["functions"]
    out.append("<h3>Functions</h3>")
    out.append("<p>Added in %s:</p>" % esc(b))
    out.append(html_list(f["added"], lambda e: code(e["signature"])))
    out.append("<p>Removed in %s:</p>" % esc(b))
    out.append(html_list(f["removed"], lambda e: code(e["signature"])))
    out.append("<p>Parameter types changed:</p>")
    out.append(html_list(f["changed"], lambda e: "%s<br>%s: %s<br>%s: %s" % (code(e["name"]), esc(a), code(e["old"]), esc(b), code(e["new"]))))
    out.append("<p>Only parameter names changed (no effect on the binary interface):</p>")
    out.append(html_list(f["renamed_parameters"], lambda e: code(e["name"])))

    r = diff["records"]
    out.append("<h3>Structures and unions</h3>")
    changed_size = [c for c in r["changed"] if c["size_changed"]]
    same_size = [c for c in r["changed"] if not c["size_changed"]]
    out.append("<p>%d changed, %d of them in size. Added in %s: %s. Removed in %s: %s.</p>" % (
        len(r["changed"]), len(changed_size), esc(b),
        ", ".join(code(e["name"]) for e in r["added"]) or "none", esc(b),
        ", ".join(code(e["name"]) for e in r["removed"]) or "none"))
    for c in changed_size + same_size:
        out.append(html_record_change(c, a, b))
    if r["added"]:
        out.append("<p>New structures in %s:</p>" % esc(b))
        out.append(html_list(r["added"], lambda e: "%s (%s, %d bytes): %s" % (
            code(e["name"]), e["layout"]["kind"], e["layout"]["size"], ", ".join(code(member_text(m)) for m in e["layout"]["members"]))))

    e = diff["enums"]
    out.append("<h3>Enums</h3>")
    out.append("<table><tr><th>Enum</th><th>Added in %s</th><th>Removed in %s</th><th>Renumbered</th></tr>" % (esc(b), esc(b)))
    for c in e["changed"]:
        out.append("<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>" % (
            code(c["name"]),
            "<br>".join("%s = %s" % (code(k), v) for k, v in c["added"]),
            "<br>".join("%s = %s" % (code(k), v) for k, v in c["removed"]),
            "<br>".join("%s: %s &rarr; %s" % (code(k), x, y) for k, x, y in c["renumbered"])))
    out.append("</table>")
    out.append("<p>New enums in %s: %s. Removed: %s.</p>" % (
        esc(b), ", ".join(code(x["name"]) for x in e["added"]) or "none", ", ".join(code(x["name"]) for x in e["removed"]) or "none"))

    out.append("<h3>Scalar types, callbacks and constants</h3>")
    s = diff["scalars"]
    out.append("<p>Scalar types added: %s. Removed: %s. Changed: %s.</p>" % (
        ", ".join(code(x["name"]) for x in s["added"]) or "none",
        ", ".join(code(x["name"]) for x in s["removed"]) or "none",
        "; ".join("%s %s &rarr; %s" % (code(x["name"]), esc(x["old"]), esc(x["new"])) for x in s["changed"]) or "none"))
    cb = diff["callbacks"]
    out.append("<p>Callbacks added: %s. Removed: %s. Changed: %s.</p>" % (
        ", ".join(code(x["name"]) for x in cb["added"]) or "none",
        ", ".join(code(x["name"]) for x in cb["removed"]) or "none",
        ", ".join(code(x["name"]) for x in cb["changed"]) or "none"))
    k = diff["constants"]
    out.append("<p>Constants added: %d, removed: %d, changed: %d.</p>" % (len(k["added"]), len(k["removed"]), len(k["changed"])))
    rows = [("added", x["name"], "", x["value"]) for x in k["added"]] + [("removed", x["name"], x["value"], "") for x in k["removed"]] + \
           [("changed", x["name"], x["old"], x["new"]) for x in k["changed"]]
    if rows:
        out.append("<table><tr><th>Constant</th><th></th><th>%s</th><th>%s</th></tr>" % (esc(a), esc(b)))
        for status, name, old, new in rows:
            out.append("<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>" % (code(name), status, code(old) if old else "", code(new) if new else ""))
        out.append("</table>")
    return "\n".join(out)


def html_libeos_diff(diff, version):
    out = []
    f = diff["functions"]
    out.append("<h3>Functions</h3>")
    out.append("<p>Lib3MFToolpath4EOS declares %d entry points that differ in their types from %s:</p>" % (len(f["changed"]), esc(version)))
    out.append(html_list(f["changed"], lambda e: "%s<br>%s: %s<br>Lib3MFToolpath4EOS: %s" % (code(e["name"]), esc(version), code(e["old"]), code(e["new"]))))
    out.append("<p>Declared by Lib3MFToolpath4EOS but not by the %s headers: %s.</p>" % (esc(version), ", ".join(code(n) for n in f["unknown_to_headers"]) or "none"))
    out.append("<p>Declared by the headers but not by Lib3MFToolpath4EOS: %d functions.</p>" % len(f["missing_in_libeos"]))
    r = diff["records"]
    out.append("<h3>Structures</h3>")
    out.append("<p>%d of Lib3MFToolpath4EOS's structures differ from %s in size or member layout:</p>" % (len(r["changed"]), esc(version)))
    for c in r["changed"]:
        out.append(html_record_change(c, version, "Lib3MFToolpath4EOS"))
    out.append("<p>Structures Lib3MFToolpath4EOS lacks: %s.</p>" % (", ".join(code(n) for n in r["missing_in_libeos"]) or "none"))
    out.append("<p>Structures Lib3MFToolpath4EOS declares that the headers don't: %s.</p>" % (", ".join(code(n) for n in r["unknown_to_headers"]) or "none"))
    e = diff["enums"]
    out.append("<h3>Enums</h3>")
    out.append("<table><tr><th>Enum</th><th>Missing in Lib3MFToolpath4EOS</th><th>Unknown to %s</th><th>Different value</th></tr>" % esc(version))
    for c in e["changed"]:
        out.append("<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>" % (
            code(c["name"]),
            "<br>".join("%s = %s" % (code(k), v) for k, v in c["missing_in_libeos"]),
            "<br>".join("%s = %s" % (code(k), v) for k, v in c["unknown_to_headers"]),
            "<br>".join("%s: %s in %s, %s in Lib3MFToolpath4EOS" % (code(k), x, esc(version), y) for k, x, y in c["renumbered"])))
    out.append("</table>")
    s = diff["scalars"]
    out.append("<h3>Scalar types and callbacks</h3>")
    out.append(html_list(s["changed"], lambda x: "%s: %s %s, Lib3MFToolpath4EOS %s" % (code(x["name"]), esc(version), esc(x["old"]), esc(x["new"]))))
    cb = diff["callbacks"]
    out.append("<p>Callbacks with different types: %s.</p>" % (", ".join(code(x["name"]) for x in cb["changed"]) or "none"))
    return "\n".join(out)


REPORT_HEAD = """<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>Lib3MFToolpath4EOS: EOSPRINT API version differences</title>
<link rel="stylesheet" href="style.css">
<style>
	.tag { display: inline-block; padding: 1px 7px; font-family: var(--mono); font-size: 11.5px; font-weight: 600; border-radius: 2px; }
	.brk { background: #fbe4e2; color: #b3261e; }
	.same { background: #ecebe6; color: #555; }
	tr.diff td { background: #fdf0d8; }
	td.num { text-align: right; font-family: var(--mono); font-size: 13px; }
	table.layout td, table.layout th { padding: 4px 8px; font-size: 13.5px; }
	h4 { margin: 22px 0 6px; font-size: 15px; }
	.small { font-size: 13.5px; color: var(--muted); }
	ul.tight li { margin: 3px 0; }
</style>
</head>
<body>
<header class="site"><a class="brand" href="index.html"><img src="images/3mf-logo.png" alt=".3MF"><span>Lib3MFToolpath4EOS</span></a><nav><a href="index.html">Library guide</a><a href="api.html">API reference</a><a href="examples.html">Examples</a><a href="power-ramps.html">Power ramps</a></nav></header>
<div class="layout">
<aside class="toc">
<h3>Contents</h3>
<ul>
<li><a href="#summary">Summary</a></li>
<li><a href="#handling">Handling in Lib3MFToolpath4EOS</a></li>
<li><a href="#versions">%(a)s to %(b)s</a></li>
<li><a href="#libeos">Lib3MFToolpath4EOS and %(a)s</a></li>
</ul>
</aside>
<main>
"""


def write_report(a, b, diff, libeos_diff, out_html, out_json):
    def count(d, kind, key):
        return len(d[kind][key])
    summary_rows = [
        ("Functions", len(a.functions), len(b.functions), count(diff, "functions", "added"), count(diff, "functions", "removed"), count(diff, "functions", "changed")),
        ("Structures and unions", len(a.records), len(b.records), count(diff, "records", "added"), count(diff, "records", "removed"), count(diff, "records", "changed")),
        ("Enums", len(a.enums), len(b.enums), count(diff, "enums", "added"), count(diff, "enums", "removed"), count(diff, "enums", "changed")),
        ("Callbacks", len(a.callbacks), len(b.callbacks), count(diff, "callbacks", "added"), count(diff, "callbacks", "removed"), count(diff, "callbacks", "changed")),
        ("Scalar types", len(a.scalars), len(b.scalars), count(diff, "scalars", "added"), count(diff, "scalars", "removed"), count(diff, "scalars", "changed")),
        ("Constants", len(a.constants), len(b.constants), count(diff, "constants", "added"), count(diff, "constants", "removed"), count(diff, "constants", "changed")),
    ]
    def moved_members(c):
        old = {m["name"]: m["offset"] for m in c["old"]["members"] if m["name"] != "reserved"}
        return [m["name"] for m in c["new"]["members"] if m["name"] in old and old[m["name"]] != m["offset"]]

    breaking = []
    for c in diff["records"]["changed"]:
        moved = moved_members(c)
        if c["size_changed"] or moved:
            what = []
            if c["size_changed"]:
                what.append("size %d &rarr; %d bytes" % (c["old"]["size"], c["new"]["size"]))
            if moved:
                what.append("members moved: %s" % ", ".join(code(n) for n in moved))
            breaking.append("%s: %s" % (code(c["name"]), "; ".join(what)))
    for c in diff["callbacks"]["changed"]:
        breaking.append("Callback %s: %s &rarr; %s" % (code(c["name"]), code(c["old"]), code(c["new"])))
    for f in diff["functions"]["changed"]:
        breaking.append("Function %s: parameter types changed" % code(f["name"]))
    for f in diff["functions"]["removed"]:
        breaking.append("Function %s removed" % code(f["name"]))
    compatible = [c["name"] for c in diff["records"]["changed"] if not c["size_changed"] and not moved_members(c)]

    parts = [REPORT_HEAD % {"a": esc(a.label), "b": esc(b.label)}]
    parts.append("<h1>EOSPRINT API version differences</h1>")
    parts.append("<p class=\"small\">Generated by <code>Scripts/eossdk_headers.py compare %s %s</code> from the headers in "
                 "<code>Libraries/EOSSDK</code>. Offsets and sizes are computed for Windows x64; the generated layout checks in "
                 "<code>Implementation/SDK</code> verify them against the vendor headers at compile time.</p>" % (esc(a.label), esc(b.label)))
    parts.append("<section id=\"summary\"><h2>Summary</h2>")
    parts.append("<table><tr><th></th><th>%s</th><th>%s</th><th>Added</th><th>Removed</th><th>Changed</th></tr>" % (esc(a.label), esc(b.label)))
    for row in summary_rows:
        parts.append("<tr><td>%s</td>%s</tr>" % (row[0], "".join("<td class=\"num\">%d</td>" % v for v in row[1:])))
    parts.append("</table>")
    parts.append("<p><b>Not binary compatible</b> between %s and %s:</p>" % (esc(a.label), esc(b.label)))
    parts.append(html_list(breaking, lambda x: x))
    parts.append("<p>Changed but compatible (new members in former reserved bytes, renamed types, same offsets): %s.</p>"
                 % (", ".join(code(n) for n in compatible) or "none"))
    parts.append("</section>")
    parts.append(HANDLING_SECTION)
    parts.append("<section id=\"versions\"><h2>%s to %s</h2>" % (esc(a.label), esc(b.label)))
    parts.append(html_version_diff(diff, a.label, b.label))
    parts.append("</section>")
    parts.append("<section id=\"libeos\"><h2>Lib3MFToolpath4EOS's hand-written declarations and %s</h2>" % esc(a.label))
    parts.append("<p>Lib3MFToolpath4EOS's declarations in <code>Implementation/libeos_eossdk.hpp</code> were written without the headers. "
                 "Structures are compared member by member by position, because Lib3MFToolpath4EOS's member names may differ. "
                 "These declarations are replaced by the generated ones.</p>")
    parts.append(html_libeos_diff(libeos_diff, a.label))
    parts.append("</section>\n</main>\n</div>\n</body>\n</html>\n")
    out_html.write_text("\n".join(parts), encoding="utf-8")
    out_json.write_text(json.dumps({"from": a.label, "to": b.label, "versions": diff, "libeos": libeos_diff}, indent=1), encoding="utf-8")


HANDLING_SECTION = """<section id="handling"><h2>Handling in Lib3MFToolpath4EOS</h2>
<table>
<tr><th>Kind of difference</th><th>Handling</th></tr>
<tr><td>Structure with a new size or layout</td><td>Each version has its own declaration in <code>Implementation/SDK/libeos_eossdk_decl_&lt;version&gt;.hpp</code>, generated from its headers. The version adapter converts it to the version-neutral model. The event and build time callbacks receive their structure by value in 2.18 and by pointer in 2.26; each adapter registers its own callback trampoline.</td></tr>
<tr><td>Function added in a later version</td><td>Resolved when the DLL is loaded; missing entry points stay empty. Using one that the loaded version lacks raises <code>SDKFEATURENOTAVAILABLE</code>, or is skipped and written to the conversion log where the feature is optional.</td></tr>
<tr><td>Function removed in a later version</td><td>Not used by Lib3MFToolpath4EOS where it is removed; the adapter reports it as unavailable.</td></tr>
<tr><td>Enum values added or removed</td><td>The neutral enums contain the values of all versions. Adapters reject values the loaded version does not know.</td></tr>
<tr><td>Only parameter names changed</td><td>No action; the binary interface is the same.</td></tr>
</table>
</section>"""


# ----------------------------------------------------------------------------------------------
# Code generation
# ----------------------------------------------------------------------------------------------

LICENSE_HEADER = """/*++

Copyright (C) 2026 3MF Consortium

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the 3MF Consortium nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE 3MF CONSORTIUM BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
"""


def version_tag(version):
    if not re.fullmatch(r"\d+\.\d+", version):
        raise SystemExit("version must look like 2.18")
    return version.replace(".", "_")


def generate(version):
    hs = load_version(version)
    tag = version_tag(version)
    ns = "SDK_" + tag
    guard = "__LIBEOS_EOSSDK_DECL_" + tag
    note = ("// Generated by Scripts/eossdk_headers.py from Libraries/EOSSDK/%s. Do not edit.\n"
            "// Declarations of EOSPRINT API %s, in the namespace Lib3MF4EOS::Impl::%s.\n" % (version, version, ns))

    lines = [LICENSE_HEADER, note, "#ifndef %s\n#define %s\n" % (guard, guard), "#include <cstdint>\n#include <cwchar>\n",
             "namespace Lib3MF4EOS {\nnamespace Impl {\nnamespace %s {\n" % ns]
    for kind, name in hs.order:
        if kind == "scalar":
            lines.append("typedef %s %s;" % (hs.scalars[name], name))
        elif kind == "enum":
            body = ",\n".join("\t%s = %d" % (k, v) for k, v in hs.enums[name])
            lines.append("typedef enum {\n%s\n} %s;\n" % (body, name))
        elif kind == "record":
            rec = hs.records[name]
            body = "\n".join("\t%s %s%s;" % (t, n, "[%s]" % a if a else "") for t, n, a in rec["members"])
            lines.append("typedef %s {\n%s\n} %s;\n" % (rec["kind"], body, name))
        elif kind == "callback":
            ret, params = hs.callbacks[name]
            lines.append("typedef %s (*%s)(%s);" % (ret, name, ", ".join(("%s %s" % p).strip() for p in params) or "void"))
        elif kind == "constant":
            ctype, value, scalar = hs.constants[name]
            if scalar:
                lines.append("static const %s %s = %s;" % (ctype, name, value))
    lines.append("\n// Entry points\n")
    funcs = [n for k, n in hs.order if k == "function"]
    for name in funcs:
        ret, params, _ = hs.functions[name]
        lines.append("typedef %s (*P%s)(%s);" % (ret, name, ", ".join(("%s %s" % p).strip() for p in params) or "void"))
    lines.append("\nstruct sEntryPoints {")
    for name in funcs:
        lines.append("\tP%s %s = nullptr;" % (name, name))
    lines.append("\n\t// Resolves all entry points from a loaded EosprintApi.dll. Entry points the DLL does not export stay nullptr.")
    lines.append("\tvoid load(void* hLibrary);")
    lines.append("\n\t// Names of the entry points this version declares but the loaded DLL does not export.")
    lines.append("\tconst char* const* missingEntryPoints(size_t& nCount) const;")
    lines.append("};\n")
    lines.append("\nconstexpr uint32_t VERSION_MAJOR = %s;\nconstexpr uint32_t VERSION_MINOR = %s;\n" % tuple(version.split(".")))
    lines.append("}\n}\n}\n\n#endif // %s\n" % guard)
    IMPL_SDK_DIR.mkdir(parents=True, exist_ok=True)
    (IMPL_SDK_DIR / ("libeos_eossdk_decl_%s.hpp" % tag)).write_text("\n".join(lines), encoding="utf-8")

    cpp = [LICENSE_HEADER, note, '#include "libeos_eossdk_decl_%s.hpp"\n' % tag,
           "#ifdef _WIN32\n#include <windows.h>\n#endif\n#include <vector>\n",
           "namespace Lib3MF4EOS {\nnamespace Impl {\nnamespace %s {\n" % ns,
           "static const char* const s_EntryPointNames[] = {"]
    cpp.append(",\n".join('\t"%s"' % n for n in funcs))
    cpp.append("};\n")
    cpp.append("void sEntryPoints::load(void* hLibrary)\n{\n#ifdef _WIN32\n\tHMODULE hModule = (HMODULE)hLibrary;")
    for name in funcs:
        cpp.append('\t%s = (P%s)GetProcAddress(hModule, "%s");' % (name, name, name))
    cpp.append("#else\n\t(void)hLibrary;\n#endif\n}\n")
    cpp.append("const char* const* sEntryPoints::missingEntryPoints(size_t& nCount) const\n{\n"
               "\tstatic thread_local std::vector<const char*> missing;\n\tmissing.clear();\n"
               "\tconst void* const pointers[] = {")
    cpp.append(",\n".join("\t\t(const void*)%s" % n for n in funcs))
    cpp.append("\t};\n\tfor (size_t nIndex = 0; nIndex < sizeof(pointers) / sizeof(pointers[0]); nIndex++)\n"
               "\t\tif (pointers[nIndex] == nullptr)\n\t\t\tmissing.push_back(s_EntryPointNames[nIndex]);\n"
               "\tnCount = missing.size();\n\treturn missing.data();\n}\n")
    cpp.append("}\n}\n}\n")
    (IMPL_SDK_DIR / ("libeos_eossdk_decl_%s.cpp" % tag)).write_text("\n".join(cpp), encoding="utf-8")

    rel = "../../Libraries/EOSSDK/%s/include/EosprintApi/" % version
    chk = [LICENSE_HEADER, note,
           "// Compile-time check that the generated declarations match the vendor headers of this version:\n"
           "// sizes, alignments and member offsets of all structures, and all enum values.\n",
           '#include "libeos_eossdk_decl_%s.hpp"\n#include <cstddef>\n' % tag]
    for header in HEADER_ORDER:
        if (SDK_DIR / version / "include" / "EosprintApi" / header).exists():
            chk.append('#include "%s%s"' % (rel, header))
    chk.append("\nnamespace V = Lib3MF4EOS::Impl::%s;\n" % ns)
    for kind, name in hs.order:
        if kind == "record":
            lay = hs.layout(name)
            chk.append('static_assert(sizeof(::%s) == sizeof(V::%s), "%s: size differs");' % (name, name, name))
            chk.append('static_assert(sizeof(V::%s) == %d, "%s: computed size differs");' % (name, lay["size"], name))
            chk.append('static_assert(alignof(::%s) == alignof(V::%s), "%s: alignment differs");' % (name, name, name))
            for m in lay["members"]:
                chk.append('static_assert(offsetof(::%s, %s) == offsetof(V::%s, %s) && offsetof(V::%s, %s) == %d, "%s.%s: offset differs");'
                           % (name, m["name"], name, m["name"], name, m["name"], m["offset"], name, m["name"]))
            chk.append("")
        elif kind == "enum":
            for entry, value in hs.enums[name]:
                chk.append('static_assert((long long)::%s == (long long)V::%s && (long long)V::%s == %dLL, "%s: value differs");'
                           % (entry, entry, entry, value, entry))
            chk.append("")
        elif kind == "scalar":
            chk.append('static_assert(sizeof(::%s) == sizeof(V::%s), "%s: size differs");' % (name, name, name))
    (IMPL_SDK_DIR / ("libeos_eossdk_layout_%s.cpp" % tag)).write_text("\n".join(chk) + "\n", encoding="utf-8")
    print("generated declarations for %s: %d functions, %d structures, %d enums, %d callbacks" % (
        version, len(funcs), len(hs.records), len(hs.enums), len(hs.callbacks)))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    p_cmp = sub.add_parser("compare", help="write the difference report")
    p_cmp.add_argument("old")
    p_cmp.add_argument("new")
    p_gen = sub.add_parser("generate", help="write the declarations of one version")
    p_gen.add_argument("version")
    args = parser.parse_args()

    if args.command == "compare":
        version_tag(args.old)
        version_tag(args.new)
        a, b = load_version(args.old), load_version(args.new)
        diff = compare_sets(a, b)
        libeos = load_libeos_declarations()
        libeos_diff = libeos_vs_headers(libeos, a)
        out_html = REPO / "Documentation" / "eossdk-versions.html"
        out_json = REPO / "Documentation" / "eossdk-versions.json"
        write_report(a, b, diff, libeos_diff, out_html, out_json)
        print("wrote %s and %s" % (out_html.relative_to(REPO), out_json.relative_to(REPO)))
    else:
        generate(args.version)
    return 0


if __name__ == "__main__":
    sys.exit(main())
