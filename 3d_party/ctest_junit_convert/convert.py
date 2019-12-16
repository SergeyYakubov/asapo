#!/usr/bin/env python

from lxml import etree
import argparse
from os.path import join

parser = argparse.ArgumentParser()
parser.add_argument("-x",dest='xsl', required=True)
parser.add_argument("-t", dest='tag',required=True)

parsed = parser.parse_args()


with open(join(parsed.tag, "Testing", "TAG")) as tagfile:
	directory = tagfile.readline().strip()

with open(join(parsed.tag, "Testing", directory, "Test.xml")) as testxmlfile:
	xmldoc = etree.parse(testxmlfile)

with open(parsed.xsl) as xslfile:
	xsl_root = etree.XML(xslfile.read())
	transform = etree.XSLT(xsl_root)

result_tree = transform(xmldoc)
print(result_tree)
