#!/bin/bash

indent *.c
indent *.h

for i in *.[ch]; do
  echo fixing $i
  sed -e "s/_t\ \*\ /_t\ \*/g" -i $i
done

rm *~
