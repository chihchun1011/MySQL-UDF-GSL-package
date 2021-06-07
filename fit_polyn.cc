#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <string>

#include <mysql.h>

#include <gsl/gsl_fit.h>
#include <gsl/gsl_multifit.h>
#include <gsl/gsl_matrix.h>

using namespace std;

extern "C" {
    bool fit_polyn_init(UDF_INIT *initid,UDF_ARGS *args, char *message);
    void fit_polyn_deinit(UDF_INIT *initid);
    void fit_polyn_clear(UDF_INIT *initid, char *is_null, char *error);
    void fit_polyn_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);
    char *fit_polyn(UDF_INIT *initid, UDF_ARGS*args, char *result, unsigned long *length, char *is_null, char *error);
}

#define DECIMALS 3

struct fit_prepare {
    size_t N_row;
    size_t degree;
    size_t y0;
    size_t X0;
    size_t X1;

    gsl_multifit_linear_workspace *ws;
    gsl_matrix *cov, *X;
    gsl_vector *y, *c;

};


bool fit_polyn_init(UDF_INIT *initid,UDF_ARGS *args, char *message){
    if(args->arg_count != 4){
        strcpy(message, "wrong number of arguments: FIT_LINEAR() requires two arguments");
        return 1;
    }

    if (args->arg_type[0] != INT_RESULT || args->arg_type[1] != INT_RESULT){       
        strcpy(message,"XXX() requires a string and an integer");
        return 1;
    }

    args->arg_type[2] = REAL_RESULT;
    args->arg_type[3] = REAL_RESULT;

    fit_prepare *prepare = new fit_prepare;
    prepare->N_row = *((int *)args->args[0]);
    prepare->degree = *((int *)args->args[1]);
    prepare->y0 = 0;
    prepare->X0 = 0;
    prepare->X1 = 0;

    prepare->X = gsl_matrix_alloc(prepare->N_row, prepare->degree);
    prepare->y = gsl_vector_alloc(prepare->N_row);
    prepare->c = gsl_vector_alloc(prepare->degree);
    prepare->cov = gsl_matrix_alloc(prepare->degree, prepare->degree);
    prepare->ws = gsl_multifit_linear_alloc(prepare->N_row, prepare->degree);

    initid->decimals = DECIMALS;
    initid->ptr = (char *)prepare;

    return 0;
}

void fit_polyn_deinit(UDF_INIT *initid){
    fit_prepare *prepare = (fit_prepare *)initid->ptr;
    delete prepare;
}

void fit_polyn_clear(UDF_INIT *initid, char *is_null, char *error){
    fit_prepare *prepare = (fit_prepare *)initid->ptr;
    prepare->N_row = 0;
    prepare->degree = 0;
    prepare->y0 = 0;
    prepare->X0 = 0;
    prepare->X1 = 0;

    gsl_multifit_linear_free(prepare->ws);
    gsl_matrix_free(prepare->X);
    gsl_matrix_free(prepare->cov);
    gsl_vector_free(prepare->y);
    gsl_vector_free(prepare->c);
}

void fit_polyn_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error){
    fit_prepare *prepare = (fit_prepare *)initid->ptr;

    gsl_vector_set(prepare->y,prepare->y0 , *(double *)args->args[3]);
    prepare->y0++;

    if(prepare->X1==prepare->degree){
        prepare->X0+=1;
        prepare->X1=0;
    }
    gsl_matrix_set(prepare->X,prepare->X0,prepare->X1,pow(*(double *)args->args[2], prepare->X1));
    prepare->X1++;

}

char *fit_polyn(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error){
    fit_prepare *prepare = (fit_prepare *)initid->ptr;

    double chisq;

    gsl_multifit_linear(prepare->X, prepare->y, prepare->c, prepare->cov, &chisq, prepare->ws);

    // // return string should less than 255 bytes
    string ret_str = "sdasd";

    // for(int i=0; i < prepare->degree; i++)
    // {
    //     ret_str += tostring(gsl_vector_get(prepare->c, i));
    //     ret_str+=" ";
    // }

    strcpy(result, ret_str.c_str());
    *length = ret_str.size();

    return result;
}