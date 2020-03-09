#!/bin/bash



html_file=$1
pages=$2
allpages=$4
websites=$3
text_file=$5


lines_in_textfile=$(wc -l < $text_file)

# calculate parameters
k=$(shuf -i 1-$(($lines_in_textfile -2000)) -n 1)
m=$(shuf -i 1000-2000 -n 1)
f=$(($websites/2+1))
q=$(($pages/2+1))

links=$(grep -v $html_file  $allpages |shuf | head -$(($f+$q)))
echo "$links" > tmp_file


#########Create HTML file###########
echo "<!DOCTYPE html>
<html>
  <body>" >> $html_file

numberof_copylines=$(($m/($f+$q)))
end=$(($numberof_copylines+$k-1))

flag_t=true
flag_b=true

while [ $flag_t == true ] ||  [ $flag_b == true ]
do
  #########read from text file######################
  if [ $flag_t == true ]
  then
    echo "  Adding to "$html_file" $numberof_copylines lines starting at line $k till $end"
    sed -n -e $k,$end"p" tmptext_file >> $html_file
    k=$(($end+1))
    end=$(($numberof_copylines+$k-1))

    if [ $end -gt $lines_in_textfile ]
    then
      end=$lines_in_textfile
      echo "  Adding to "$html_file" $numberof_copylines lines starting at line $k till $end"
      sed -n -e $k,$end"p" tmptext_file >> $html_file
      flag_t=false
    fi
  fi
  #################################################
  ######WriteLink#################################
  if [ $flag_b == true ]
  then
    read -r line < tmp_file
    sed -i 1d tmp_file
    if [ -z $line ]
    then
      flag_b=false
    else
      echo "  Adding link to "$line""
      echo "$line" >> pages_within_link
      link=$( echo "$line" | awk -F '/' '{print "../"$(NF-1)"/"$NF} ')
      page=$( echo "$line" | awk -F '/' '{print $NF}')
      echo "  <a href=$link>$page</a>" >> $html_file
    fi
  fi
done

echo "  </body>
</html>" >> $html_file

rm -f tmp_file
