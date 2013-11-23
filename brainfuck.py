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
                pass
            if nesting_level < 0:
                return False
        if nesting_level > 0:
            return False
        return True
    
    def match_braces(self):
        self.matching_braces = [0 for _ in xrange(len(self.machine))]
        positions = []
        for i in xrange(len(self.machine)):
            if self.machine[i] not in "[]":
                continue
            else:
                if self.machine[i] == "[":
                    positions.append(i)
                else:
                    positions, self.matching_braces[i] = positions[:-1], positions[-1]
        for i in xrange(len(self.matching_braces)):
            if self.machine[i] == "]":
                self.matching_braces[self.matching_braces[i]] = i
    
    def execute(self, max_iterations = 1000*1000, input_string = None):
        iters = 0
        pos = 0
        tape = [0]
        cmd_pos = 0
        nesting_level = 0
        input_pos = 0
        while iters <= max_iterations:
            iters += 1
            c = self.machine[cmd_pos]
            if c == ".":
                if self.output_mode == "ASCII":
                    yield chr(tape[pos])
                elif self.output_mode == "int":
                    yield tape[pos]
                cmd_pos += 1
            elif c == ",":
                if input_pos >= len(input_string):
                    break
                tape[pos] = ord(input_string[input_pos])
                input_pos += 1
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
                    cmd_pos = self.matching_braces[cmd_pos]
                else:
                    cmd_pos += 1
            elif c == "]":
                if tape[pos] != 0:
                    cmd_pos = self.matching_braces[cmd_pos]
                else:
                    cmd_pos += 1
            else:
                cmd_pos += 1
            if cmd_pos >= len(self.machine):
                break

def main():
    hello_world = """++++++++++[>+++++++>++++++++++>+++>+<<<<-]
    >++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>."""
    bf = BrainFuckMachine(machine = hello_world, maximum_cell_value = 255)
    print bf.validate()
    bf.match_braces()
    res = ""
    for i in bf.execute():
        res += i
    print res
    rot13 = """-,+[                         Read first character and start outer character reading loop
    -[                       Skip forward if character is 0
        >>++++[>++++++++<-]  Set up divisor (32) for division loop
                               (MEMORY LAYOUT: dividend copy remainder divisor quotient zero zero)
        <+<-[                Set up dividend (x minus 1) and enter division loop
            >+>+>-[>>>]      Increase copy and remainder / reduce divisor / Normal case: skip forward
            <[[>+<-]>>+>]    Special case: move remainder back to divisor and increase quotient
            <<<<<-           Decrement dividend
        ]                    End division loop
    ]>>>[-]+                 End skip loop; zero former divisor and reuse space for a flag
    >--[-[<->+++[-]]]<[         Zero that flag unless quotient was 2 or 3; zero quotient; check flag
        ++++++++++++<[       If flag then set up divisor (13) for second division loop
                               (MEMORY LAYOUT: zero copy dividend divisor remainder quotient zero zero)
            >-[>+>>]         Reduce divisor; Normal case: increase remainder
            >[+[<+>-]>+>>]   Special case: increase remainder / move it back to divisor / increase quotient
            <<<<<-           Decrease dividend
        ]                    End division loop
        >>[<+>-]             Add remainder back to divisor to get a useful 13
        >[                   Skip forward if quotient was 0
            -[               Decrement quotient and skip forward if quotient was 1
                -<<[-]>>     Zero quotient and divisor if quotient was 2
            ]<<[<<->>-]>>    Zero divisor and subtract 13 from copy if quotient was 1
        ]<<[<<+>>-]          Zero divisor and add 13 to copy if quotient was 0
    ]                        End outer skip loop (jump to here if ((character minus 1)/32) was not 2 or 3)
    <[-]                     Clear remainder from first division if second division was skipped
    <.[-]                    Output ROT13ed character from copy and clear it
    <-,+                     Read next character
]                            End character reading loop
"""
    bf = BrainFuckMachine(machine = "[][[]][[][]]", maximum_cell_value = 255, input_allowed = True)
    bf.match_braces()
    myjoin = lambda lst: reduce(lambda x, y: x + " %2s"%str(y), lst, "")
    print myjoin(range(len(bf.machine)))
    print myjoin(bf.machine)
    print myjoin(bf.matching_braces)
    bf = BrainFuckMachine(machine = rot13, maximum_cell_value = 255, input_allowed = True)

    print bf.validate()
    bf.match_braces()
    print bf.matching_braces
    res = ""
    for i in bf.execute(input_string = "hello world\0"):
        print i
        res += i
    print res
    bf = BrainFuckMachine(machine = "+[-.+.]", output_mode = "int")
    bf.validate()
    bf.match_braces()
    for i in bf.execute(max_iterations = 10):
        print i
    bf = BrainFuckMachine(machine = "<>+[]-.", output_mode = "int")
    bf.validate()
    bf.match_braces()
    for i in bf.execute(max_iterations = 1000):
        print i


        
if __name__ == "__main__":
    main()