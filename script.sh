if test $# -ne 1
then
    echo "the wrong nr of arguments"
    exit 1
fi

output_gcc=$(gcc -Wall "$1" 2>&1)
exit_code=$?

if test $exit_code
then
    errors=$(echo "$output_gcc" | grep -c "error" )
    warnings=$(echo "$output_gcc" | grep -c "warning" )

    echo "$errors$warnings"
fi
