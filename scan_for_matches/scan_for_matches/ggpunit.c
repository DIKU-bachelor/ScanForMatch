#include <stdlib.h>
/*  NOTES: June 30, 1992

    These result from considering what changes will need to be made
    to allow this to be effectively called from OR-parallel Prolog systems.


    1. The comments on what parse_dna_cmd and parse_peptide_cmd returned
       were wrong (they said 1, but it returns maximum # of matched chars).
       Why return max # matched?

    2. The routines should "auto-initialize".  This is probably not done,
       due to GenoGraphics and IGD requiring slighly differing 
       initializations.  These should be made consistent (code_to_punit goes
       into ggpunit.c

    3. Forms of parse_cmd, parse_dna_cmd, and parse_peptide_cmd should be
       written to return the PUs (and BR) in dynamically allocated memory.

    4. Forms of first_match, next_match, and pattern_match need to be
       written that operate on the dynamically allocated state.

For better or worse, I did these changes. 6/30/92 RAO
*/

/* ggpunit.c contains routines to parse a string and store it as a punit
   and to search encoded uncompressed DNA or uncoded protein sequence
*/
/* Style:
   Every function must have a prototype inside the #ifndef CC
   Where it is defined, each function must use the old style for
   typing parameters, i.e.:

char *char_pat(n,p)
int *n;
char *p;
{
   ....
}

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX(A,B) (((A))>((B)) ? ((A)) : ((B)))

FILE *diag_file;

/* the following variable hides the relationship between parsing punits
   and executing searches.  These routines compose the interface:

   int parse_dna_cmd(line)      returns max # characters that could be matched
                                    if successful parse (sets ad_pu_s)
   char *line;                  returns 0 if unsuccessful parse

   int parse_peptide_cmd(line)  returns max # characters that could be matched
                                    if successful parse (sets ad_pu_s)
   char *line;                  returns 0 if unsuccessful parse

   int first_match(start,len,hits)  0 on miss; # pattern units on hit
   char *start;             start of search
   int len;                 length of search
   char *hits[];            on hit, set to vector of starts of punits +
                                just past the end of last punit, so if
                                the routine returns N, hits will have N+1
                                entries.  Each entry will the point where
                                a punit matched, except the last, which
                                is just past the last punit

   int next_match(hits)      continues search
                            (returns 0 or # pattern units to indicate success)

   int cont_match(hits)      continues search (non-overlapping)
                            (returns 0 or # pattern units to indicate success)

   char *hits[];              set on success
*/

char punit_to_code[256];
char code_to_punit[256];
int initialized=0;

char known_char[16] = {0,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0};
char known_char_index[16] =
                 {-1,0,1,-1,2,-1,-1,-1,3,-1,-1,-1,-1,-1,-1,-1};

int punit_sequence_type;

#define PEPTIDE 1
#define DNA 2

#define BOOL  int
#define TRUE  1
#define FALSE 0

#define MAX_SOUGHT_CHARS 1000000
#define MAX_PUNITS 100
#define MAX_CODES 100   /* max length of a single punit being matched to */
#define MAX_WEIGHTS 2000 /* max weight storage */
#define MAX_TOTAL_CODES 600 /* max character info storage */
#define MAX_NAMES    50
typedef struct weight_vec {
    int aw,cw,gw,tw;
} wv_t;

struct punit {
    int type;
    struct punit *nxt_punit,*prev_punit;
    struct punit *BR;
    int anchored;
    char *hit;
    int mlen;   /* how much matched */
    union {
        struct or_info_struct {
            struct punit *or1, *or2;
	    char *SR;
	    int alt;
        } or;
        struct exact_info_struct {
            int len;
            char *code;
        } exact;
        struct any_struct {
            long code_matrix;
        } any;
        struct char_info_struct {
            int len;
            char *code;
        } char_match;
        struct range_info_struct {
            int min, width;  /* width of x...y is y-x */
            int nxt;
        } range;
        struct compl_info_struct {
            int ins, del, mis, of,rule_set;
        } compl;
        struct repeat_info_struct {
            int ins, del, mis, of;
        } repeat;
        struct sim_info_struct {
            int ins, del, mis, len;
            char *code;
        } sim;
        struct llim_struct {
	    int llim_vec[MAX_NAMES+1];
            int bound;
	} llim;
        struct weight_info_struct {
            int len;
            int *vec;
            int cutoff;
            int tupsz;
	    int maxwt;
        } wvec;
    } info;
};

typedef struct punit punit_t;



/* state is used to retain state for a search -- needed for parallel
   Prolog versions
*/
struct state {
    punit_t pu_s[MAX_PUNITS];
    char cv[MAX_TOTAL_CODES];
    int iv[21*MAX_WEIGHTS/2];
    punit_t *BR;   
    punit_t *ad_pu_s;
    char *past_last;
};


/* ============================================================= */
struct punit *ad_pu_s = NULL;  /* CONNECTS PARSER TO SCAN ROUTINES */
char *start_srch, *end_srch, *past_last;

struct punit *names[MAX_NAMES];
punit_t *BR1;
char *rule_sets[MAX_NAMES];

/* prototypes */
#ifdef CC
#define PROTO(X) ()
#else
#define PROTO(X) X
#endif

void free_state PROTO((struct state *st));
void reverse_punit PROTO((char *buff));
void reverse_one_punit PROTO((punit_t *pu,char **buff));
char *num PROTO((int *n, char *p));
int next_match PROTO((char *hits[]));
int max_mat PROTO((struct punit *pu));
void set_anchors PROTO((struct punit *pu));
void set_anchors_on PROTO((struct punit *pu));
punit_t *parser PROTO((punit_t **pu_s,char **cv,int **iv,char *line));
char *punit_cons_list PROTO((punit_t **pu_s,char **cv,int **iv,char *p));
char *punit_cons PROTO((punit_t **pu_s,char **cv,int **iv,char *p));
char *punit_list PROTO((punit_t **pu_s,char **cv,int **iv,char *p));
char *punit_parse PROTO((punit_t **pu_s,char **cv,int **iv,char *p));
char *or_pat PROTO((punit_t **pu_s,char **cv,int **iv,char *p));
char *char_pat PROTO((char **enc_pat,char *p));
char *range_pat PROTO((int *min,int *max,char *p));
char *elipses PROTO((char *p));
char *wt_pat PROTO((int *maxwt, int *cutoff,int *tupsz,int **t,int *n,char *p));
char *wt_template PROTO((int *tupsz,int **t,int *n,char *p));
char *n_tuple PROTO((int *n,int *t,char *p));
char *repeat_pat PROTO((int *n,int *mis,int *ins,int *del,char *p));
char *inv_rep_pat PROTO((int *n,int *mis,int *ins,int *del,char *p));
char *compl_pat PROTO((int *rs,int *n,int *mis,int *ins,int *del,char *p));
char *rule_id PROTO((int *rs,char *p));
char *compl_id PROTO((int *rs,int *n,char *p));
char *sim_pat PROTO((char **enc_pat,int *mis,int *ins,int *del,char *p));
char *misinsdel PROTO((int *mis,int *ins,int *del,char *p));
char *dna_pat PROTO((char **enc_pat,char *p));
char *name_assgn PROTO((int *n,char *p));
char *name_id PROTO((int *n, char *p));
char *white_space PROTO((char *p));
int parse_cmd PROTO((char *line));
char *wt_pat PROTO((int *maxwt,int *cutoff,int *tupsz,int **t,int *n,char *p));
void rev_compl_data PROTO((int rule_set,unsigned char *data,int len,unsigned char *result));
int loose_match PROTO((unsigned char *one_data,int one_len,
                unsigned char *two_data, int two_len,
                int max_ins, int max_del, int max_mis, int rule_set,
                int *match_range, int compl_flag));
