#!/bin/bash

for line in $(cat Makefile  | grep -o \.\/src.*\.cpp); do
  echo \"$line\", 
done 
