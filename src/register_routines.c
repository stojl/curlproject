#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>


extern SEXP C_webservice(SEXP df);

static const R_CallMethodDef R_CallDef[] = {
  {"C_webservice", (DL_FUNC) &C_webservice, 1},
  {NULL, NULL, 0}
};

void R_init_curlproject(DllInfo *dll) {
  R_registerRoutines(dll, NULL, R_CallDef, NULL, NULL);
}