char *punit_cons_list PROTO((punit_t **pu_s,char **cv,int **iv,char *p));
int max_mats PROTO((struct punit *pu));
int cont_match PROTO((char *hits[]));
int pattern_match PROTO((punit_t *pu,char *start,char *end,char *hits[],int first, punit_t **BR1));
int collect_hits PROTO((struct punit *pu,char **revhits));
/* char *malloc(int ln);    NOTE: on some platforms this needs to be commented out */

/* ============================================================= */
struct state *get_state()
{
    return((struct state *) malloc(sizeof(struct state)));
}

void free_state(st)
struct state *st;
{
    free(st);
}

/* ============================================================= */

#define EXACT_PUNIT    0          /* punit_t.type */
#define RANGE_PUNIT    1          /* punit_t.type */
#define COMPL_PUNIT    2          /* punit_t.type */
#define REPEAT_PUNIT   3          /* punit_t.type */
#define SIM_PUNIT      4          /* punit_t.type */
#define WEIGHT_PUNIT   5          /* punit_t.type */
#define OR_PUNIT       6          /* punit_t.type */
#define ANY_PUNIT      8          /* punit_t.type */
#define LLIM_PUNIT     9          /* length limit punit */
#define INV_REP_PUNIT 10          /* for real palindromic inverted repeats */
#define MATCH_START   11          /* ^ */
#define MATCH_END     12          /* $ */

#define A_BIT 0x01              /* char bitfield */
#define C_BIT 0x02              /* char bitfield */
#define G_BIT 0x04              /* char bitfield */
#define T_BIT 0x08              /* char bitfield */

char *white_space(p)
char *p;
{
    while (isspace(*p))
        p++;
    return(p);
}

char *num(n,p)
int *n;
char *p;
{
    int i;
    int sign;
    
    if (*p == '-')
    {
	sign = -1;
	p++;
    }
    else
	sign=1;
    
    if ((*p < '0') || (*p > '9'))
        return NULL;
    else
    {
        i = *(p++) - '0';
        while ((*p >= '0') && (*p <= '9'))
            i = (i * 10) + (*(p++) - '0');
        *n = i*sign;
        return p;
	
    }
}

char *name_id(n,p)
int *n;
char *p;
{
    if ((*(p++) == 'p') && (p = num(n,p)) && (*n >= 0) && (*n <= MAX_NAMES))
        return p;
    else
        return NULL;
}

char *name_assgn(n,p)
int *n;
char *p;
{
    if ((p = name_id(n,p)) && (p = white_space(p)) && (*(p++) == '='))
        return p;
    else
        return NULL;
}

char *dna_pat(enc_pat,p)
char **enc_pat;
char *p;
{
    char *p1;

    if (punit_sequence_type!=DNA)
       return(NULL);
    for (p1 = *enc_pat;
         (*p != '\0') && (*p != ' ')  && (*p != '\t')  && (*p != '[') && (*p != '\n') && (*p != ')');
         p++)
    {
        *p1 = punit_to_code[*p];
        if (*p1)
            p1++;
        else
            return NULL;
    }
    if (p1 > *enc_pat)
    {
        *p1 = '\0'; *enc_pat = p1+1;
        return p;
    }
    else
    {
        return NULL;
    }
}

char *misinsdel(mis,ins,del,p)
int *mis,*ins,*del;
char *p;
{
    if ((*(p++) == '[') && (p = num(mis,p)) && (*(p++) == ',') &&
                           (p = num(ins,p)) && (*(p++) == ',') &&
                           (p = num(del,p)) && (*(p++) == ']'))
        return p;
    else
        return NULL;
}

char *sim_pat(enc_pat,mis,ins,del,p)
char **enc_pat,*p;
int *mis,*ins,*del;
{
    char *p1;

    if (((punit_sequence_type == DNA) && (p1 = dna_pat(enc_pat,p))) ||
	((punit_sequence_type == PEPTIDE) && (p1 = char_pat(enc_pat,p))))
    {
        if (p = misinsdel(mis,ins,del,p1))
            return p;
        else
        {
            *mis = *ins = *del = 0;
            return p1;
        }
    }
    else
        return NULL;
}

char *compl_id(rs,n,p)
int *rs;
int *n;
char *p;
{
    if(punit_sequence_type!=DNA)
       return(NULL);

    if (*p == '~')
    {
	*rs = -1; 
        return name_id(n,p+1);
    }
    else if ((p=rule_id(rs,p)) && (*p == '~'))
       return name_id(n,p+1);
    else
        return NULL;
}

char *compl_pat(rs,n,mis,ins,del,p)
int *rs,*n,*mis,*ins,*del;
char *p;
{
    char *p1;

    if(punit_sequence_type!=DNA)
       return(NULL);

    if (p = compl_id(rs,n,p))
    {
        if (p1 = misinsdel(mis,ins,del,p))
            return p1;
        else
        {
            *mis = *ins = *del = 0;
            return p;
        }
    }
    else
        return NULL;
}

char *repeat_pat(n,mis,ins,del,p)
int *n,*mis,*ins,*del;
char *p;
{
    char *p1;

    if (p = name_id(n,p))
        if (p1 = misinsdel(mis,ins,del,p))
            return p1;
        else
        {
            *mis = *ins = *del = 0;
            return p;
        }
    else
        return NULL;
}

char *inv_rep_pat(n,mis,ins,del,p)
int *n,*mis,*ins,*del;
char *p;
{
    char *p1;

    if (*(p++) != '<') return NULL;
    
    if (p = name_id(n,p))
        if ((punit_sequence_type == DNA) && (p1 = misinsdel(mis,ins,del,p)))
            return p1;
        else
        {
            *mis = *ins = *del = 0;
            return p;
        }
    else
        return NULL;
}

char *n_tuple(n,t,p)
int *n,*t;
char *p;
{
    if ((*(p++) == '(') && (p = white_space(p)) && (p = num(t,p)))
    {
        *n = 1;
        while (TRUE)
        {
            if (*p == ',')
            {
                p++;
		p = white_space(p);
                if (p = num(t+(*n),p))
		{
                    (*n)++;
		    p = white_space(p);
		}
                else
                    return NULL;
            }
            else if (*(p++) == ')')
                return p;
            else
                return NULL;
        }
    }
    else
        return NULL;
}

char *wt_template(tupsz,t,n,p)
int *tupsz,**t,*n;
char *p;
{
    int sz;

    *n = 0;
    if ((*(p++) == '{') && (p = white_space(p)) && (p = n_tuple(tupsz,*t,p)))
    {
        *t += *tupsz; (*n)++;
        while ((p = white_space(p)) && (*p == ','))
        {
            p++;
            if ((p = white_space(p)) &&
                (p = n_tuple(&sz,*t,p)) && (sz == *tupsz))
            {
                (*n)++;
                *t += sz;
            }
            else
                return NULL;
        }
        if ((*(p++) == '}') && ((*tupsz == 4) || (*tupsz == 20) || (*tupsz == 21)))
            return p;
        else
            return NULL;
    }
    else
        return NULL;
}

char *wt_pat(maxwt,cutoff,tupsz,t,n,p)
int *maxwt,*cutoff,*tupsz,**t,*n;
char *p;
{
  char *p1;

    if ((p1 = num(maxwt,p)) && (p1 = white_space(p1)) && (*(p1++) == '>') && 
	(p1 = white_space(p1)) && (p1 = wt_template(tupsz,t,n,p1)) && (p1 = white_space(p1)) &&
        (*(p1++) == '>') && (p1 = white_space(p1)) && (p1 = num(cutoff,p1)))
    {
      return p1;
    }

    *maxwt = 1000000;
    
    if ((p = wt_template(tupsz,t,n,p)) && (p = white_space(p)) &&
        (*(p++) == '>') && (p = white_space(p)) && (p = num(cutoff,p)))
        return p;
    else
        return NULL;
}

