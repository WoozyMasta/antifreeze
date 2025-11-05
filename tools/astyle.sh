#!/usr/bin/env bash
set -eu

: "${PROJECT_DIR:=${1:-$PWD}}"
: "${ASTYLE_RC:=${2:-.astylerc}}"

cd "$PROJECT_DIR"
astyle --project="$ASTYLE_RC" ./'*.c' ./'*.cpp'

while read -r file; do
  first_line="$(tools/first_code_line.awk "$file")"
  if [ "${first_line:-}" != "#ifdef SERVER" ]; then
    echo "FAIL: $file"
    exit 1
  fi
done < <(find ./scripts -type f -name "*.c")
