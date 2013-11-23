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

class BrainFuckMachine(object):
    def __init__(self, machine, output_mode = "ASCII", maximum_cell_value = None,
                 input_allowed = False):
        self.machine = machine
        self.output_mode = output_mode
        self.maximum_cell_value = maximum_cell_value
        self.input_allowed = input_allowed
        
    def validate(self):
        nesting_level = 0
        for c in self.machine:
            if c == "[":
                nesting_level += 1
            elif c == "]":
                nesting_level -= 1
            elif c == ",":
                if not self.input_allowed:
                    return False
            elif c not in "<>+-.":
                return False
            if nesting_level < 0:
                return False
        return True
    
    def execute(self, max_iterations = 1000*1000):
        iters = 0
        pos = 0
        tape = [0]
        cmd_pos = 0
        nesting_level = 0
        while iters <= max_iterations:
            c = self.machine[cmd_pos]
            if c == ".":
                if self.output_mode == "ASCII":
                    yield chr(tape[pos])
                cmd_pos += 1
            elif c == "+":
                tape[pos] += 1
                if self.maximum_cell_value:
                    tape[pos] = tape[pos] % self.maximum_cell_value
                cmd_pos += 1
            elif c == "-":
                tape[pos] -= 1
                if self.maximum_cell_value:
                    if tape[pos] < 0:
                        tape[pos] = self.maximum_cell_value
                cmd_pos += 1
            elif c == "<":
                pos -= 1
                if pos < 0:
                    tape = [0] + tape
                    pos = 0
                cmd_pos += 1
            elif c == ">":
                pos += 1
                if pos >= len(tape):
                    tape.append(0)
                cmd_pos += 1
            elif c == "[":
                if tape[pos] == 0:
                    nesting_level = 1
                    while nesting_level > 0:
                        cmd_pos += 1
                        if self.machine[cmd_pos] == "]":
                            nesting_level -= 1
                else:
                    cmd_pos += 1
            elif c == "]":
                if tape[pos] != 0:
                    nesting_level = -1
                    while nesting_level < 0:
                        cmd_pos -= 1
                        if self.machine[cmd_pos] == "[":
                            nesting_level += 1
                else:
                    cmd_pos += 1
            if cmd_pos >= len(self.machine):
                break

def main():
    hello_world = "++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>."
    bf = BrainFuckMachine(machine = hello_world, maximum_cell_value = 255)
    print bf.validate()
    res = ""
    for i in bf.execute():
        print i
        res += i
    print res
        
if __name__ == "__main__":
    main()