#!/bin/sh

#tests that pass
echo
echo hello
echo
#test and
echo both && echo prints
false && echo donotshow
echo
#test or
false || echo print
echo onlythisprints || echo notthis
echo
#test semi colon
echo HELLO;echo BYE
echo
#tests diff and redirection
diff command-internals.h execute-command.c > diff_file
# remove file
rm diff_file
#test flags
ls 
ls -a > c
ls -a > d
diff c d
rm c
rm d 
echo
#test exec
(exec echo hello || echo h) && echo bye
echo
#test subshell
echo hello> a
echo bye > b
(echo hello; echo bye; echo 1; echo c; echo a) > a
(echo thisisb)>b
#test pipe
sort a | cat b - | sort -r |cat a - | cat b -
(sort a | cat b - | sort -r |cat a - | cat b -) | echo hello
#test redirect
sort <a>b
rm a
rm b
echo
echo
#redirect
echo hello>file
(echo thisshouldbeinfile)>file
rm file


echo hi > a
echo bye > b

#sort a| sort -r <|cat b -
rm a
rm b

echo
(echo hello) ls
echo hello

