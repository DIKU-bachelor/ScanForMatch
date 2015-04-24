IFS=$'\n'
c=0
while read i; do
  c=$((c+1))
  echo . >> pattern.txt
  echo . >> scan_for_matches_m.txt
  echo . >> scanfm_m.txt
  rm pattern.txt
  rm scan_for_matches_m.txt
  rm scanfm_m.txt
  echo "$i" >> pattern.txt
  echo ""
  echo "*****************************************************************"
  echo "Test $c: ($i)"
  ./scan_for_matches pattern.txt < chr10_orig.fa >> scan_for_matches_m.txt
  ./scanfm -p "$i" -d chr10.fa >> scanfm_m.txt
  python compare.py scanfm_m.txt scan_for_matches_m.txt
done < $1
