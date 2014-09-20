#! /bin/bash

astyle -A10 -t4 -xn -xc -xl -xk -C -S -N -Y -f -p -xw -k2 -y -r "include/*.hpp"
astyle -A10 -t4 -xn -xc -xl -xk -C -S -N -Y -f -p -xw -k2 -y -r "src/*.cpp"