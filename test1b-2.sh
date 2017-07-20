#! /bin/sh

# UCLA CS 111 Lab 1 - Test that valid syntax is processed correctly.

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >test.sh <<'EOF'

echo Line | cat | wc -l

echo tsebsiaciremA | rev | tee temp.out | rev | rev

echo UCLA; echo fight; echo fight; echo fight

(echo gomootong ; echo gomootong) | uniq | wc -l

(echo hank; echo williams; echo sr) | wc -l

(false || true) && echo gpa

(sleep 1 ; echo dorm) && echo loud ; echo smell


EOF

cat >test.exp <<'EOF'
1
Americaisbest
UCLA
fight
fight
fight
1
3
gpa
dorm
loud
smell
EOF

../timetrash test.sh >test.out 2>test.err || exit

diff -u test.exp test.out || exit
test ! -s test.err || {
  cat test.err
  exit 1
}

) || exit

rm -fr "$tmp"
