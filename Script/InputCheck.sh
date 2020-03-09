#!/bin/bash


Usage="Usage:
  /webcreator.sh root_directory text_file w p

  root_directory:Directory which exists
  text_file:Text with at least 10000 lines
  w:number of websites to be created >0
  p:Webpages number > 0"


if [[ -d $1 && -f $2 ]] #check existance of files and inputs
then


  if [  $(wc -l < $2) -lt 10000 ]  #check number of lines in file
  then
    echo "$Usage"
    exit
  fi

  if !( echo $3 | egrep -q '^[0-9]+$') || ! ( echo $4 | egrep -q '^[0-9]+$') #check if is integer
  then
    echo "$Usage"
    exit
  fi

  if [[ $3 -le 0 ||  $4 -le 0 ]]  #check if positive
  then
    echo "$Usage"
    exit
  fi

  echo "OK"

else
  echo "$Usage"
fi
