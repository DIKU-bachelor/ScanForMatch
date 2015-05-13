echo "-----------------------------------------------------"
echo "Sequence tests"
echo "-----------------------------------------------------"

echo "./scanfm -p   -d sq1d.txt"
./scanfm -p   -d sq1d.txt
echo "./scanfm -p A -d sq2d.txt"
./scanfm -p A -d sq2d.txt
echo "./scanfm -p ACCGT -d sq3d.txt"
./scanfm -p ACCGT -d sq3d.txt
echo "./scanfm -p ACC[1,0,0] -d sq4d.txt"
./scanfm -p ACC[1,0,0] -d sq4d.txt
echo "./scanfm -p ATG[0,1,0] -d sq5d.txt"
./scanfm -p ATG[0,1,0] -d sq5d.txt
echo "./scanfm -p ACC[0,0,1] -d sq6d.txt"
./scanfm -p ACC[0,0,1] -d sq6d.txt
echo "./scanfm -p ACTTT[1,2,0] -d sq7d.txt"
./scanfm -p ACTTT[1,2,0] -d sq7d.txt
echo "./scanfm -p ATTCCCTT[2,2,1] -d sq8d.txt"
./scanfm -p ATTCCCTT[2,2,1] -d sq8d.txt
echo "./scanfm -p R -d sq9d.txt"
./scanfm -p R -d sq9d.txt
echo "./scanfm -p RRGGWW -d sq10d.txt"
./scanfm -p RRGGWW -d sq10d.txt


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
