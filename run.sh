#!/bin/sh

mpirun -n 4 -host 192.168.0.1,192.168.0.2,192.168.0.3,192.168.0.4 ./mpibenchmark_p2p
