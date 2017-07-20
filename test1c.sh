#! /bin/sh

# UCLA CS 111 Lab 1 - Test that valid syntax is processed correctly.

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >test.sh <<'EOF'

echo Semi > sleep.txt 

sleep 1

echo Line

echo mantel > odor.txt

sleep 1

cat odor.txt > sleep.txt

echo sims > sleep.txt

cat sleep.txt | cat && echo brevity

EOF

cat >test.exp <<'EOF'
Line
sims
brevity
EOF

../timetrash -t test.sh >test.out 2>test.err || exit

diff -u test.exp test.out || exit
test ! -s test.err || {
  cat test.err
  exit 1
}

) || exit

rm -fr "$tmp"
