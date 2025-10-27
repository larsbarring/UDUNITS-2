#!/usr/bin/env bash
set -euo pipefail

# ---------- locate local build ----------
REPO_ROOT="${REPO_ROOT:-..}"
UDU="${REPO_ROOT}/prog/udunits2"

if [[ ! -x "$UDU" ]]; then
echo "Error: could not find built udunits2 binary at $UDU" >&2
echo "Hint: build first (make), or set REPO_ROOT=/path/to/repo-root" >&2
exit 127
fi

# Make sure the runtime linker can find libudunits2 from the build tree
export LD_LIBRARY_PATH="${REPO_ROOT}/lib/.libs:${LD_LIBRARY_PATH:-}"
export DYLD_LIBRARY_PATH="${REPO_ROOT}/lib/.libs:${DYLD_LIBRARY_PATH:-}"

# ---------- helpers ----------
pass=0
fail=0

ok() { printf '✅ %s\n\n' "$*"; }
bad() { printf '❌ %s\n\n' "$*" >&2; }

# Runs a command; expects exit 0 and output to contain a string
run_expect_ok_contains() {
local desc="$1"; local expect="$2"; shift 2
local out rc
set +e
out=$("$@"); rc=$?
set -e
if [[ $rc -eq 0 && "$out" == *"$expect"* ]]; then
ok "$desc"
pass=$((pass+1))
else
bad "$desc (exit=$rc; expected output containing: $expect)"
printf '%s\n' "$out" >&2
fail=$((fail+1))
fi
}

# Runs a command; expects non-zero and output to contain a string
run_expect_fail_contains() {
local desc="$1"; local expect="$2"; shift 2
local out rc
set +e
out=$("$@" 2>&1); rc=$?
set -e
if [[ $rc -ne 0 && "$out" == *"$expect"* ]]; then
ok "$desc"
pass=$((pass+1))
else
bad "$desc (exit=$rc; expected non-zero with output containing: $expect)"
printf '%s\n' "$out" >&2
fail=$((fail+1))
fi
}

# Runs a command; only checks exit code == 0
run_expect_ok() {
local desc="$1"; shift
local rc
set +e
"$@"; rc=$?
set -e
if [[ $rc -eq 0 ]]; then
ok "$desc"
pass=$((pass+1))
else
bad "$desc (exit=$rc)"
fail=$((fail+1))
fi
}

# ---------- tests ----------
main() {
echo "Using udunits2: $UDU"

# Accept (should succeed)
run_expect_ok_contains 'nanosecond → 1e-09 s' '1e-09 s' "$UDU" -H "nanosecond" -W ""
run_expect_ok_contains 'picosecond → 1e-12 s' '1e-12 s' "$UDU" -H "picosecond" -W ""
run_expect_ok_contains '1e-9 s → s' '1e-09 s' "$UDU" -H "1e-9 s" -W "s"
run_expect_ok_contains '2 s → ms' '2000 ms' "$UDU" -H "2 s" -W "ms"
run_expect_ok_contains 'kPa → Pa' '1000 Pa' "$UDU" -H "kPa" -W "Pa"
run_expect_ok_contains '(1/3) s → ms' '333.333' "$UDU" -H "(1/3) s" -W "ms"
run_expect_ok 'kg m s-2 → N (just exit 0)' "$UDU" -H "kg m s-2" -W "N"

# Reject (clear error message)
run_expect_fail_contains 'nan → error' 'not allowed' "$UDU" -H "nan" -W "1"
run_expect_fail_contains '+inf → error' 'not allowed' "$UDU" -H "+inf" -W "1"
run_expect_fail_contains 'nan(payload) kg → error' 'not allowed' "$UDU" -H "nan(payload) kg" -W "g"
run_expect_fail_contains 'nan m → error' 'not allowed' "$UDU" -H "nan m" -W "s"
run_expect_fail_contains 'inf s → error' 'not allowed' "$UDU" -H "inf s" -W "ms"
run_expect_fail_contains 'pico second → error' "Don't recognize" "$UDU" -H "pico second" -W ""

# Ambiguity / edge probes (per your latest policy both should fail like pico second)
run_expect_fail_contains 'nan_oops → fail' "Don't recognize" "$UDU" -H "nan_oops" -W ""
run_expect_fail_contains 'infinitystone → fail' "Don't recognize" "$UDU" -H "infinitystone" -W ""

# Spaces
run_expect_fail_contains '" nanosecond" with leading space' "Don't recognize" "$UDU" -H " nanosecond" -W ""
run_expect_fail_contains '"inf s" with -W "" → error' 'not allowed' "$UDU" -H "inf s" -W ""
run_expect_ok 'NaNoSeCoNd (mixed case)' "$UDU" -H "NaNoSeCoNd" -W ""
run_expect_ok '2nanosecond (2 ns)' "$UDU" -H "2nanosecond" -W "s" || true
run_expect_ok 'nanoyear (alias-dependent)' "$UDU" -H "nanoyear" -W "" || true


echo "Passed: $pass Failed: $fail"
echo ""
echo "========================================================="
echo ""
[[ $fail -eq 0 ]]
}



main "$@"