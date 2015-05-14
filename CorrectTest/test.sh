echo "-----------------------------------------------------"
echo "Sequence tests"
echo "-----------------------------------------------------"

echo "empty sequence"
./scanfm -p   -d sq1d.txt
echo "A"
./scanfm -p A -d sq2d.txt
echo "ACCGT"
./scanfm -p ACCGT -d sq3d.txt
echo "ACGT[0,0,0]"
./scanfm -p ACGT[0,0,0] -d sq5d.txt
echo "ACC[1,0,0]"
./scanfm -p ACC[1,0,0] -d sq5d.txt
echo "ATG[0,1,0]"
./scanfm -p ATG[0,1,0] -d sq6d.txt
echo "ACC[0,0,1]"
./scanfm -p ACC[0,0,1] -d sq7d.txt
echo "AGGT[1,1,0]"
./scanfm -p AGGT[1,1,0] -d sq8d.txt
echo "ACGT[1,0,1]"
./scanfm -p ACGT[1,0,1] -d sq9d.txt
echo "ATTGT[0,1,1]"
./scanfm -p ATTGT[0,1,1] -d sq10d.txt
echo "AACGT[1,1,1]"
./scanfm -p AACGT[1,1,1] -d sq11d.txt
echo "ACCT[2,0,0]"
./scanfm -p ACCT[2,0,] -d sq12d.txt
echo "AGTTT[0,2,0]"
./scanfm -p AGTTT[0,2,0] -d sq13d.txt
echo "ACGT[0,0,2]"
./scanfm -p ACGT[0,0,2] -d sq14d.txt
echo "ACTTT[1,2,0]"
./scanfm -p ACTTT[1,2,0] -d sq15d.txt
echo "TCGAT[3,0,2]"
./scanfm -p TCGAT[3,0,2] -d sq16d.txt
echo "TCGGT[0,2,3]"
./scanfm -p TCGGT[0,2,3] -d sq17d.txt
echo "ATTCCCTT[2,2,1]"
./scanfm -p ATTCCCTT[2,2,1] -d sq18d.txt
echo "ATTC[0,6,0]"
./scanfm -p ATTC[0,6,0] -d sq19d.txt
echo "ATTCG[8,0,0]"
./scanfm -p ATTCG[8,0,0] -d sq20d.txt
echo "R"
./scanfm -p R -d sq21d.txt
echo "RRGGWW"
./scanfm -p RRGGWW -d sq22d.txt
echo "UUMMRRWWSSYYKKBBBDDDHHHVVVNNNN"
./scanfm -p UUMMRRWWSSYYKKBBBDDDHHHVVVNNNN -d sq23d.txt


echo "-----------------------------------------------------"
echo "Range tests"
echo "-----------------------------------------------------"

echo "./scanfm -p 0...0 -d ra1d.txt"
echo "failed resulting in never ending loop"
echo "./scanfm -p 0...1 -d ra2d.txt"
echo "failed resulting in never ending loop"
echo "./scanfm -p 4...9 -d ra3d.txt"
./scanfm -p 4...9 -d ra3d.txt


echo "-----------------------------------------------------"
echo "Reference tests"
echo "-----------------------------------------------------"

echo "./scanfm -p p1=0...0 2...4 p1 -d re1d.txt"
./scanfm -p p1=0...0 2...4 p1 -d re1d.txt
echo "./scanfm -p p1=1...1 2...2 p1 -d re2d.txt"
./scanfm -p p1=1...1 2...2 p1 -d re2d.txt
echo "./scanfm -p p1=3...5 2...2 p1 -d re3d.txt"
./scanfm -p p1=3...5 2...2 p1 -d re3d.txt
echo "./scanfm -p p1=3...5 p1 -d re4d.txt"
./scanfm -p p1=3...5 p1 -d re4d.txt
echo "./scanfm -p p1=0...0 2...2 ~p1 -d re5d.txt"
./scanfm -p p1=0...0 2...2 ~p1 -d re5d.txt
echo "./scanfm -p p1=1...1 2...2 ~p1 -d re6d.txt"
./scanfm -p p1=1...1 2...2 ~p1 -d re6d.txt
echo "./scanfm -p p1=3...5 2...2 ~p1 -d re7d.txt"
./scanfm -p p1=3...5 2...2 ~p1 -d re7d.txt
echo "./scanfm -p p1=3...3 ~p1 -d re8d.txt"
./scanfm -p p1=3...3 ~p1 -d re8d.txt
echo "./scanfm -p p1=3...4 2...2 p1[2,1,1] -d re9d.txt"
./scanfm -p p1=3...4 2...2 p1[2,1,1] -d re9d.txt
echo "./scanfm -p p1=1...1 2...3 ~p1[0,1,0] -d re10d.txt"
./scanfm -p p1=1...1 2...3 ~p1[0,1,0] -d re10d.txt
echo "./scanfm -p p1=3...4 2...3 ~p1[1,0,1] -d re11d.txt"
./scanfm -p p1=3...4 2...3 ~p1[1,0,1] -d re11d.txt
echo "./scanfm -p p1=3...4 ~p1[1,0,0] -d re12d.txt"
./scanfm -p p1=3...4 ~p1[1,0,0] -d re12d.txt


echo "-----------------------------------------------------"
echo "Multiple PU's tests"
echo "-----------------------------------------------------"

echo "./scanfm -p ACGT TGCA -d mu1d.txt"
./scanfm -p ACGT TGCA -d mu1d.txt
echo "./scanfm -p 4...5 TTTT[1,1,1] -d mu2d.txt"
./scanfm -p 4...5 TTTT[1,1,1] -d mu2d.txt
echo "./scanfm -p TTTT[0,1,1] TTA -d mu3d.txt"
./scanfm -p TTTT[0,1,1] TTA -d mu3d.txt
echo "./scanfm -p p1=2...3 p2=2...2 p1 p2 -d mu4d.txt"
./scanfm -p p1=2...3 p2=2...2 p1 p2 -d mu4d.txt
echo "./scanfm -p p1=3...3 p2=2...3 2...3 p2 p1 -d mu5d.txt"
./scanfm -p p1=3...3 p2=2...3 2...3 ~p2 ~p1 -d mu5d.txt
