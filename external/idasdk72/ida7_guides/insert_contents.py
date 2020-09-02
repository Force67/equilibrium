#!/usr/bin/python

import argparse
p = argparse.ArgumentParser()
p.add_argument("-t", "--template", type=str, help="The template file path", required=True)
p.add_argument("-c", "--contents", type=str, help="The contents file path", required=False)
p.add_argument("-T", "--tag", type=str, help="The 'tag' to look for, in the template file contents", required=True)
p.add_argument("-o", "--output", type=str, help="The output file path", required=True)
p.add_argument("-L", "--title", type=str, help="A possible title")
args = p.parse_args()

with open(args.template, "rb") as fin:
    template_str = fin.read()

def get_tag_idx(tag):
    tag_str = "<!--%s-->" % tag
    return template_str.index(tag_str), tag_str
tag_idx, tag_str = get_tag_idx(args.tag)

with open(args.contents, "rb") as fin:
    contents_str = fin.read()

output_str = template_str[0:tag_idx] + contents_str + template_str[tag_idx + len(tag_str):]

if args.title:
    tag_idx, tag_str = get_tag_idx("TITLE")
    output_str = output_str[0:tag_idx] + args.title + output_str[tag_idx + len(tag_str):]

with open(args.output, "wb") as fout:
    fout.write(output_str)

