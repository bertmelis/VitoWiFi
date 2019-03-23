#!/bin/bash

find ./examples -type d -print0 | while IFS= read -r -d $'\0' line; do
  echo "$line";
done
