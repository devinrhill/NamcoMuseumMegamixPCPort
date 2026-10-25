#!/bin/sh
# Syntax-check every src/**/*.cpp in parallel. Logs go to $OUT (default build/syn).
# Prints the files that pass; move those into cmake/sources.cmake.
cd "$(dirname "$0")/.." || exit 1
OUT=${OUT:-build/syn}
rm -rf "$OUT"; mkdir -p "$OUT"
find src -name '*.cpp' | xargs -P "$(nproc)" -I{} sh -c '
  o=$(echo {} | tr / _)
  if g++ -std=c++20 -fsyntax-only -fpermissive -w -Iinclude {} > "'"$OUT"'/$o.log" 2>&1; then echo {} >> "'"$OUT"'/passing.txt"; fi'
echo "passing: $(sort "$OUT/passing.txt" | wc -l) / $(find src -name '*.cpp' | wc -l)"
sort "$OUT/passing.txt" > "$OUT/passing.sorted"