char *elipses(p)
char *p;
{
    if ((*(p++) == '.') && (*(p++) == '.') && (*(p++) == '.'))
        return p;
    else
        return NULL;
}

char *word(str,p)
char *str;
char *p;
{
    while (*str)
    {
      if (*(str++) != *(p++))
	return NULL;
    }
    return p;
}
/*

    matches things like
  
            length(p1+p2+p6) < 13
*/    
char *llim_pat(v,max,p)
int *v,*max;
char *p;
{
  char *p1;

  *v = 1; 
  if ((p = word("length(",p)) && (p = name_id(v + 1,p)))
  {
    while ((p1=white_space(p)) && (*(p1++) == '+') && (p1=white_space(p1)) &&
           (p1 = name_id((v+1) + ((*v)++),p1)))
      p = p1;

    if ((p = white_space(p)) && (*(p++) == ')') && (p = white_space(p)) && (*(p++) == '<') && (p = white_space(p)) &&
        (p = num(max,p)))
        return p;
    else
        return NULL;
  }
  return NULL;
}
      
char *range_pat(min,max,p)
int *min,*max;
char *p;
{
    if ((p = num(min,p)) && (p = white_space(p)) && (p = elipses(p)) &&
        (p = white_space(p)) && (p = num(max,p)))
        return p;
    else
        return NULL;
}

char *char_pat(enc_pat,p)
char **enc_pat,*p;
{
    char *p1;

    if (punit_sequence_type==PEPTIDE && isalpha(*p))
    {
        for (p1 = *enc_pat;
             isalpha(*p);
             p++)
        {
            *(p1++) = (char) toupper((int) *p);
        }

        *p1 = '\0'; *enc_pat = p1+1;
        return p;
    }
    else
        return NULL;
}

char *any_pat(acm,p)
char *p;
long *acm;
{
    char *p1;
    int i;
    long cm;

    if (punit_sequence_type==PEPTIDE && strncmp("any(",p,4) == 0)
    {
	p += 4;
        for (cm=0; isalpha(*p); p++)
        {
	    cm |= 1 << (toupper((int) *p) - 'A');
        }
	if (*(p++) == ')')
	{
	    *acm=cm;
	    return p;
	}
	else
	    return NULL;
    }
    else if (punit_sequence_type==PEPTIDE && strncmp("notany(",p,7) == 0)
    {
	p += 7;
        for (cm=0x3ffffff;
             isalpha(*p);
             p++)
        {
	    cm &= ~((long) (1 << (toupper((int) *p) - 'A')));
        }
	if (*(p++) == ')')
	{
	    *acm=cm;
	    return p;
	}
	else
	    return NULL;
    }
    else
        return NULL;
}

char *or_pat(pu_s,cv,iv,p)
punit_t **pu_s;
char **cv,*p;
int **iv;
{
    punit_t *or_pu,*p1,*p2;
    char *punit_cons_list();

    or_pu = (*pu_s)++;
    if ((*(p++) == '(') && (p = white_space(p)) &&
        (p1 = *pu_s) && (p = punit_cons_list(pu_s,cv,iv,p)) &&
        (p = white_space(p)) && (*(p++) == '|') && (p = white_space(p)) &&
        (p2 = *pu_s) && (p = punit_cons_list(pu_s,cv,iv,p)) &&
        (p = white_space(p)) && (*(p++) == ')'))
    {
        or_pu->type = OR_PUNIT;
        or_pu->info.or.or1 = p1;
        or_pu->info.or.or2 = p2;
        p1->prev_punit = p2->prev_punit = or_pu;
        or_pu->nxt_punit = or_pu->prev_punit = NULL;
        return(p);
    }
    else
    {
        *pu_s = or_pu;
        return NULL;
    }
}

char *dna_char(p,c)
char *p,*c;
{
    switch (*p)
    {
      case 'a':
      case 'A':
	*c = 0; return p+1;

      case 'c':
      case 'C':
	*c = 1; return p+1;

      case 'g':
      case 'G':
	*c = 2; return p+1;

      case 't':
      case 'T':
      case 'u':
      case 'U':
	*c = 3; return p+1;

      default:
	return NULL;
    }
}

char *bond(p,rs)
char *p;
char rs[16];
{
    char c1,c2;

    if ((p = dna_char(p,&c1)) && (p = dna_char(p,&c2)))
    {
	rs[(c1 << 2) + c2] = 1;
	return p;
    }
    else
	return NULL;
}

char *parse_bonds(p,rs)
char *p;
char rs[16];
{
    if (p = bond(p,rs))
    {
	while ((*p == ',') && (p = bond(p+1,rs)))
	    ;
	return p;
    }
    else
	return NULL;
}

char *bond_set(p,n)
char *p;
int n;
{
    char rs[16];
    int i,j;

    for (i=0; i < 16; i++)
	rs[i]=0;

    if ((*(p++) == '{') && (p=parse_bonds(p,rs)) && (*(p++) == '}'))
    {
	if (!rule_sets[n])
	{
	    if ((rule_sets[n] = malloc(16)) == NULL)
	    {
		fprintf(stderr,"memory allocation failure\n");
		exit(1);
	    }
	}
	for (i=0; i < 16; i++)
	    *(rule_sets[n]+i) = rs[i];
	return p;
    }
    else
	return NULL;
}

char *rule_id(n,p)
int *n;
char *p;
{
    if ((*(p++) == 'r') && (p = num(n,p)) && (*n >= 0) && (*n <= MAX_NAMES))
        return p;
    else
        return NULL;
}

char *parse_rule_set(p)
char *p;
{
    int n;

    if(punit_sequence_type!=DNA)
       return(NULL);

    if ((p = rule_id(&n,p)) && (p = white_space(p)) && (*(p++) == '=') && (p=bond_set(p,n)))
        return p;
    else
        return NULL;
}

