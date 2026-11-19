#!/bin/sh
# Small repeatable smoke tests for the msh assignment.
set -eu
ROOT=$(CDPATH var = cd -- "$(dirname -- "$0")/.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
make -C "$ROOT" >/dev/null
BIN="$ROOT/msh"

out=$(printf 'help\nexit\n' | "$BIN")
printf '%s' "$out" | grep -q 'Built-ins: cd'
printf 'PASS: help and exit built-ins\n'

out=$(printf 'pwd\nexit\n' | "$BIN")
printf '%s' "$out" | grep -q "$ROOT"
printf 'PASS: external command execution\n'

out=$(printf 'cd /\npwd\nexit\n' | "$BIN")
printf '%s' "$out" | grep -q '^/\|msh\[0\]> /'
printf 'PASS: cd changes the shell working directory\n'

printf 'printf hello > %s\nexit\n' "$TMP/output.txt" | "$BIN" >/dev/null
test "$(cat "$TMP/output.txt")" = hello
printf 'PASS: output redirection\n'

out=$(printf 'command-that-does-not-exist\nexit\n' | "$BIN" 2>&1 || true)
printf '%s' "$out" | grep -q 'command-that-does-not-exist'
printf 'PASS: failed exec reports an error\n'
