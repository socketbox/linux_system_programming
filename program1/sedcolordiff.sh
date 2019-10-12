#!/bin/bash   
#https://stackoverflow.com/questions/8800578/how-to-colorize-diff-on-the-command-line
sed 's/^-/\x1b[31m-/;s/^+/\x1b[32m+/;s/^@/\x1b[34m@/;s/$/\x1b[0m/'

