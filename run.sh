#!/bin/sh
exec xterm -e "make run 2>&1 | tee .run.out+err"
