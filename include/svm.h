#ifndef SVM_H
#define SVM_H

#include <vector>

// the implementation is basically a simplification of the already simple libsvm
// found in https://github.com/cjlin1/libsvm

typedef struct {
    int index; // -1 = end of a vector
    float value;
} svm_node_t;

typedef struct {
	int l; // number of training data
    std::vector<float> y; // target values
    std::vector<std::vector<svm_node_t> > x; // training vecs
} svm_problem_t;

typedef enum { 
    C_SVC, 
    NU_SVC, 
    ONE_CLASS, 
    EPSILON_SVR, 
    NU_SVR 
} svm_type_t;

typedef enum { 
    LINEAR, 
    POLY, 
    RBF, 
    SIGMOID, 
    RBFWEIGH, 
    RBFWMATRIX, 
    PRECOMPUTED 
} kernel_type_t; 

typedef struct {
	svm_type_t svm_type;
    kernel_type_t kernel_type;
	int degree;				/* for poly */
	float gamma;			/* for poly/rbf/sigmoid */
	float coef0;			/* for poly/sigmoid */
    std::vector<float> kernel_weight;	/* for rbfweight/rbfwmatrix */
	int kernel_dim;			/* for rbfweight/rbfwmatrix */
	bool normalizeKernel;
	float kernel_norm;

	/* these are for training only */
	float cache_size;		/* in MB */
	float eps;				/* stopping criteria */
	float C;				/* for C_SVC, EPSILON_SVR and NU_SVR */
	int nr_weight;			/* for C_SVC */
    std::vector<float> weight_label;		/* for C_SVC */
    std::vector<float> weight;			/* for C_SVC */
	float nu;				/* for NU_SVC, ONE_CLASS, and NU_SVR */
	float p;				/* for EPSILON_SVR */
	int shrinking;			/* use the shrinking heuristics */
	int probability;		/* do probability estimates */

    svm_parameter_t() : kernel_weight(0), weight_label(0), weight(0), kernel_dim(0), nr_weight(0) { }
    svm_parameter_t& operator= (const svm_parameter_t &param);
} svm_parameter_t;

typedef struct {
	svm_parameter_t param;	// parameter
	int nr_class;			// number of classes, = 2 in regression/one class svm
	int l;					// total #SV
    std::vector<std::vector<svm_node_t> > SV;			// SVs (SV[l])
    std::vector<float> sv_coef;		// coefficients for SVs in decision functions (sv_coef[k-1][l])
    std::vector<float> rho;			// constants in decision functions (rho[k*(k-1)/2])
    std::vector<float> probA;		// pairwise probability information
    std::vector<float> probB;
    std::vector<float> eps;

	// for classification only
    std::vector<int> label;				// label of each class (label[k])
    std::vector<int> nSV;				// number of SVs for each class (nSV[k])
							// nSV[0] + nSV[1] + ... + nSV[k-1] = l
	int free_sv;			// 1 if svm_model is created by svm_load_model
							// 0 if svm_model is created by svm_train
} svm_model_t;

#endif
