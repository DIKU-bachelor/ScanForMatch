import os
for file in os.listdir("."):
    if file.endswith(".txt"):
        print file
        f = open(file, "r")
        content = f.read()
        print content
        f.close()
        newCon = ">seq1 \n" + content;
        fw = open(file, "w")
        fw.write(newCon)
        fw.close()