char *punit_parse(pu_s,cv,iv,p)
punit_t **pu_s;
char **cv,*p;
int **iv;
{
    punit_t *pu1;
    int i,j,k,n,rs;
    char *p1,*p2;
    int *i1;
    int tupsz;

    while (p1 = parse_rule_set(p))
	p = white_space(p1);

    pu1 = (*pu_s)++;
    if (p1 = name_assgn(&i,p))
    {
        if(names[i]) return(NULL); /* ensure only one occurrence of pN= */
        names[i] = (struct punit *) pu1;
        p = p1;
    }
    p = white_space(p);
    if (*p == '^')
    {
	pu1->type = MATCH_START;
	p1 = p+1;
    }
    else if (*p == '$')
    {
	pu1->type = MATCH_END;
	p1 = p+1;
    }
    else if (p1 = range_pat(&i,&j,p))
    {
        pu1->type = RANGE_PUNIT;
        pu1->info.range.min = i;
        pu1->info.range.width = j-i;
    }
    else if (p1 = any_pat(&(pu1->info.any.code_matrix),p))
    {
        pu1->type = ANY_PUNIT;
    }
    else if (p1 = llim_pat(pu1->info.llim.llim_vec,&(pu1->info.llim.bound),p))
    {
        pu1->type = LLIM_PUNIT;
    }
    else if (p1 = inv_rep_pat(&n,&i,&j,&k,p))
    {
        pu1->type = INV_REP_PUNIT;
        pu1->info.repeat.of = n;
        pu1->info.repeat.mis = i;
        pu1->info.repeat.ins = j;
        pu1->info.repeat.del = k;
    }
    else if (p1 = repeat_pat(&n,&i,&j,&k,p))
    {
        pu1->type = REPEAT_PUNIT;
        pu1->info.repeat.of = n;
        pu1->info.repeat.mis = i;
        pu1->info.repeat.ins = j;
        pu1->info.repeat.del = k;
    }
    else if ((p2 = *cv) && (p1 = sim_pat(cv,&i,&j,&k,p)))
    {
        if ((i == 0) && (j == 0) && (k == 0))
        {
            pu1->type = EXACT_PUNIT;
            pu1->info.exact.code = p2;
            pu1->info.exact.len = strlen(p2);
        }
        else
        {
            pu1->type = SIM_PUNIT;
            pu1->info.sim.code = p2;
            pu1->info.sim.len = strlen(p2);
            pu1->info.sim.mis = i;
            pu1->info.sim.ins = j;
            pu1->info.sim.del = k;
        }
    }
    else if (p1 = compl_pat(&rs,&n,&i,&j,&k,p))
    {
        pu1->type = COMPL_PUNIT;
        pu1->info.compl.rule_set = rs;
        pu1->info.compl.of = n;
        pu1->info.compl.mis = i;
        pu1->info.compl.ins = j;
        pu1->info.compl.del = k;
    }
    else if ((i1 = *iv) && (p1 = wt_pat(&k,&i,&tupsz,iv,&j,p)))
    {
        pu1->type = WEIGHT_PUNIT;
        pu1->info.wvec.vec = i1;
        pu1->info.wvec.len = j;
        pu1->info.wvec.cutoff = i;
        pu1->info.wvec.maxwt = k;
        pu1->info.wvec.tupsz = tupsz;
    }
    else
    {
        *pu_s = pu1;
        return NULL;
    }
    return(p1);
}

char *punit_list(pu_s,cv,iv,p)
punit_t **pu_s;
char **cv;
int **iv;
char *p;
{
    punit_t *last, *next;
    char *p1;

    last = NULL;

    p = white_space(p);
    next = *pu_s;
    while (p1 = punit_parse(pu_s,cv,iv,p))
    {
        next->prev_punit = last;
        if (last)
            last->nxt_punit = next;
        last = next;
        next = *pu_s;
        p = white_space(p1);
    }
    if (!last)
        return NULL;
    else
    {
        last->nxt_punit = NULL;
        return p;
    }
}

char *punit_cons(pu_s,cv,iv,p)
punit_t **pu_s;
char **cv,*p;
int **iv;
{
    char *punit_list();
    char *or_pat();
    char *p1;

    while (p1 = parse_rule_set(p))
	p = white_space(p1);

    if (p1 = or_pat(pu_s,cv,iv,p))
        return p1;
    else
        return punit_list(pu_s,cv,iv,p);
}

char *punit_cons_list(pu_s,cv,iv,p)
punit_t **pu_s;
char **cv;
int **iv;
char *p;
{
    punit_t *last, *next;
    char *p1;

    last = NULL;

    p = white_space(p);
    next = *pu_s;
    while (p1 = punit_cons(pu_s,cv,iv,p))
    {
        next->prev_punit = last;
        if (last)
            last->nxt_punit = next;

        while (next->nxt_punit)
            next = next->nxt_punit;

        last = next;
        next = *pu_s;
        p = white_space(p1);
    }
    if (!last)
        return NULL;
    else
    {
        last->nxt_punit = NULL;
        return p;
    }
}

punit_t *parser(pu_s,cv,iv,line)
punit_t **pu_s;
char **cv;
int **iv;
char *line;
{
    punit_t *first;

    first = *pu_s;
    first->prev_punit = NULL;

    line = punit_cons_list(pu_s,cv,iv,white_space(line));
    if(line && (line=white_space(line)) && *line=='\0') /* read all of it */
        return first;
    else
        return NULL;
}

void set_anchors_on(pu)
struct punit *pu;
{
    pu->anchored = 1;
    if (pu->type == OR_PUNIT)
    {
        set_anchors_on(pu->info.or.or1);
        set_anchors_on(pu->info.or.or2);
    }
    if (pu=pu->nxt_punit)
    {
        set_anchors_on(pu);
    }
}

void set_anchors(pu)
struct punit *pu;
{
    pu->anchored = 0;
    if (pu->type == OR_PUNIT)
    {
        set_anchors(pu->info.or.or1);
        set_anchors(pu->info.or.or2);
    }

    if (pu=pu->nxt_punit)
    {
        set_anchors_on(pu);
    }
}

int parse_dna_cmd(line)
char *line;
{
   punit_sequence_type = DNA;
   return(parse_cmd(line));
}

int parse_peptide_cmd(line)
char *line;
{
   punit_sequence_type = PEPTIDE;
   return(parse_cmd(line));
}

int parse_cmd(line)
char *line;
{
    static punit_t pu_s[MAX_PUNITS];
    static char cv[MAX_TOTAL_CODES];
    static int iv[21*MAX_WEIGHTS/2];

    return(parse_cmd_ns(line,pu_s,cv,iv));
}

int parse_dna_cmd_ns(line,st)
char *line;
struct state **st;
{
   struct state *st1;
   int rc;

   st1 = get_state();
   *st = st1;

   punit_sequence_type = DNA;
   rc=parse_cmd_ns(line,st1->pu_s,st1->cv,st1->iv);
   st1->ad_pu_s = ad_pu_s;
   return rc;
}

int parse_peptide_cmd_ns(line,st)
char *line;
struct state **st;
{
   struct state *st1;
   int rc;

   st1 = get_state();
   *st = st1;

   punit_sequence_type = PEPTIDE;
   rc=parse_cmd_ns(line,st1->pu_s,st1->cv,st1->iv);
   st1->ad_pu_s = ad_pu_s;
   return rc;
}

int parse_cmd_ns(line,pu_s,cv,iv)
char *line;
punit_t pu_s[MAX_PUNITS];
char cv[MAX_TOTAL_CODES];
int iv[21*MAX_WEIGHTS/2];
{
    char *ad_cv;
    int  *ad_iv;

    int i;
    
    if (!initialized)
	build_conversion_tables();

    for (i=0; i < MAX_NAMES; i++)
        names[i] = NULL;

    ad_iv = iv; ad_cv = cv; ad_pu_s = (struct punit *) pu_s;

    if (((int) strlen(line)) >= ((int) MAX_SOUGHT_CHARS)) 
    {
        ad_pu_s = NULL;
        return 0;
    }
    else
    {
        if (parser(&ad_pu_s,&ad_cv,&ad_iv,line) == NULL)
        {
/*          fprintf(stderr,"Sorry, could not parse %s\n",line);   */
            ad_pu_s = NULL;
            return 0;
        }
        else
        {
            set_anchors(pu_s);
            ad_pu_s = (struct punit *) pu_s;
            return max_mats(pu_s);
        }
    }
}

int max_mat(pu)
struct punit *pu;
{
    struct punit *p1;
    long x;
    int i;

    switch (pu->type)
    {
      case EXACT_PUNIT:
          return pu->info.exact.len;
          break;

      case LLIM_PUNIT:
	  return 0;
	  break;

      case RANGE_PUNIT:
          return pu->info.range.min + pu->info.range.width;
          break;

      case ANY_PUNIT:
          return 1;
          break;

      case COMPL_PUNIT:
          p1 = names[pu->info.compl.of];
          return (max_mat(p1) + pu->info.compl.ins);
          break;

      case REPEAT_PUNIT:
      case INV_REP_PUNIT:
          p1 = names[pu->info.repeat.of];
          return (max_mat(p1) + pu->info.repeat.ins);
          break;

      case SIM_PUNIT:
          return (pu->info.sim.len + pu->info.sim.ins);
          break;

      case WEIGHT_PUNIT:
          return pu->info.wvec.len;
          break;

      case OR_PUNIT:
          return MAX(max_mat(pu->info.or.or1),max_mat(pu->info.or.or2));
          break;
      }
    return(0);
}

