#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>

#define MAX_SEQ_LEN 250000000
#define MAX_PAT_LINE_LN 32000

/*  TEST program:

    scan_for_matches pattern < fasta_input > fasta_output
*/

/* ============================================================= */
/* ============================================================= */


char compl(c)
char c;
{
    switch (c)
    {
      case 'a':
	return 't';
      case 'A':
	return 'T';

      case 'c':
	return 'g';
      case 'C':
	return 'G';

      case 'g':
	return 'c';
      case 'G':
	return 'C';

      case 't':
      case 'u':
	return 'a';
      case 'T':
      case 'U':
	return 'A';

      case 'm':
	return 'k';
      case 'M':
	return 'K';

      case 'r':
	return 'y';
      case 'R':
	return 'Y';

      case 'w':
	return 'w';
      case 'W':
	return 'W';

      case 's':
	return 'S';
      case 'S':
	return 'S';

      case 'y':
	return 'r';
      case 'Y':
	return 'R';

      case 'k':
	return 'm';
      case 'K':
	return 'M';

      case 'b':
	return 'v';
      case 'B':
	return 'V';

      case 'd':
	return 'h';
      case 'D':
	return 'H';

      case 'h':
	return 'd';
      case 'H':
	return 'D';

      case 'v':
	return 'b';
      case 'V':
	return 'B';

      case 'n':
	return 'n';
      case 'N':
	return 'N';

      default:
	return c;
    }
}

