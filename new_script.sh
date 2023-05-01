#!/bin/bash

if [[ $# -ne 1 ]]; then
    echo "Error: wrong number of arguments"
    exit 1
fi

file_extension="${1##*.}"
if [[ "$file_extension" != "c" || ! -f "$1" ]]; then
    echo "Error: $1 is not a C file"
    exit 1
fi

output_gcc=$(gcc -Wall -Wextra "$1" 2>&1)
exit_code=$?

#if[[ $exit_code -ne 0 ]]; then 
    errors=$(echo "$output_gcc" | grep -c "error")
    warnings=$(echo "$output_gcc" | grep -c "warning")

    if [[ $errors -eq 0 && $warnings -eq 0 ]]; then
        score=10
    elif [[ $errors -gt 0 ]]; then
        score=1
    elif [[ $warnings -gt 10 ]]; then
        score=2
    else
        score=$((2 + 8 * (10 - warnings) / 10))
    fi

    echo "Score: $score"
#else