int max_mats(pu)
struct punit *pu;
{
    int sum = 0;

    while (pu)
    {
        sum += max_mat(pu);
        pu = pu->nxt_punit;
    }
    return sum;
}

int first_match(start,len,hits)
char *start;
int len;
char *hits[];
{
    int i;

    if (ad_pu_s == NULL)
	return(-1);
    start_srch = start; end_srch = start+(len-1);
    BR1=NULL;
    i = pattern_match(ad_pu_s,start_srch,end_srch,hits,1, &BR1);
    past_last = hits[i];
    return i;
}

int next_match(hits)
char *hits[];
{
    int i;

    if (ad_pu_s == NULL)
	return(-1);
    i = pattern_match(ad_pu_s,start_srch,end_srch,hits,0,&BR1);
    past_last = hits[i];
    return i;
}

int first_match_ns(start,len,hits,st)
char *start;
int len;
char *hits[];
struct state *st;
{
    int i;

    if (st->ad_pu_s == NULL)
	return(-1);
    
    start_srch = start; end_srch = start+(len-1);
    st->BR = NULL;
    i = pattern_match(st->ad_pu_s,start_srch,end_srch,hits,1, &(st->BR));
    st->past_last = hits[i];
    if (!i)
	free_state(st);
    return i;
}

int next_match_ns(hits,st)
char *hits[];
struct state *st;
{
    int i;

    if (st->ad_pu_s == NULL)
	return(-1);
    i = pattern_match(st->ad_pu_s,start_srch,end_srch,hits,0,&(st->BR));
    st->past_last = hits[i];
    if (!i)
	free_state(st);
    return i;
}

struct punit *next_punit(pu)
struct punit *pu;
{
  struct punit *pu1,*pu2;
  
  if (pu->nxt_punit) return pu->nxt_punit;
  for (pu1=pu, pu2=pu1->prev_punit; 
       (pu2 && ((pu2->nxt_punit == pu1) || (! pu2->nxt_punit)));
       pu1=pu2, pu2=pu1->prev_punit) 
    ;
  if (pu2 && (pu2->nxt_punit != pu1))
    return pu2->nxt_punit;
  
  return NULL;
}

/* ==================== TAKE THIS ====================================== */
#define SUCCESS { CR->mlen = SR - CR->hit; pu1=next_punit(CR); if (pu1) { CR = pu1; goto TRY;} else {goto LEAVE;}}

#define KnownChar(C)  (punit_sequence_type == PEPTIDE ? 1 : known_char[(C)])

#define Matches(C1,C2) (punit_sequence_type == PEPTIDE ? \
			((C1 == C2) || (C2 == 'X')): \
                        (KnownChar((C1) & 15) && ((((C1) & 15) & ((C2) & 15)) == (C1 & 15))))
#define MatchRuleSet(RuleSet,C1,C2) (*(rule_sets[RuleSet] + C2 + known_char_index[(C1 & 15)]))
/*
int MatchRuleSet(RuleSet,C1,C2)
int RuleSet;
char C1,C2;
{
   int x,y;

   y = known_char_index[C1 & 15];

   if (y == -1)  return 0;

   x = *(rule_sets[RuleSet]+C2+y);
   return x;
}
*/

#define ExMatches(RuleSet,C1,C2) ((RuleSet == -1) ? \
                                  (Matches(C1,C2)) : \
				  (MatchRuleSet(RuleSet,C1,C2)))

char *position_first_match(pu)
punit_t *pu;
{
    if (pu->type != OR_PUNIT)
    {
	return pu->hit;
    }
    else
    {
	if (pu->info.or.alt == 1)
	{
	    return position_first_match(pu->info.or.or1);
	}
	else
	{
	    return position_first_match(pu->info.or.or2);
	}
    }
}

int length_of_match(pu)
punit_t *pu;
{
    punit_t *pu1;

    if (pu->type == OR_PUNIT)
    {
	if (pu->info.or.alt == 1)
	{
            pu1=pu->info.or.or1;
	}
	else
	{
            pu1=pu->info.or.or2;
	}
	for (pu->mlen=0; pu1; pu1=pu1->nxt_punit)
	    pu->mlen += length_of_match(pu1);
    }
    return pu->mlen;
}

