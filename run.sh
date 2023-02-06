#!/bin/bash
echo "Arguments: $@"

if (( $# < 4 ))
  then
    echo "Not enough arguments suplied"
    exit -1
fi

# IFS=' ' read -ra args <<< "$@"
# for i in "${args[@]}"; do
#   # process "$i"
# done

if [ "$1" != "-np" ] || ! [[ "$2" =~ ^[0-9]+$ ]] || [ "$3" != "-fp" ]; then
        echo "The arguments suplied are not in the correct format"
    exit -1;
fi

cd out/build/
echo "$2 $4"
mpirun -np 1 --oversubscribe TEMA_APD $2 $4