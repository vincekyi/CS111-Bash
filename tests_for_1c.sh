#!/bin/bash

#! /bin/sh


tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit
cat >test.sh <<'EOF'
(echo 1stcommand && sleep 1) || sort <file>file1
sleep 1
(echo 2ndcommand && sleep 1) || (echo 1stcommand && echo file1)>file2
((echo 3rdcommand || echo file2) && sleep 1) && (echo this && echo that && echo at) > file4
sleep 2
((echo 4thcommand || echo >file4) && sleep 1)  && rm file4
#this shouldn't affect the execution
sleep 3
(echo 5thcommand || echo file4) && (echo this && echo that) > this
sleep 4
(echo 6thcommand || echo this) && (echo a && echo b) > that
(sort < this | cat that - | sort -r) > that
sleep 5
(echo 7thcommand || echo this || echo that) && rm this && rm that
EOF

cat >test.exp <<'EOF'
1stcommand
2ndcommand
3rdcommand
4thcommand
5thcommand
6thcommand
7thcommand
EOF

../timetrash -t test.sh >test.out 2>test.err || exit

diff -u test.exp test.out || exit
test ! -s test.err || {
  cat test.err
  exit 1
}

) || exit

rm -fr "$tmp"