int pattern_match(pu,start,end,hits,first,BR1)
punit_t **BR1;
punit_t *pu;
char *start,*end;
char *hits[];
int first;
{
    punit_t *BR;   /* only static variable here (I think) */

    int i,j;
    char *try();
    char *revhits[MAX_PUNITS];
    punit_t *CR;
    char *SR,*ER;
    char *last;
    char *p1,*p2,*p3;
    int *pv1,*pv3;
    wv_t *pv2;
    int ln;
    int wval;
    struct punit *pu1;
    char scratch[4000];

    BR = *BR1;
    SR = start; ER = end;
    if (!first) goto BACKTRACK;
    CR = pu;

  TRY:
    switch (CR->type)
    {
      case MATCH_START:
	    if (SR == start)
	    {
		CR->hit = SR;
		SUCCESS;
	    }
	    goto BACKTRACK;
	break;
	    
      case MATCH_END:
	    if (SR == end+1)
	    {
		CR->hit = SR;
		SUCCESS;
	    }
	    goto BACKTRACK;
        break;
	    
      case ANY_PUNIT:
            last = ER;
            if ((last > SR) && CR->anchored)
                last = SR;
            while (SR <= last)
            {
		i = *SR;
		if (i >= 'A' && i <= 'Z' &&
		    ((1 << (i - 'A')) & CR->info.any.code_matrix))
                {
		    break;
                }
                SR++;
            }
            if (SR > last)
            {
                goto BACKTRACK;
            }
            else
            {
                if ((CR->hit = SR) < last)
                {
                    CR->BR = BR;
                    BR = CR;
                }
                SR++;
                SUCCESS;
            }
        break;

      case LLIM_PUNIT:
        for (ln=0, i=CR->info.llim.llim_vec[0]; i;)
        {
	  pu1 = names[CR->info.llim.llim_vec[i--]];
	  ln += pu1->mlen;
        }
        if (ln < CR->info.llim.bound)
	{
	  CR->hit = SR;
	  SUCCESS;
	}
	goto BACKTRACK;
	break;

      case RANGE_PUNIT:
        if ((SR + (i=CR->info.range.min-1)) <= ER)
        {
            CR->hit = SR;
            SR += i+1;
            if (((SR <= ER) && (CR->info.range.width)) ||
                (!CR->anchored && (SR <= ER)))
            {
                CR->BR = BR;
                BR=CR;
                CR->info.range.nxt = CR->info.range.min+1;
            }
            SUCCESS;
        }
        else
        {
            goto BACKTRACK;
        }
        break;

      case EXACT_PUNIT:
            last = ER+1 - CR->info.exact.len;
            if ((last > SR) && CR->anchored)
                last = SR;
            p1 = CR->info.exact.code;
            ln = CR->info.exact.len - 1;
            while (SR <= last)
            {
                if (Matches(*SR,*p1))
                {
                    p2 = SR+1; p3 = p1+1;
                    for (i=ln; i && Matches(*p2,*p3); i--,p3++,p2++)
                        ;
                    if (!i)
                        break;
                }
                SR++;
            }
            if (SR > last)
            {
                goto BACKTRACK;
            }
            else
            {
                if ((CR->hit = SR) < last)
                {
                    CR->BR = BR;
                    BR = CR;
                }
                SR += CR->info.exact.len;
                SUCCESS;
            }
        break;

      case COMPL_PUNIT:
        pu1 = names[CR->info.compl.of];
        p1 = pu1->hit;
        ln = pu1->mlen;
        if ((CR->info.compl.rule_set != -1) || 
            CR->info.compl.ins || CR->info.compl.del || CR->info.compl.mis)
        {
            if (i=loose_match((unsigned char *)p1,ln,
                              (unsigned char *)SR,ER+1-SR,
                              CR->info.compl.ins,
                              CR->info.compl.del,
                              CR->info.compl.mis,
                              CR->info.compl.rule_set,
                              &j,1))
            {
	        i--;
                CR->hit = SR;
                SR += i;
                SUCCESS;
            }
        }
        else if (ER-SR >= ln-1)
        {
            CR->hit = SR;
            p1 = pu1->hit + (ln-1);
            while (ln--)
            {
	       /* FIX: 4/25/92: complement cannot match ambiguous char */
               if (!KnownChar((*p1)&15) || 
                   (((*(p1--) >> 4) & 15) != (*(SR++) & 15)))
                   goto BACKTRACK;
            }
            SUCCESS;
        }
        goto BACKTRACK;
        break;

      case REPEAT_PUNIT:
        pu1 = names[CR->info.repeat.of];
        p1 = pu1->hit;

        if (!(ln = pu1->mlen))
	{
            CR->hit = SR;
            SUCCESS;
        }
	    
        if (i=loose_match((unsigned char *)p1,ln,
                          (unsigned char *)SR,ER+1-SR,
                          CR->info.repeat.ins,
                          CR->info.repeat.del,
                          CR->info.repeat.mis,
                          -1,&j,0))
        {
	    i--;
            CR->hit = SR;
            SR += i;
            SUCCESS;
        }
        goto BACKTRACK;
        break;

      case INV_REP_PUNIT:
        pu1 = names[CR->info.repeat.of];
        p1 = pu1->hit;

        if (!(ln = pu1->mlen))
	{
            CR->hit = SR;
            SUCCESS;
        }

	if (ln > 4000)
	{
	    p3 = (char *) malloc(ln);
	}
	else
	{
	    p3 = scratch;
	}
        for (i=0,j=ln-1; i < ln; i++,j--)
	{
	    *(p3+i) = *(p1+j);
	    
	}

        if (i=loose_match((unsigned char *)p3,ln,
                          (unsigned char *)SR,ER+1-SR,
                          CR->info.repeat.ins,
                          CR->info.repeat.del,
                          CR->info.repeat.mis,
                          -1,&j,0))
        {
	    i--;
            CR->hit = SR;
            SR += i;
            SUCCESS;
        }
        goto BACKTRACK;
        break;

      case SIM_PUNIT:
        last = ER+1 + CR->info.sim.ins - CR->info.sim.len;
        if ((last > SR) && CR->anchored)
            last = SR;
        p1 = CR->info.sim.code;
        ln = CR->info.sim.len - 1;
        while (SR <= last)
        {
            if (i=loose_match((unsigned char *)CR->info.sim.code,
                              CR->info.sim.len,
                              (unsigned char *)SR,ER+1-SR,
                              CR->info.sim.ins,
                              CR->info.sim.del,
                              CR->info.sim.mis,
                              -1,&j,0))
            {
	        i--;
                if ((CR->hit = SR) < last)
                {
                    CR->BR = BR;
                    BR = CR;
                }
                SR += i;
                SUCCESS;
            }
            SR++;
        }
        goto BACKTRACK;
        break;

      case WEIGHT_PUNIT:
            last = ER+1 - CR->info.wvec.len;
            if ((last > SR) && CR->anchored)
                last = SR;
            pv1 = CR->info.wvec.vec;
            ln = CR->info.wvec.len;
            while (SR <= last)
            {
                if (CR->info.wvec.tupsz == 4)
                {
                    for (pv2= (wv_t *) pv1, p1=SR, i=ln, wval=0;
                         i;
                         i--,pv2++)
                    {
                        switch (*(p1++) & 15)
                        {
                          case A_BIT: wval += pv2->aw; break;
                          case C_BIT: wval += pv2->cw; break;
                          case G_BIT: wval += pv2->gw; break;
                          case T_BIT: wval += pv2->tw; break;
                          case (A_BIT+C_BIT):
                              wval += (pv2->aw >> 1) + (pv2->cw >> 1);
                            break;
                          case (A_BIT+G_BIT):
                              wval += (pv2->aw >> 1) + (pv2->gw >> 1);
                            break;
                          case (A_BIT+T_BIT):
                              wval += (pv2->aw >> 1) + (pv2->tw >> 1);
                            break;
                          case (C_BIT+G_BIT):
                              wval += (pv2->cw >> 1) + (pv2->gw >> 1);
                            break;
                          case (C_BIT+T_BIT):
                              wval += (pv2->cw >> 1) + (pv2->tw >> 1);
                            break;
                          case (G_BIT+T_BIT):
                              wval += (pv2->gw >> 1) + (pv2->tw >> 1);
                            break;
                          case (C_BIT+G_BIT+T_BIT):
                              wval += (pv2->cw / 3) +
                                      (pv2->gw / 3) +
                                      (pv2->tw / 3);
                            break;
                          case (A_BIT+G_BIT+T_BIT):
                              wval += (pv2->aw / 3) +
                                      (pv2->gw / 3) +
                                      (pv2->tw / 3);
                            break;
                          case (A_BIT+C_BIT+T_BIT):
                              wval += (pv2->aw / 3) +
                                      (pv2->cw / 3) +
                                      (pv2->tw / 3);
                            break;
                          case (A_BIT+C_BIT+G_BIT):
                              wval += (pv2->aw / 3) +
                                      (pv2->cw / 3) +
                                      (pv2->gw / 3);
                            break;
                          case (A_BIT+C_BIT+G_BIT+T_BIT):
                              wval += (pv2->aw >> 2) +
                                      (pv2->cw >> 2) +
                                      (pv2->gw >> 2) +
                                      (pv2->tw >> 2);
                            break;
                        }
                    }
                }
                else   /* handling 20-tuples  (or 21-tuples) */
                {
                    for (pv3=pv1, p1=SR, i=ln, wval=0;
                         i;
                         i--,pv3 += CR->info.wvec.tupsz)
                    {
                        switch (*(p1++))
                        {
                          case 'A': wval += *(pv3+0); break;
                          case 'C': wval += *(pv3+1); break;
                          case 'D': wval += *(pv3+2); break;
                          case 'E': wval += *(pv3+3); break;
                          case 'F': wval += *(pv3+4); break;
                          case 'G': wval += *(pv3+5); break;
                          case 'H': wval += *(pv3+6); break;
                          case 'I': wval += *(pv3+7); break;
                          case 'K': wval += *(pv3+8); break;
                          case 'L': wval += *(pv3+9); break;
                          case 'M': wval += *(pv3+10); break;
                          case 'N': wval += *(pv3+11); break;
                          case 'P': wval += *(pv3+12); break;
                          case 'Q': wval += *(pv3+13); break;
                          case 'R': wval += *(pv3+14); break;
                          case 'S': wval += *(pv3+15); break;
                          case 'T': wval += *(pv3+16); break;
                          case 'V': wval += *(pv3+17); break;
                          case 'W': wval += *(pv3+18); break;
                          case 'Y': wval += *(pv3+19); break;
			  default: 
			      if (CR->info.wvec.tupsz > 20)
				  wval += *(pv3+20);
			      break;
                        }
                    }
                }
                if ((wval > CR->info.wvec.cutoff) && (wval < CR->info.wvec.maxwt))
                {
                    break;
                }
                SR++;
            }
            if (SR > last)
            {
                goto BACKTRACK;
            }
            else
            {
                if ((CR->hit = SR) < last)
                {
                    CR->BR = BR;
                    BR = CR;
                }
                SR += CR->info.wvec.len;
                SUCCESS;
            }
        break;

      case OR_PUNIT:
	CR->BR = BR; 
	BR=CR; 
	CR->info.or.SR = SR;
	CR->hit = SR;
	CR->info.or.alt = 1;
	CR = CR->info.or.or1;
	goto TRY;
        break;
    }

  BACKTRACK:
    if (!BR)
        return 0;
    else
    {
        CR = BR;
        BR = CR->BR;
        SR = CR->hit;
        switch (CR->type)
        {
          case RANGE_PUNIT:
            if ((CR->info.range.nxt <= CR->info.range.min + CR->info.range.width) &&
                (SR + CR->info.range.nxt-1 <= ER))
            {
                SR = SR + CR->info.range.nxt++;
                BR = CR;
                SUCCESS;
            }
            else if (((++CR->hit + CR->info.range.min-1) <= ER) && !CR->anchored)
            {
                CR->info.range.nxt = CR->info.range.min+1;
                SR = CR->hit + CR->info.range.min;
                BR = CR;
                SUCCESS;
            }
            else
                goto BACKTRACK;
            break;

          case LLIM_PUNIT:
          case ANY_PUNIT:
          case EXACT_PUNIT:
          case SIM_PUNIT:
          case WEIGHT_PUNIT:
            SR++; goto TRY;
            break;

          case OR_PUNIT:
            if (CR->info.or.alt == 1)
	    {
	      CR->info.or.alt = 2;
	      CR = CR->info.or.or2;
	      goto TRY;
	    }
	    else if ((! CR->anchored) && (CR->info.or.alt == 2))
	    {
		SR++; goto TRY;
	    }
            else
            {
                goto BACKTRACK;
            }
            break;
        }
    }

  LEAVE:
    i = collect_hits(CR,revhits);
    for (j=0; i--; j++)
        hits[j] = revhits[i];
    hits[j] = SR;
    *BR1 = BR;
    return j;
}

