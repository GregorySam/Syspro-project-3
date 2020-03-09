#!/bin/bash


Generate_NewRandom_Integer()
{
  Y=$(shuf -i $1-$2 -n 1)
  while [ true ]
  do
    if ! grep -Fxq $Y  links_file
    then
      break
    fi
    Y=$(shuf -i $1-$2 -n 1)
  done
  echo $Y
}

#############################Input Check#################################
output=$(./InputCheck.sh $1 $2 $3 $4)

if [ "$output" == "OK" ]
then
  root_dir=$1
  text_file=$2
  websites=$3
  pages=$4
else
  echo "$output"
  exit
fi

##############################Create Directorie--Files################################
if ! [ -z "$(ls -A $root_dir)" ] #check if empty
then
  echo "Directory is not empty,purging..."
  rm -rf $root_dir/*
fi

echo -ne > allpages
for i in `seq 0 $((websites - 1))`  #generate websites
do

  echo -ne > links_file

  newdir="$root_dir/site$i"
  mkdir $newdir

  for j in `seq 1 $pages`       #generate filenames
  do
    x=$(Generate_NewRandom_Integer 1 $(($pages*$websites)))
    echo $x >> links_file
    filename=$root_dir/site$i/page$i"_"$x.html
    echo $filename >> allpages
  done
done
rm -f links_file

sed 's/^/\t/' $text_file > tmptext_file

i=0
while read -r LINE #generate HTML files
do
  if [ $(($i % $4)) == 0 ]
  then
    echo "####################################################################"
    echo "Creating web site $(($i / $4))"
  fi
  i=$(($i+1))
  ./CreateFile.sh "$LINE" "$websites" "$pages" allpages tmptext_file

done < allpages

 r1=$(sort pages_within_link | uniq | wc -l)
 r2=$(wc -l < allpages)


if [ $r1 -eq $r2 ]
then
  echo "All pages have at least one incoming link"
else
  echo "Some pages does not have at least one incoming link"
fi

rm -f pages_within_link

rm -f tmptext_file
