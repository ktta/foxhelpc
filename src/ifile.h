
#ifndef ifile_h_included
#define ifile_h_included

typedef struct ifile ifile_t;


int ifile_open(ifile_t **Rf,char *name);
int ifile_get_line(ifile_t *f,uint8_t **buffer, size_t *len,int nullterm);
void ifile_close(ifile_t *f);
int ifile_line_number(ifile_t *f);
char *ifile_error(ifile_t* f);
char *ifile_file_name(ifile_t* f);

#endif