int collect_hits(pu,revhits)
struct punit *pu;
char **revhits;
{
    struct punit *last;
    char **p = revhits;

    last = NULL;
    while (pu)
    {
        if (pu->type != OR_PUNIT)
        {
            *(p++) = pu->hit;
	    last = pu;
	    pu = pu->prev_punit;
        }
        else
        {
            if (pu->nxt_punit == last)
            {
                if (pu->info.or.alt == 1)
                    pu = pu->info.or.or1;
                else
                    pu = pu->info.or.or2;
                while (pu->nxt_punit)
                    pu = pu->nxt_punit;
		last = NULL;
		
            }
	    else
	    {
		last = pu;
		pu = pu->prev_punit;
	    }
        }
    }
    return p-revhits;
}


int build_conversion_tables()
{
    int the_char;

    for (the_char=0; the_char < 256; the_char++) {
        switch(tolower(the_char)) {
          case 'a': punit_to_code[the_char] = A_BIT; break;
          case 'c': punit_to_code[the_char] = C_BIT; break;
          case 'g': punit_to_code[the_char] = G_BIT; break;
          case 't': punit_to_code[the_char] = T_BIT; break;
          case 'u': punit_to_code[the_char] = T_BIT; break;
          case 'm': punit_to_code[the_char] = (A_BIT | C_BIT); break;
          case 'r': punit_to_code[the_char] = (A_BIT | G_BIT); break;
          case 'w': punit_to_code[the_char] = (A_BIT | T_BIT); break;
          case 's': punit_to_code[the_char] = (C_BIT | G_BIT); break;
          case 'y': punit_to_code[the_char] = (C_BIT | T_BIT); break;
          case 'k': punit_to_code[the_char] = (G_BIT | T_BIT); break;
          case 'b': punit_to_code[the_char] = (C_BIT | G_BIT | T_BIT); break;
          case 'd': punit_to_code[the_char] = (A_BIT | G_BIT | T_BIT); break;
          case 'h': punit_to_code[the_char] = (A_BIT | C_BIT | T_BIT); break;
          case 'v': punit_to_code[the_char] = (A_BIT | C_BIT | G_BIT); break;
          case 'n': punit_to_code[the_char] = (A_BIT | C_BIT | G_BIT | T_BIT); break;
          default:
            punit_to_code[the_char] = 0;
            break;
        }
        if (punit_to_code[the_char] & A_BIT)
            punit_to_code[the_char] |= T_BIT << 4;
        if (punit_to_code[the_char] & C_BIT)
            punit_to_code[the_char] |= G_BIT << 4;
        if (punit_to_code[the_char] & G_BIT)
            punit_to_code[the_char] |= C_BIT << 4;
        if (punit_to_code[the_char] & T_BIT)
            punit_to_code[the_char] |= A_BIT << 4;
    }

    for (the_char=0; the_char < 256; the_char++)
    {
	switch (the_char & 15)
	{
	  case A_BIT:
	    code_to_punit[the_char] = 'A';
	    break;

	  case C_BIT:
	    code_to_punit[the_char] = 'C';
	    break;

	  case G_BIT:
	    code_to_punit[the_char] = 'G';
	    break;

	  case T_BIT:
	    code_to_punit[the_char] = 'T';
	    break;

	  case A_BIT + C_BIT:
	    code_to_punit[the_char] = 'M';
	    break;

	  case A_BIT + G_BIT:
	    code_to_punit[the_char] = 'R';
	    break;

	  case A_BIT + T_BIT:
	    code_to_punit[the_char] = 'W';
	    break;

	  case C_BIT + G_BIT:
	    code_to_punit[the_char] = 'S';
	    break;

	  case C_BIT + T_BIT:
	    code_to_punit[the_char] = 'Y';
	    break;

	  case G_BIT + T_BIT:
	    code_to_punit[the_char] = 'K';
	    break;

	  case C_BIT + G_BIT + T_BIT:
	    code_to_punit[the_char] = 'B';
	    break;

	  case A_BIT + G_BIT + T_BIT:
	    code_to_punit[the_char] = 'D';
	    break;

	  case A_BIT + C_BIT +T_BIT:
	    code_to_punit[the_char] = 'H';
	    break;

	  case A_BIT + C_BIT + G_BIT:
	    code_to_punit[the_char] = 'V';
	    break;

	  case A_BIT + C_BIT + G_BIT +T_BIT:
	    code_to_punit[the_char] = 'N';
	    break;
	}
    }
    initialized=1;
    return(0);
}

int comp_data(in,out)
char *in,*out;
{
    while (*in)
        *(out++) = punit_to_code[*(in++)];
    *out = '\0';
    return(0);
}


/*------------------------*/
/* code for loose_match() */

#define MIN(A,B) ((A) < (B) ? (A) : (B))



/* returns a pointer to the reverse complement of data. */
/*  When reverse complements with specified rule set, the character
    is set to the index into the rule set for the previously matched char
*/
void rev_compl_data(rule_set,data, len,result)
int rule_set;
unsigned char *data,*result;
int len;
{
    unsigned int i;

    while (len > 0) {
        i = *(data++);
	if (rule_set == -1)
	    result[--len] = (unsigned char)( (i >> 4) & 15 );
	else
	    result[--len] = (unsigned char) known_char_index[( i & 15)] << 2;
    }
}


