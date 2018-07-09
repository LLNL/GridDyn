# /*
#  * LLNS Copyright Start
#  * Copyright (c) 2014-2018, Lawrence Livermore National Security
#  * This work was performed under the auspices of the U.S. Department
#  * of Energy by Lawrence Livermore National Laboratory in part under
#  * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
#  * Produced at the Lawrence Livermore National Laboratory.
#  * All rights reserved.
#  * For details, see the LICENSE file.
#  * LLNS Copyright End
#  */

"""
This file is in a state of flux as I fix warnings. Some of these things will
end up in parser.py, but most of this file will likely be deleted
"""

import parser


## filter lines to only include warning starts
def identity(x):
    return x

def filter_apply(predicate, function, arg):
    # arg = (thing to filter by, thing to run the function on)
    pred = predicate(arg[0])
    fun = None
    if pred: # don't run function if you don't have to
        fun = function(arg[1])
    return pred, fun

def filter_pair(results):
    # keep things in list that go (True, _)
    return [value for keep, value in results if keep]

filter_apply_is_start = lambda arg: filter_apply(parser.is_start, identity, arg)

## extract chunk ranges
def get_line_ranges(line_starts, total):
    # get line ranges, assuming sorted
    ranges = [(i, j) for i, j in zip(line_starts, line_starts[1:])]
    ranges.append((line_starts[-1], total))
    return ranges

## get chunk from line range
def get_chunk(line_range, lines):
    return parser.Chunk(lines[line_range[0]:line_range[1]])

## get chunks from file
def get_chunks(filename):
    with open(filename, 'r') as file:
        lines = file.readlines()
        linenos = range(len(lines))
        print('Read {} lines of file {}'.format(len(lines), filename))

    line_starts = filter_pair(map(filter_apply_is_start, zip(lines, linenos)))
    line_ranges = get_line_ranges(line_starts, len(lines))

    get_chunk_from_lines = lambda line_range: get_chunk(line_range, lines)

    chunks = list(map(get_chunk_from_lines, line_ranges))
    return chunks

## pipeline
#default_before_chunks = get_chunks('default_before.tidy')
default_after_chunks =  get_chunks('default_after.tidy')
#all_before_chunks = get_chunks('all_before.tidy')
all_after_chunks =  get_chunks('all_after.tidy')

#unique_default_before_chunks = set(default_before_chunks)
#unique_all_before_chunks = set(all_before_chunks)
#before_chunks = unique_default_before_chunks | unique_all_before_chunks

unique_default_after_chunks = set(default_after_chunks)
unique_all_after_chunks = set(all_after_chunks)
after_chunks = unique_default_after_chunks | unique_all_after_chunks

#fixed_chunks = before_chunks - after_chunks
#new_chunks = after_chunks - before_chunks

chunks = after_chunks

#print('fixed: {}'.format(len(fixed_chunks)))
#print('new: {}'.format(len(new_chunks)))
#print('total now: {}'.format(len(chunks)))

# create map[guideline] -> list of chunks violating that guideline
guidelines = set(chunk.guideline for chunk in chunks)
guideline_map = {}
for guideline in guidelines:
    guideline_map[guideline] = []

for chunk in chunks:
    guideline_map[chunk.guideline].append(chunk)

# sort each list by filename
filename_key = lambda x: x.file_name
for guideline in guidelines:
    guideline_map[guideline] = sorted(guideline_map[guideline], key=filename_key)

guideline_priority = [(k, len(v)) for k, v in guideline_map.iteritems()]
guideline_priority.sort(key=lambda x: x[1])

for k, v in guideline_priority:
    print('{}: {}'.format(k, v))


not_comment = lambda line: line.strip()[0] != '#'
not_empty = lambda line: line.strip() != ''


with open('guideline_notes.txt', 'r') as f:
    guidelines = f.readlines()

guidelines = filter(not_comment, guidelines)
guidelines = filter(not_empty, guidelines)
guidelines = [l.strip() for l in guidelines]

def separator(color='red'):
    colormap = {
        'red': '\033[31m',
        'green': '\033[32m',
    }
    decolor = '\033[0m'
    sep = colormap[color] + '='*50 + decolor
    return sep

print(separator('green'))
# only grab first guideline
for i in guideline_map[guidelines[0]]:
    print(i.original)
    print(separator())

filenames = set(map(lambda i: i.file_name, guideline_map[guidelines[0]]))
print('\nFiles referenced:')
for filename in sorted(filenames):
    print('\t{}'.format(filename))
