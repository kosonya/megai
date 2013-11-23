#!/usr/bin/env python
# -*- coding: utf-8 -*-
#    Copyright (c) 2013 Maxim Kovalev
#    This file is part of MegAI.
#
#    MegAI is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    MegAI is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with MegAI.  If not, see <http://www.gnu.org/licenses/>.
if __name__ == '__main__' and __package__ is None:
    from os import sys, path
    sys.path.append(path.dirname(path.dirname(path.abspath(__file__))))
    __package__ = path.dirname(path.dirname(path.abspath(__file__)))
    
import brainfuck
import itertools

def main():
    symbols = "<>[]+-."
    desired_out = [0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1]
    for size in xrange(10):
        for comb in itertools.product(symbols, repeat = size):
            machine = "".join(comb)
            if machine == "":
                continue
            print "Testing machine", machine
            bf = brainfuck.BrainFuckMachine(machine = machine, output_mode = "int")
            if not bf.validate():
                print "Invalid, skipped"
                continue
            bf.match_braces()
            res = []
            i = 0
            for c in bf.execute(max_iterations = 100):
                res.append(c)
                if c != desired_out[i]:
                    print "Wanted:", desired_out
                    print "Got:", res
                    break
                if len(res) == len(desired_out):
                    break
                i += 1
            if res == desired_out:
                print "Sucess!"
                print "Desired output:", desired_out
                print "Output:", res
                print "Machine:", machine
                return
            else:
                print "Looks like this doesn't halt or output the string of the appropriate length"
            
    
if __name__ == "__main__":
    main()