/*
 *  one is a small code string of length one_len+1, to be matched against
 * the region pointed at by two (which has two_len+1 codes).  We want to
 * know how much of two can be matched by one, allowing max_ins insertions,
 * max_del deletions, and max_mis mismatches.
 *  The number returned will be the least amount of two that can be matched;
 * match_range will be set to how much more can be matched.
 */
#define Stack(N) {stack[nxtent].p1=one_data; stack[nxtent].p2=two_data; \
                  stack[nxtent].n1=one_len; stack[nxtent].n2=two_len;   \
                  stack[nxtent].mis=max_mis; stack[nxtent].ins=max_ins; \
                  stack[nxtent].del=max_del; stack[nxtent++].next_choice=N;}

#define Pop                                                 \
            one_data = stack[nxtent].p1;                    \
            two_data = stack[nxtent].p2;                    \
            one_len = stack[nxtent].n1;                     \
            two_len = stack[nxtent].n2;                     \
            max_mis = stack[nxtent].mis;                    \
            max_ins = stack[nxtent].ins;                    \
            max_del = stack[nxtent].del;                    \
            if (stack[nxtent].next_choice == 1)             \
            {                                               \
                if (max_del)                                \
                    stack[nxtent].next_choice = 2;          \
                goto ins_char;                              \
            }                                               \
            else                                            \
                goto del_char;


int loose_match(one_data, one_len, two_data, two_len,
                max_ins, max_del, max_mis, rule_set,
                match_range, compl_flag)
unsigned char *one_data, *two_data;
int one_len, two_len;
int max_ins, max_del, max_mis;
int *match_range;
int compl_flag,rule_set;
{
    unsigned char result[MAX_CODES];
    unsigned char *start_two_data = two_data;
    int i, nxtent;
    struct stackent {
        unsigned char *p1,*p2;
        int n1,n2;
        int mis,ins,del;
        int next_choice;
    } stack[100];


    if (compl_flag) {
	/* FIX: 4/25/92 RAO; complements of ambiguous regions fail */
	for (i=0; i < one_len; i++)
 	    if (!KnownChar(*(one_data+i) & 15))
		return 0;
        rev_compl_data(rule_set,one_data, one_len,result);
        one_data = result;
    }

    /* special-case for ins=del=0 */
    if ((max_ins == 0) && (max_del == 0))
    {
        if (one_len > two_len)
        {
            return 0;
        }
        for (i=one_len; i >= 1; i--)
        {
            if (!KnownChar((*two_data)&15) ||
		(!ExMatches(rule_set,*two_data,*one_data) && (--max_mis < 0)))
                return 0;
            else
            {
                two_data++; one_data++;
            }
        }
	return (two_data-start_two_data)+1;
    }

    nxtent=0;
    while (two_len || nxtent)
    {
        if (two_len && one_len && KnownChar((*two_data)&15) &&
	    ExMatches(rule_set,*two_data,*one_data))
        {
            two_data++; one_data++; two_len--;
            if (!(--one_len))
                return (two_data - start_two_data)+1;
        }
	else if (max_mis && (one_len >= 1) && (two_len >= 1))
	{
	    if (max_ins)
	    {
	      Stack(1);
	    }
	    else if (max_del)
	    {
	      Stack(2);
	    }
	    max_mis--; one_data++; two_data++; one_len--; two_len--;
	    if (! one_len)
	    {
	      return (two_data - start_two_data)+1;
	    }
	}
	else if ((max_ins) && (one_len >= 1))
	{
	    if ((max_del) && (two_len >= 1))
	    {
	      Stack(2);
	    }
	  ins_char:
	    max_ins--; one_data++; one_len--;
	    if (! one_len)
	        return (two_data - start_two_data)+1;
	}
	else if ((max_del) && (two_len >= 1))
	{
	  del_char:
	    max_del--; two_data++; two_len--;
	    if (! one_len)
	      return (two_data - start_two_data)+1;
	}
	else if (nxtent--)
	{
	  Pop;
	}
	else
	  return 0;
    }
    return 0;
}

int cont_match(hits)
char *hits[];
{
   int i;
   char *past_last1;
   past_last1 = past_last;
   while(((i=next_match(hits)) > 0 )&&(hits[0] < past_last1))
      ;
   past_last = hits[i];
   return(i);
}

int cont_match_ns(hits,st)
char *hits[];
struct state *st;
{
   int i;
   char *past_last1;
   past_last1 = st->past_last;
   while(((i=next_match_ns(hits,st)) > 0 )&&(hits[0] < past_last1))
      ;
   st->past_last = hits[i];
   return(i);
}

#ifdef REVERSE_PUNIT

#define FIXBUFF *buff += strlen(*buff)

/* WARNING: VERY UNRELIABLE */
void reverse_punit(buff)
char *buff;
{
   punit_t *pu;

   strcpy(buff,"Reversed: ");
   buff += strlen(buff);
   for(pu = ad_pu_s; pu; pu=pu->nxt_punit)
      reverse_one_punit(pu,&buff);
}

#define NEXTBUFF(X) **buff=((X)), *buff += 1, **buff='\0'

void reverse_one_punit(pu,buff)
punit_t *pu;
char **buff;
{
   punit_t *pu1;
   int i;

   for(i=0;i<MAX_NAMES;i++)
      if(pu==names[i])
      {
         sprintf(*buff," p%d=",i);
         FIXBUFF;
         break;
      }
   if(i==MAX_NAMES)
      NEXTBUFF(' ');
   switch(pu->type)
   {
      case RANGE_PUNIT:
      (void) sprintf(*buff,"%d...%d",
             pu->info.range.min,
             pu->info.range.min+pu->info.range.width);
      break;

      case EXACT_PUNIT:
      for(i=0;i<pu->info.exact.len;i++)
        NEXTBUFF(display_seq_char[0xF & pu->info.exact.code[i]]);
      break;

      case LLIM_PUNIT:
      exit(2);
      break;

      case COMPL_PUNIT:
      (void) sprintf(*buff,"~p%d[%d,%d,%d]",
             pu->info.compl.of,
             pu->info.compl.mis,
             pu->info.compl.ins,
             pu->info.compl.del);
      break;

      case REPEAT_PUNIT:
      (void) sprintf(*buff,"p%d[%d,%d,%d]",
             pu->info.repeat.of,
             pu->info.repeat.mis,
             pu->info.repeat.ins,
             pu->info.repeat.del);
      break;

      case SIM_PUNIT:
      for(i=0;i<pu->info.sim.len;i++)
        NEXTBUFF(display_seq_char[0xF & pu->info.sim.code[i]]);

      (void) sprintf(*buff,"[%d,%d,%d]",
             pu->info.sim.mis,
             pu->info.sim.ins,
             pu->info.sim.del);
      break;

      case WEIGHT_PUNIT:
      if(pu->info.wvec.tupsz != 4) exit(3);
      NEXTBUFF('{');

      for(i=0;i<pu->info.wvec.len;i++)
      {
         wv_t *wts = (wv_t *) pu->info.wvec.vec+i;
         if(i) NEXTBUFF(',');
         sprintf(*buff,"(%d,%d,%d,%d)",wts->aw,wts->cw,wts->gw,wts->tw);
         FIXBUFF;
      }

      NEXTBUFF('}');
      (void) sprintf(*buff,">%d",pu->info.wvec.cutoff);
      break;


      case OR_PUNIT:
      NEXTBUFF('(');
      for(pu1=pu->info.or.or1; pu1; pu1 = pu1->nxt_punit)
        reverse_one_punit(pu1,buff);
      NEXTBUFF('|');
      for(pu1=pu->info.or.or2; pu1; pu1 = pu1->nxt_punit)
        reverse_one_punit(pu1,buff);
      NEXTBUFF(')');
      break;
   }
   FIXBUFF;
}
#endif
/*THIS IS THE END*/
