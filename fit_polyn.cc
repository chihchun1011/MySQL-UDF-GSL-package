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
    bool fit_linear_init(UDF_INIT *initid,UDF_ARGS *args, char *message);
    void fit_linear_deinit(UDF_INIT *initid);
    void fit_linear_clear(UDF_INIT *initid, char *is_null, char *error);
    void fit_linear_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);
    char *fit_linear(UDF_INIT *initid, UDF_ARGS*args, char *result, unsigned long *length, char *is_null, char *error);
}

#define DECIMALS 6

struct fit_prepare {
    size_t N_row;
    size_t degree;

    vector<double> data_x;
    vector<double> data_y;

    gsl_multifit_linear_workspace *ws;
    gsl_matrix *cov, *X;
    gsl_vector *y, *c;

    X = gsl_matrix_alloc(N_row, degree);
    y = gsl_vector_alloc(N_row);
    c = gsl_vector_alloc(degree);
    cov = gsl_matrix_alloc(degree, degree);

    double coeff[degree];
};


bool fit_linear_init(UDF_INIT *initid,UDF_ARGS *args, char *message){
    if(args->arg_count != 4){
        strcpy(message, "wrong number of arguments: FIT_LINEAR() requires two arguments");
        return 1;
    }

    args->arg_type[0] = INT_RESULT;
    args->arg_type[1] = INT_RESULT;
    args->arg_type[2] = REAL_RESULT;
    args->arg_type[3] = REAL_RESULT;

    fit_prepare *prepare = new fit_prepare;
    prepare->N_row = *((int *)arg->arg[0]);
    prepare->degree = *((int *)arg->arg[1]);

    initid->decimals = DECIMALS;
    initid->ptr = (char *)prepare;

    return 0;
}

void fit_linear_deinit(UDF_INIT *initid){
    fit_prepare *prepare = (fit_prepare *)initid->ptr;
    delete prepare;
}

void fit_linear_clear(UDF_INIT *initid, char *is_null, char *error){
    fit_prepare *prepare = (fit_prepare *)initid->ptr;
    prepare->N_row = 0;
    prepare->degree = 0;
    prepare->data_x.clear();
    prepare->data_y.clear();

    gsl_multifit_linear_free(prepare->ws);
    gsl_matrix_free(prepare->X);
    gsl_matrix_free(prepare->cov);
    gsl_vector_free(prepare->y);
    gsl_vector_free(prepare->c);
}

void fit_linear_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error){
    fit_prepare *prepare = (fit_prepare *)initid->ptr;
    prepare->data_x.push_back(*(double *)args->args[2]);
    prepare->data_y.push_back(*(double *)args->args[3]); 
}

char *fit_linear(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error){
    fit_prepare *prepare = (fit_prepare *)initid->ptr;

    double *data_x = prepare->data_x.data();  // convert vector to array
    double *data_y = prepare->data_y.data();

    double chisq;

    int i, j;
    for(i=0; i < prepare->N_row; i++){
        
        for(j=0; j < prepare->degree; j++){
            gsl_matrix_set(X, i, j, pow(data_x[i], j));
        }
        
        gsl_vector_set(y, i, data_y[i]);
    }

    prepare->ws = gsl_multifit_linear_alloc(prepare->N_row, prepare->degree);
    gsl_multifit_linear(prepare->X, prepare->y, prepare->c, prepare->cov, &chisq, prepare->ws);

    /* store result ... */
    for(i=0; i < prepare->degree; i++)
    {
        prepare->coeff[i] = gsl_vector_get(prepare->c, i);
    }

    // return string should less than 255 bytes
    string ret_str;

    for (i=0; i < DEGREE; i++){
        ret_str+=to_string(coeff[i])
        ret_str+=" "
    }
    strcpy(result, ret_str.c_str());
    *length = ret_str.size();
    
    return result;
}