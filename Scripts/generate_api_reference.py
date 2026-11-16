#!/usr/bin/env python3
"""
Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Generates Documentation/api.html from the ACT API definition ACT/Lib3MFToolpath4EOS.xml.
          Run it after changing the API:  python Scripts/generate_api_reference.py
"""

import html
import os
import xml.etree.ElementTree as ET

REPOSITORY_DIR = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
API_DEFINITION = os.path.join(REPOSITORY_DIR, "ACT", "Lib3MFToolpath4EOS.xml")
OUTPUT_FILE = os.path.join(REPOSITORY_DIR, "Documentation", "api.html")

ACT_NAMESPACE = "{http://schemas.autodesk.com/netfabb/automaticcomponenttoolkit/2018}"


def tag(name):
	return ACT_NAMESPACE + name


def esc(text):
	return html.escape(text or "", quote=True)


class CReference:
	def __init__(self, root):
		self.root = root
		self.classes = [node.get("name") for node in root.findall(tag("class"))]
		self.enums = [node.get("name") for node in root.findall(tag("enum"))]
		self.structs = [node.get("name") for node in root.findall(tag("struct"))]

	def link(self, name):
		if name in self.classes or name in self.enums or name in self.structs:
			return '<a href="#%s">%s</a>' % (esc(name), esc(name))
		return '<span class="external">%s</span>' % esc(name.replace(":", "::"))

	def typeName(self, node):
		sType = node.get("type")
		sClass = node.get("class") or ""
		if sType in ("class", "enum", "struct", "optionalclass"):
			return self.link(sClass)
		if sType == "basicarray":
			return "array of %s" % esc(sClass)
		if sType == "structarray":
			return "array of %s" % self.link(sClass)
		if sType == "functiontype":
			return "callback %s" % esc(sClass)
		return esc(sType)

	def signature(self, method):
		inputs = []
		outputs = []
		returnType = "void"
		for param in method.findall(tag("param")):
			sPass = param.get("pass")
			sText = '%s <span class="param">%s</span>' % (self.typeName(param), esc(param.get("name")))
			if sPass == "return":
				returnType = self.typeName(param)
			elif sPass == "out":
				outputs.append("out " + sText)
			else:
				inputs.append(sText)
		return '<code class="signature">%s <b>%s</b>(%s)</code>' % (returnType, esc(method.get("name")), ", ".join(inputs + outputs))

	def methodHTML(self, method):
		lines = ['<div class="method" id="%s">' % esc(method.get("name")), self.signature(method)]
		lines.append("<p>%s</p>" % esc(method.get("description")))
		params = method.findall(tag("param"))
		if params:
			lines.append('<table class="params"><tr><th>Parameter</th><th>Direction</th><th>Type</th><th>Description</th></tr>')
			for param in params:
				lines.append("<tr><td><code>%s</code></td><td>%s</td><td>%s</td><td>%s</td></tr>" % (
					esc(param.get("name")), esc(param.get("pass")), self.typeName(param), esc(param.get("description"))))
			lines.append("</table>")
		lines.append("</div>")
		return "\n".join(lines)

	def generate(self):
		root = self.root
		sVersion = root.get("version")
		out = []
		out.append("<!DOCTYPE html>")
		out.append('<html lang="en"><head><meta charset="utf-8">')
		out.append("<title>Lib3MFToolpath4EOS %s API reference</title>" % esc(sVersion))
		out.append('<link rel="stylesheet" href="style.css"></head><body>')
		out.append('<header class="site"><a class="brand" href="index.html"><img src="images/3mf-logo.png" alt=".3MF"><span>Lib3MFToolpath4EOS</span></a>'
			'<nav><a href="index.html">Library guide</a><a href="api.html" class="active">API reference</a><a href="examples.html">Examples</a><a href="power-ramps.html">Power ramps</a><a href="conversion.html">Conversion</a></nav></header>')
		out.append('<div class="layout"><aside class="toc"><h3>Global</h3><ul><li><a href="#Wrapper">Wrapper functions</a></li><li><a href="#Errors">Error codes</a></li></ul>')
		out.append("<h3>Classes</h3><ul>")
		for sName in self.classes:
			out.append('<li><a href="#%s">%s</a></li>' % (esc(sName), esc(sName)))
		out.append("</ul><h3>Enums and structs</h3><ul>")
		for sName in self.enums + self.structs:
			out.append('<li><a href="#%s">%s</a></li>' % (esc(sName), esc(sName)))
		out.append("</ul></aside><main>")

		out.append("<h1>Lib3MFToolpath4EOS %s API reference</h1>" % esc(sVersion))
		out.append("<p>Generated from <code>ACT/Lib3MFToolpath4EOS.xml</code> by <code>Scripts/generate_api_reference.py</code>. "
			"The reference is language neutral: parameters marked <code>out</code> are output parameters, the value before the "
			"method name is the return value. Every binding maps these to its own conventions; in the C++ bindings, classes are "
			"returned as shared pointers (<code>PEOSContext</code> etc.) and errors are thrown as <code>ELib3MF4EOSException</code>.</p>")

		globalNode = root.find(tag("global"))
		out.append('<section id="Wrapper"><h2>Wrapper functions</h2>')
		out.append("<p>Functions of the library object (<code>Lib3MF4EOS::CWrapper</code> in C++).</p>")
		for method in globalNode.findall(tag("method")):
			out.append(self.methodHTML(method))
		out.append("</section>")

		for node in root.findall(tag("class")):
			sName = node.get("name")
			sParent = node.get("parent")
			out.append('<section id="%s"><h2>%s</h2>' % (esc(sName), esc(sName)))
			if sParent:
				out.append('<p class="inherits">Inherits from %s</p>' % self.link(sParent))
			if node.get("description"):
				out.append("<p>%s</p>" % esc(node.get("description")))
			for method in node.findall(tag("method")):
				out.append(self.methodHTML(method))
			out.append("</section>")

		for node in root.findall(tag("enum")):
			out.append('<section id="%s"><h2>enum %s</h2><table class="params"><tr><th>Name</th><th>Value</th></tr>' % (esc(node.get("name")), esc(node.get("name"))))
			for option in node.findall(tag("option")):
				out.append("<tr><td><code>%s</code></td><td>%s</td></tr>" % (esc(option.get("name")), esc(option.get("value"))))
			out.append("</table></section>")

		for node in root.findall(tag("struct")):
			out.append('<section id="%s"><h2>struct %s</h2><table class="params"><tr><th>Member</th><th>Type</th><th>Description</th></tr>' % (esc(node.get("name")), esc(node.get("name"))))
			for member in node.findall(tag("member")):
				out.append("<tr><td><code>%s</code></td><td>%s</td><td>%s</td></tr>" % (esc(member.get("name")), self.typeName(member), esc(member.get("description"))))
			out.append("</table></section>")

		out.append('<section id="Errors"><h2>Error codes</h2><table class="params"><tr><th>Code</th><th>Name</th><th>Description</th></tr>')
		for error in root.find(tag("errors")).findall(tag("error")):
			out.append("<tr><td>%s</td><td><code>%s</code></td><td>%s</td></tr>" % (esc(error.get("code")), esc(error.get("name")), esc(error.get("description"))))
		out.append("</table></section>")

		out.append("</main></div></body></html>")
		return "\n".join(out) + "\n"


def main():
	root = ET.parse(API_DEFINITION).getroot()
	with open(OUTPUT_FILE, "w", encoding="utf-8", newline="\n") as outputFile:
		outputFile.write(CReference(root).generate())
	print("Wrote " + OUTPUT_FILE)


if __name__ == "__main__":
	main()
