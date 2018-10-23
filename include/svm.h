/*
    Binary SVM trained using the SMO algorithm.
    Reference: "The Simplified SMO Algorithm" (http://math.unt.edu/~hsp0009/smo.pdf)
*/
#ifndef SVM_H
#define SVM_H
#include <vector>

typedef enum {
    RBF,
    LINEAR
} svm_kernel_type_t;

struct kernel_t;
typedef struct kernel_t kernel_t;

struct kernel_t {
    double (*kernel_function)(const kernel_t& kernel, const std::vector<double>& v1, const std::vector<double>& v2);
    std::vector<std::vector<double> > kernel_cache; // cache kernel computations to avoid expensive recomputations. Could use too much memory if N is too large.
    std::vector<std::vector<double> >* x; // weights
    double gamma; // for RBF computation
};

svm_kernel_type_t get_kernel_type(const char* s);
double (*get_kernel_function(svm_kernel_type_t type))(const kernel_t&, const std::vector<double>&, const std::vector<double>&);

kernel_t make_kernel(svm_kernel_type_t type, std::vector<std::vector<double> >* x, bool use_cache, double gamma = 0.f);
double kernel_compute(const kernel_t& kernel, int i, int j);
double kernel_linear(const kernel_t& kernel, const std::vector<double>& v1, const std::vector<double>& v2);
double kernel_rbf(const kernel_t& kernel, const std::vector<double>& v1, const std::vector<double>& v2);

typedef struct {
    std::vector<std::vector<double> > datum;
    std::vector<int> labels;
} svm_problem_t;

svm_problem_t make_svm_problem(const std::vector<std::vector<double> >& datum, const std::vector<int>& labels);

typedef struct {
    double C = 1.0f; // regularization parameter
    double tol = 1e-4; // numerical tolerance. Don't touch unless you're pro. Other usual values are between 1e-2 and 1e-3 
    double alpha_tol = 1e-7; // non-support vectors for space and time efficiency are truncated. To guarantee correct result set this to 0 to do no truncating. If you want to increase efficiency, experiment with setting this little higher, up to maybe 1e-4 or so.

    long long max_iter = 10000; // max number of iterations
    int num_passes = 10; // how many passes over alphas with no change before we halt? Increase for more precision.

    bool do_cache = true; // set to true to cache expensive kernel computations

    double gamma = 2.f; // for rbf, equivalent to 1/2*sigma^2

    svm_kernel_type_t kernel_type = RBF; // Currently between RBF and LINEAR kernel

} svm_parameter_t;

typedef struct {
    svm_parameter_t param;
    int N, D; // N is number of data points, D is the dimension of the data
    std::vector<double> alpha;
    double b = 0.f; // bias

    bool use_w = false;  // internal efficiency flag
    std::vector<double> w; // cache weights for linear kernels to speed up evaluation during test time.

    kernel_t kernel;

    svm_problem_t problem; // problem the model was based on
    int num_iter; // number of iterations used to train the model
} svm_model_t;

// data is NxD array of doubles. labels are 1 or -1.
svm_model_t svm_train(svm_problem_t problem, const svm_parameter_t& param);
svm_model_t svm_train(const std::vector<std::vector<double> >& datum, const std::vector<int>& labels);

// example is an array of length D. Returns margin of given example
// this is the core prediction function. All others are for convenience mostly and end up calling this one somehow.
double svm_margin(const svm_model_t& model, const std::vector<double>& example);
// data is an NxD array. Returns vector of margins.
std::vector<double> svm_margins(const svm_model_t& model, const std::vector<std::vector<double> >& data);

// Return predictions for given example
int svm_predict(const svm_model_t& model, const std::vector<double>& example);
// data is an NxD matrix. Returns vector of 1 or -1, predictions
std::vector<int> svm_predict(const svm_model_t& model, const std::vector<std::vector<double> >& data);

// if we trained a linear svm, it is possible to calculate just the weights and the offset
// prediction is then y_hat = sign(X * w + b)
void svm_get_linear_weights(const svm_model_t& model, std::vector<double>* w, double* b);

#endif
