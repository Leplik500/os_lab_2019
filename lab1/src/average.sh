#!/bin/bash
echo "quantity of arguments = $#"
for arg in "$@"
 do
	let sum=$sum+$arg
done
echo "average = $(echo "scale=2; $sum/$#" | bc)"
