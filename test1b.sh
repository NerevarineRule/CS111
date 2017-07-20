#! /bin/sh

# UCLA CS 111 Lab 1 - Test that valid syntax is processed correctly.

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >test.sh <<'EOF'
echo Hooray

echo Semi

echo Line && sleep 1

echo Hello   World

echo Pipe | cat

echo Good | grep G

EOF

cat >test.exp <<'EOF'
Hooray
Semi
Line
Hello World
Pipe
Good
EOF

../timetrash test.sh >test.out 2>test.err || exit

diff -u test.exp test.out || exit
test ! -s test.err || {
  cat test.err
  exit 1
}

) || exit

rm -fr "$tmp"
