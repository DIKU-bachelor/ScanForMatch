extern int parse_dna_cmd(char *line);
extern int parse_peptide_cmd(char *line);
extern int comp_data(char *in, char *out);
extern int first_match(char *start, int len, char *hits[]);
extern int cont_match(char *hits[]);
