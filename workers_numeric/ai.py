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
import multiprocessing
import threading
import sys

def work(in_q, out_q, stop_flag):
    while True:
        print "ololo"
        print "stop_flag:", stop_flag.value
        if stop_flag.value == 1:
            return
        try:
            deisred_out, machine = in_q.get_nowait()
            print deisred_out, machine
        except Exception:
            continue
        
        if False:
            bf = brainfuck.BrainFuckMachine(machine = machine, output_mode = "int")
            if not bf.validate():
                continue
            bf.match_braces()
            res = []
            i = 0
            for c in bf.execute(max_iterations = 10):
                res.append(c)
                if c != desired_out[i]:
                    out_q.put( (False, machine, res) )
                    break
                if len(res) == len(desired_out):
                    out_q.put( (True, machine, res) )
                    break
                i += 1
            print res == desired_out, machine, res
            out_q.put( (res == desired_out, machine, res))

        

def main():
    symbols = "<>[]+-."
    desired_out = [0]
    stop_flag = multiprocessing.Value('i', 0)
    in_q = multiprocessing.Queue()
    out_q = multiprocessing.Queue()
    stdout_q = multiprocessing.Queue()
    workers = []
    for _ in xrange(1):
        #worker = multiprocessing.Process(target = work, args=(in_q, out_q, stop_flag))
        worker = threading.Thread(target = work, args=(in_q, out_q, stop_flag))
        worker.daemon = True
        worker.start()
        workers.append(worker)
    #fetcher = multiprocessing.Process(target = fetch, args=(out_q, stdout_q, stop_flag))
    #fetcher = threading.Thread(target = fetch, args=(out_q, stdout_q, stop_flag))
    #fetcher.daemon = True
    #fetcher.start()
    for size in [1]:
        for comb in itertools.product(symbols, repeat = size):
            try:
                print stdout_q.get_nowait()
            except Exception as e:
                pass
            machine = "".join(comb)
            print machine
            if machine == "":
                continue
            print "stop flag from main:", stop_flag.value
            if stop_flag.value == 1:
                print machine
                for i in xrange(len(workers)):
                    workers[i].join()
                fetcher.join()
                try:
                    print stdout_q.get_nowait()
                except Exception:
                    pass
                sys.exit(0)
            print (desired_out, machine)
            in_q.put( (desired_out, machine) )
            c = in_q.get()
            print c
            in_q.put(c)

    stop_flag.value = 1
    for i in xrange(len(workers)):
        workers[i].join()
    #fetcher.join()
    try:
        print stdout_q.get_nowait()
    except Exception:
        pass
    while not out_q.empty():
        print out_q.get()
    sys.exit(0)
            
            
            
if __name__ == "__main__":
    main()