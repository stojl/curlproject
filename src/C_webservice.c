#include <R.h>
#include <Rinternals.h>

/* Hvis du skal sende mere end en tweet, så skal du gentænke dit eget potentiale */
#define STRING_BUFF_SIZE 140 + 2 + 2
#define DOUBLE_BUFF_SIZE 24 + 2
#define INTEGER_BUFF_SIZE 16 + 2
#define DEFAULT_BUFF_SIZE 20 + 2 + 2

int df_buffer_size(SEXP df) {
  int N = length(df);

  SEXP names = getAttrib(df, R_NamesSymbol);

  int buffsize = 2 + length(names) * STRING_BUFF_SIZE; /* Buffer has size 2 to account for \0 and \n at the end of each row. */

  for(int i = 0; i < N; ++i) {
    switch(TYPEOF(VECTOR_ELT(df, i))) {
    case REALSXP:
      buffsize += DOUBLE_BUFF_SIZE;
      break;
    case INTSXP:
      buffsize += INTEGER_BUFF_SIZE;
      break;
    case STRSXP:
      buffsize += STRING_BUFF_SIZE;
      break;
    default:
      buffsize += DEFAULT_BUFF_SIZE;
    }
  }

  return buffsize * length(VECTOR_ELT(df, 0));
}

char* concat_str_double(char *buff, const double x) {
  snprintf(buff, DOUBLE_BUFF_SIZE, "%e", x);

  while(*buff != '\0') {
    buff += 1;
  }

  return buff;
}

char* concat_str_int(char *buff, const int x) {
  snprintf(buff, INTEGER_BUFF_SIZE, "%d", x);

  while(*buff != '\0') {
    buff += 1;
  }

  return buff;
}

char* concat_str_char(char *buff, const char *x) {
  snprintf(buff, STRING_BUFF_SIZE, "\"%s\"", x);

  while(*buff != '\0') {
    buff += 1;
  }

  return buff;
}

char* concat_str_numeric_name(char *buff, const char *x) {
  snprintf(buff, STRING_BUFF_SIZE, "%s", x);

  while(*buff != '\0') {
    buff += 1;
  }

  return buff;
}

char* concat_str_na(char *buff) {
  snprintf(buff, 4, "%s", "NA");

  while(*buff != '\0') {
    buff += 1;
  }

  return buff;
}
/*
char* concatenate_string(char *buff, const void *obj, int type) {

  switch(type) {
  case REALSXP:
    snprintf(buff, DOUBLE_BUFF_SIZE, "%e", *(double*)obj);
    break;
  case INTSXP:
    snprintf(buff, INTEGER_BUFF_SIZE, "%d", *(int*)obj);
    break;
  case CHARSXP:
    snprintf(buff, STRING_BUFF_SIZE, "%s", (char*)obj);
    break;
  default:
    snprintf(buff, DEFAULT_BUFF_SIZE, "%s", "NA");
    break;
  }

  while(*buff != '\0') {
    buff += 1;
  }

  return buff;
}*/

char* dataframe_to_string(SEXP df) {

  int buffer_size = df_buffer_size(df);


  char *buffer = (char*)malloc(buffer_size * sizeof(char));
  buffer[0] = '\0';

  char *buff_ptr = buffer;

  int n_cols = length(df);
  int n_rows = length(VECTOR_ELT(df, 0));

  /* Write column names to string */
  SEXP names = getAttrib(df, R_NamesSymbol);
  for(int i = 0; i < n_cols; ++i) {
    if(TYPEOF(VECTOR_ELT(df, i)) == REALSXP || TYPEOF(VECTOR_ELT(df, i)) == INTSXP) {
      buff_ptr = concat_str_numeric_name(buff_ptr, CHAR(STRING_ELT(names, i)));
    } else {
      buff_ptr = concat_str_char(buff_ptr, CHAR(STRING_ELT(names, i)));
    }

    if(i < n_cols - 1) {
      buff_ptr[0] = '\t';
      ++buff_ptr;
    }
  }
  buff_ptr[0] = '\n';
  ++buff_ptr;

  double **double_columns = (double**)malloc(sizeof(double*) * n_cols);
  int **int_columns = (int**)malloc(sizeof(int*) * n_cols);
  int *types = (int*)malloc(sizeof(int) * n_cols);

  for(int i = 0, d_idx = 0, i_idx = 0; i < n_cols; ++i) {
    SEXP col = VECTOR_ELT(df, i);
    switch(TYPEOF(col)) {
    case REALSXP:
      types[i] = REALSXP;
      double_columns[d_idx] = REAL(col);
      ++d_idx;
      break;
    case INTSXP:
      types[i] = INTSXP;
      int_columns[i_idx] = INTEGER(col);
      ++i_idx;
      break;
    case STRSXP:
      types[i] = STRSXP;
      break;
    default:
      types[i] = -999;
      break;
    }
  }

  for(int j = 0; j < n_rows; ++j) {
    int double_idx = 0;
    int int_idx = 0;
    for(int i = 0; i < n_cols; ++i) {
      switch(types[i]) {
      case REALSXP:
        buff_ptr = concat_str_double(buff_ptr, double_columns[double_idx][j]);
        ++double_idx;
        break;
      case INTSXP:
        buff_ptr = concat_str_int(buff_ptr, int_columns[int_idx][j]);
        ++int_idx;
        break;
      case STRSXP:
        buff_ptr = concat_str_char(buff_ptr, CHAR(STRING_ELT(VECTOR_ELT(df, i), j)));
        break;
      default:
        buff_ptr = concat_str_na(buff_ptr);
        break;
      }
      if(i < n_cols - 1) {
        buff_ptr[0] = '\t';
        buff_ptr[1] = '\0';
        ++buff_ptr;
      }
    }
    if(j < n_rows - 1) {
      buff_ptr[0] = '\n';
      ++buff_ptr;
    }
  }
  free(double_columns);
  free(int_columns);
  free(types);
  return buffer;

}

SEXP C_webservice(SEXP df) {
  SEXP out = PROTECT(allocVector(STRSXP, 1));

  char *buff = dataframe_to_string(df);
  SET_STRING_ELT(out, 0, mkChar(buff));
  free(buff);
  UNPROTECT(1);
  return out;
}
