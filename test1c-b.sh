#! /bin/sh

# UCLA CS 111 Lab 1 - Test that valid syntax is processed correctly.

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >test.sh <<'EOF'

(echo Semi && echo handsome && echo lord) > sleep.txt 

sleep 1

echo persist && echo help | grep p > odor.txt

sleep 1

(echo silent ; echo hills ; echo game) | wc -l > germ.txt

cat germ.txt | cat | cat | cat > odor.txt

cat odor.txt > sleep.txt

cat sleep.txt | cat && echo brevity

EOF

cat >test.exp <<'EOF'
persist
3
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
