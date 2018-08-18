/*
    This is a binary SVM and is trained using the SMO algorithm.
    Reference: "The Simplified SMO Algorithm" (http://math.unt.edu/~hsp0009/smo.pdf)
*/
#ifndef SVM_H
#define SVM_H
#include <vector>

typedef enum {
    RBF,
    LINEAR
} svm_kernel_type_t;

typedef struct {
    double C = 1.0f;
    double eps = 1e-4; // numerical tolerance. Don't touch unless you're pro
    double alpha_tol = 1e-7; // non-support vectors for space and time efficiency are truncated. To guarantee correct result set this to 0 to do no truncating. If you want to increase efficiency, experiment with setting this little higher, up to maybe 1e-4 or so.

    long long max_iter = 10000; // max number of iterations
    int num_passes = 10; // how many passes over data with no change before we halt? Increase for more precision.

    bool do_cache = true; // cache kernel computations to avoid expensive recomputations. Could use too much memory if N is too large.

    double gamma = 2.f; // for rbf, equivalent to 1/2*sigma^2

    svm_kernel_type_t kernel_type = RBF; // Currently between RBF and LINEAR kernel

    std::vector<std::vector<double> > datum;
    std::vector<int> labels;

} svm_parameter_t;

typedef struct {
    svm_parameter_t param;
    int N, D; // N is number of data points, D is the dimension of the data
    std::vector<double> alpha;
    double b = 0;
    bool use_w = false;  // internal efficiency flag

    double (*kernel)(const std::vector<double>& v1, const std::vector<double>& v2);

    std::vector<double> w; // cache weights for linear kernels to speed up evaluation during test time.
} svm_model_t;

double kernel_linear(const std::vector<double>& v1, const std::vector<double>& v2);
double kernel_rbf(const std::vector<double>& v1, const std::vector<double>& v2);

// data is NxD array of doubles. labels are 1 or -1.
void svm_train(const svm_parameter_t& param, svm_model_t* model, int* train_iter);
void svm_train(const std::vector<std::vector<double> >& datum, const std::vector<int>& labels);

// inst is an array of length D. Returns margin of given example
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
