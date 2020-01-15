#include "dplyr.h"

namespace dplyr {
void stop_mutate_mixed_NULL() {
  SEXP sym_stop_mutate_mixed_NULL = Rf_install("stop_mutate_mixed_NULL");
  SEXP call = Rf_lang1(sym_stop_mutate_mixed_NULL);
  Rf_eval(call, dplyr::envs::ns_dplyr);
}

void stop_mutate_not_vector(SEXP result) {
  SEXP sym_stop_mutate_not_vector = Rf_install("stop_mutate_not_vector");
  SEXP call = Rf_lang2(sym_stop_mutate_not_vector, result);
  Rf_eval(call, dplyr::envs::ns_dplyr);
}
}

SEXP dplyr_mask_eval_all_mutate(SEXP quo, SEXP env_private, SEXP env_context, SEXP dots_names, SEXP sexp_i) {
  SEXP rows = PROTECT(Rf_findVarInFrame(env_private, dplyr::symbols::rows));
  R_xlen_t ngroups = XLENGTH(rows);

  SEXP mask = PROTECT(Rf_findVarInFrame(env_private, dplyr::symbols::mask));
  SEXP caller = PROTECT(Rf_findVarInFrame(env_private, dplyr::symbols::caller));

  SEXP res = PROTECT(Rf_allocVector(VECSXP, 2));
  SEXP chunks = PROTECT(Rf_allocVector(VECSXP, ngroups));
  bool seen_vec = false;
  bool needs_recycle = false;
  bool seen_null = false;

  for (R_xlen_t i = 0; i < ngroups; i++) {
    SEXP rows_i = VECTOR_ELT(rows, i);
    R_xlen_t n_i = XLENGTH(rows_i);
    SEXP current_group = PROTECT(Rf_ScalarInteger(i + 1));
    Rf_defineVar(dplyr::symbols::current_group, current_group, env_private);
    Rf_defineVar(dplyr::symbols::dot_dot_group_size, Rf_ScalarInteger(n_i), env_context);
    Rf_defineVar(dplyr::symbols::dot_dot_group_number, current_group, env_context);

    SEXP result_i = PROTECT(rlang::eval_tidy(quo, mask, caller));
    SET_VECTOR_ELT(chunks, i, result_i);

    if (Rf_isNull(result_i)) {
      seen_null = true;

      if (seen_vec) {
        dplyr::stop_mutate_mixed_NULL();
      }

    } else if (vctrs::vec_is_vector(result_i)) {
      seen_vec = true;

      if (vctrs::short_vec_size(result_i) != n_i) {
        needs_recycle = true;
      }

    } else {
      dplyr::stop_mutate_not_vector(result_i);
    }

    UNPROTECT(2);
  }

  if (seen_null && seen_vec) {
    // find out the first time the group was NULL so that the error will
    // be associated with this group
    int i = 0;
    for (int i = 0; i < ngroups; i++) {
      if (Rf_isNull(VECTOR_ELT(chunks, i))) {
        SEXP rows_i = VECTOR_ELT(rows, i);
        R_xlen_t n_i = XLENGTH(rows_i);
        SEXP current_group = PROTECT(Rf_ScalarInteger(i + 1));
        Rf_defineVar(dplyr::symbols::current_group, current_group, env_private);
        Rf_defineVar(dplyr::symbols::dot_dot_group_size, Rf_ScalarInteger(n_i), env_context);
        Rf_defineVar(dplyr::symbols::dot_dot_group_number, current_group, env_context);

        dplyr::stop_mutate_mixed_NULL();
      }
    }
  }

  // there was only NULL results
  if (ngroups > 0 && !seen_vec) {
    chunks = R_NilValue;
  }
  SET_VECTOR_ELT(res, 0, chunks);
  SET_VECTOR_ELT(res, 1, Rf_ScalarLogical(needs_recycle));

  UNPROTECT(5);

  return res;
}