int main(argc,argv)
int argc;
char *argv[];
{
    extern char *optarg;
    extern int optind;

    FILE *fp;
    char *data,*cdata;
    char tmp;
    int i,j,k,i1;
    char *hits[2000];
    char *p,*pc;
    char id[1000];
    char line[MAX_PAT_LINE_LN];
    int got_gt = 0;

    int stop_after = 100000000;
    int max_hits   = 100000000;
    int hit_in_line;

    int show_overlaps=0;
    int complements=0;
    int protein=0;
    int errflag = 0;
    FILE *ig_fp = NULL;
    char *ignore[20000];
    int ig_index=0;
    int ln;

    int c;

    while ((c = getopt(argc, argv, "pcnmo:i:")) != -1)
	switch (c) {
	  case 'o':
	    show_overlaps=1;
	    break;

	  case 'c':
	    complements=1;
	    break;

	  case 'p':
	    protein=1;
	    break;

	  case 'n':
	    if (sscanf(optarg,"%d",&stop_after) != 1)
	    {
		errflag=1;
		fprintf(stderr,"invalid value on -n option (make it a positive integer)\n");
	    }
	    break;

	  case 'm':
	    if (sscanf(optarg,"%d",&max_hits) != 1)
	    {
		errflag=1;
		fprintf(stderr,"invalid value on -m option (make it a positive integer)\n");
	    }
	    break;

	  case 'i':
	    if ((ig_fp = fopen(optarg,"r")) == NULL)
	    {
		errflag=1;
		fprintf(stderr,"invalid file name for ids to ignore\n");
	    }
	    break;
	}
    
    if (errflag || (optind >= argc) || ((fp = fopen(argv[optind],"r")) == NULL))
    {
	fprintf(stderr,"errflag=%d optind=%d argc=%d\n",errflag,optind,argc);
	fprintf(stderr, "usage: scan_for_matches -c [for complementary strand] -p [protein][-n stop_after_n_misses] [-m max_hits] [-i file_of_ids_to_ignore] -o [overlapping hits] pattern_file < fasta_input > hits\n");
	exit (2);
    }

    if (ig_fp)
    {
	while (fscanf(ig_fp,"%s",id) == 1)
	{
	    if ((ignore[ig_index] = malloc(strlen(id)+1)) == NULL)
	    {
		fprintf(stderr,"memory allocation error\n");
		exit(1);
	    }
	    strcpy(ignore[ig_index++],id);
	}
	close(ig_fp);
	if (ig_index)
	    fprintf(stderr,"ignoring %d id(s)\n",ig_index);
    }

   if (((data  = malloc(MAX_SEQ_LEN+1)) == NULL) ||
       ((cdata  = malloc(MAX_SEQ_LEN+1)) == NULL))
   {
       fprintf(stderr,"failed to alloc memory\n");
   }

   pc = line;
   i = fgetc(fp);
   while ((i != EOF) && (pc < line+(MAX_PAT_LINE_LN-1)))
   {
       if (i == '%')
       {
	   while (((i = fgetc(fp)) != '\n') && (i != EOF))
	       ;
	   if (i != EOF)
	       i = ' ';
       }
       else if (i == '\n')
	   i = ' ';
       else
       {
	   *(pc++) = i;
	   i = fgetc(fp);
       }
   }
   *pc = 0;
   close(fp);
   if ((protein && !parse_peptide_cmd(line)) || (!protein && !parse_dna_cmd(line)))
   {
       fprintf(stderr,"failed to parse pattern: %s\n",line);
       exit(1);
   }
/*      printf("names[1] = %x\n",names[1]); */

    while ((max_hits > 0) && ((!got_gt && (fscanf(stdin,">%s",id) == 1)) ||
	   (got_gt && (fscanf(stdin,"%s",id) == 1))))
    {
/*	printf("processing %s\n",id); */
        while (getc(stdin) != '\n')
	  ;
	for (p=data; ((i = getc(stdin)) != -1) && (i != '>');)
	{
	    if ((i != ' ') && (i != '\n'))
	      *(p++) = i;
	}
	if (i == '>')
	    got_gt = 1;
	else
	    got_gt = 0;

        *p=0;

	for (i=0; (i < ig_index) && (strcmp(ignore[i],id) != 0); i++)
	    ;
	
        if (i == ig_index)
	{
	    if (!protein)
		comp_data(data,cdata);
	    else
		strcpy(cdata,data);
	    
	    ln = strlen(data);
	    /*	printf("names[1] = %x\n",names[1]); */
	    for (hit_in_line=0, i = first_match(cdata,ln,hits);
		 (max_hits > 0) && (i > 0);
		 /*		 i = cont_match(hits) */ ) 
/*		 i = next_match(hits)) */ /*   to get overlapping hits */
	    {
	        hit_in_line = 1; max_hits--;
	      
		printf(">%s:[%ld,%ld]\n",id,1+hits[0]-cdata,1+(hits[i] - 1 - cdata));
		for (i1=0; i1 < i; i1++)
		{
		    j = hits[i1+1] - hits[i1];
		    for (p=data + (hits[i1]-cdata); j; j--)
			printf("%c",*(p++));
		    printf(" ");
		}
		printf("\n");
		if (! show_overlaps)
		{
		    i = cont_match(hits);
		}
		else
		{
		    i = next_match(hits);
		}
	    }

	    if (complements)
	    {
		for (i=0, j=ln-1; i <= j; i++,j--)
		{
		    tmp = compl(data[i]);
		    data[i] = compl(data[j]);
		    data[j] = tmp;
		}
		comp_data(data,cdata);

		/*	printf("names[1] = %x\n",names[1]); */
		for (i = first_match(cdata,ln,hits);
		     (max_hits > 0) && (i > 0);
		     i = cont_match(hits))
		    /*		 i = next_match(hits))     to get overlapping hits */
		{
		    hit_in_line = 1; max_hits--;
		  
		    printf(">%s:[%ld,%ld]\n",id,1+(ln-1) - (hits[0]-cdata),1+(ln-1) - (hits[i] - 1 - cdata));
		    for (i1=0; i1 < i; i1++)
		    {
			j = hits[i1+1] - hits[i1];
			for (p=data + (hits[i1]-cdata); j; j--)
			    printf("%c",*(p++));
			printf(" ");
		    }
		    printf("\n");
		}
	    }

	    if (!hit_in_line)
	    {
		if (--stop_after == 0)
		{
		    fprintf(stderr,"exceeded limit of lines failing to match\n");
		    exit(1);
		}
	    }
	}
    }
    /*    printf("successfully completed\n"); */

    return(EXIT_SUCCESS);
}

