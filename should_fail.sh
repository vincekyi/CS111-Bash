#! /bin/sh

tmp=$0-$$.tmp
mkdir "$tmp" || exit
(
cd "$tmp" || exit
status=


n=1
for bad in \
  ' echo (this)'\
  'echo hello < thisdoesntexistihope'\
  'echo hello |& echo yeah'\
  'echo hello >a; echo bye>b;
	sort a | cat b - <'\
 '&& ; '\
  '(echo hello);)ls(' \
  '(echo hello) ls;' \
  'echo (hello)' \
  'echo hello < echo hi' \
  '(ecc) || echo hi' \
  '(echo hello; echo hi;)(;)' \
  '| echo hello' \
  'ls -l>hello < yup '
do
  echo "$bad" >test$n.sh || exit
  ../timetrash  test$n.sh >test$n.out 2>test$n.err 
  test -s test$n.err || {
    echo >&2 "test$n: no error message for: $bad"
    status=1
  }
  n=$((n+1))
done

#exit $status
) #|| exit

rm -fr "$tmp" || echo didntwork
