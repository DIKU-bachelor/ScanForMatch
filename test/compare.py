import sys;

def deformat_line(line):
  return line[line.find(":[") + 2 : line.find(",")] + "  " \
    + line[line.find("\n") + 1:].replace(" ", "");

def compare_files(scanfm_fname, scan_for_matches_fname):
  f1 = open(scanfm_fname, 'r');
  f2 = open(scan_for_matches_fname, 'r');
  line1 = '';
  line2 = '';
  fail = 0;
  counter = 0
  while(True):
    counter += 1;
    line1 = f1.readline();
    line2 = f2.readline();
    line2 += f2.readline();
    dline = deformat_line(line2);
    if (line1 == '' and line2 == ''):
      if (not fail):
        print "PASSED";
      return;
    if (line1 == ''):
      fail = 1;
      print "FAILED: scanfm missing match: " + dline;
      return;
    if (line2 == ''):
      fail = 1;
      print "FAILED: scan_for_matches missing match: " \
        + line1;
      return;
    if (dline != line1):
      fail = 1;
      print "FAILED: matches nr " + str(counter) + " not equal:\nscanfm:           " + line1 \
        + "\nscan_for_matches: " + dline;
      return

compare_files(sys.argv[1], sys.argv[2]);
