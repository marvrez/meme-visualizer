#include "svm.h"

#include <cassert>
#include <cmath>
#include <algorithm>


#include "rng.h"

svm_kernel_type_t get_kernel_type(const char* s)
{
    if(strcmp(s, "rbf") == 0) return RBF;
    if(strcmp(s, "linear") == 0) return LINEAR;
    return LINEAR;
}

double (*get_kernel_function(svm_kernel_type_t type))(const kernel_t&, const std::vector<double>&, const std::vector<double>&)
{
    switch(type) {
        case RBF:
            return kernel_rbf;
        case LINEAR:
            return kernel_linear;
    }
    return kernel_rbf;
}

kernel_t make_kernel(svm_kernel_type_t type, std::vector<std::vector<double> >* x, bool use_cache, double gamma)
{
    kernel_t kernel;
    kernel.x = x;
    kernel.gamma = gamma;
    kernel.kernel_function = get_kernel_function(type);

    if(use_cache) {
        kernel.kernel_cache = std::vector<std::vector<double> >(x->size(), std::vector<double>(x->size(), 0));
        for (int i = 0; i < x->size(); ++i) {
            for (int j = 0; j < x->size(); ++j) {
                kernel.kernel_cache[i][j] = kernel.kernel_function(kernel, (*x)[i],(*x)[j]);
            }
        }
    }

    return kernel;
}

double kernel_compute(const kernel_t& kernel, int i, int j)
{
    if(kernel.kernel_cache.size() > 0) return kernel.kernel_cache[i][j];
    return kernel.kernel_function(kernel, (*kernel.x)[i], (*kernel.x)[j]);
}

double kernel_linear(const kernel_t& kernel, const std::vector<double>& v1, const std::vector<double>& v2)
{
    double result = 0.f;
    for(int i = 0; i < v1.size(); ++i) result += v1[i] * v2[i];
    return result;
}

double kernel_rbf(const kernel_t& kernel, const std::vector<double>& v1, const std::vector<double>& v2)
{
    double dist_squared = 0.f;
    for(int i = 0; i < v1.size(); ++i) dist_squared += (v1[i] - v2[i])*(v1[i] - v2[i]);
    return exp(-kernel.gamma * dist_squared);
}

svm_model_t svm_train(svm_problem_t problem, const svm_parameter_t& param)
{
    svm_model_t model;
    model.param = param;
    model.problem = problem;
    model.kernel = make_kernel(param.kernel_type, &problem.datum, param.do_cache, param.gamma);
    model.N = problem.datum.size(), model.D = problem.datum[0].size();
    model.alpha = std::vector<double>(model.N, 0.f);
    
    // The SMO algorithm
    int iter = 0, passes = 0;
    RNG rng(0, model.N - 1);
    while(passes < param.num_passes && iter < param.max_iter) {
        int num_alpha_changed = 0;
        for(int i = 0; i < model.N; ++i) {
            double e_i = svm_margin(model, problem.datum[i]) - problem.labels[i];
            if( (problem.labels[i]*e_i < -param.tol && model.alpha[i] < param.C)
            || (problem.labels[i]*e_i > param.tol && model.alpha[i] > 0) ) {
                // alpha_i needs update. Pick a j to update it with
                int j = i;
                while(j == i) j = rng.getInt(); 
                double e_j = svm_margin(model, problem.datum[j]) - problem.labels[j]; // can think of this as error between SVM output and j-th example

                double a_i = model.alpha[i], a_j = model.alpha[j]; // store old alphas
                // calculate L and H bounds for j to ensure we're in [0,C]x[0,C] box
                double L = std::max((double)0, problem.labels[i] == problem.labels[j] ? a_i + a_j - param.C : a_j - a_i),
                       H = std::min(param.C, problem.labels[i] == problem.labels[j] ? a_i + a_j : a_j - a_i + param.C);

                if(fabs(L - H) < 1e-4) continue;

                double eta = 2*kernel_compute(model.kernel, i,j) - kernel_compute(model.kernel, i,i) - kernel_compute(model.kernel, j,j);
                if(eta >= 0) continue;

                // compute new alpha_j and clip it inside [0,C]x[0,C] box
                // then compute alpha_i based on it.
                double new_a_j = a_j - problem.labels[j]*(e_i-e_j) / eta;
                new_a_j = (new_a_j > H) ? H : (new_a_j < L) ? L : new_a_j;
                if(fabs(new_a_j - a_j) < 1e-4) continue;
                double new_a_i = a_i + problem.labels[i]*problem.labels[j]*(a_j - new_a_j);
                model.alpha[i] = new_a_i, model.alpha[j] = new_a_j;

                // update the bias term
                double expr = problem.labels[i]*(new_a_i-a_i)*kernel_compute(model.kernel, i,i) - problem.labels[j]*(new_a_j-a_j)*kernel_compute(model.kernel, i,j);
                double b1 = model.b - e_i - expr, b2 = model.b - e_j - expr;
                model.b = 0.5*(b1 + b2);
                if(new_a_i > 0 && new_a_i < param.C) model.b = b1;
                if(new_a_j > 0 && new_a_j < param.C) model.b = b2;

                num_alpha_changed++;
            } 
        } 
        iter++;
        passes = num_alpha_changed == 0 ? passes + 1 : 0;
    } 

    // Speed up evaluation during test time by caching weights if using linear kernel
    if(param.kernel_type == LINEAR) {
        model.w = std::vector<double>(model.D);
        for(int j = 0; j < model.D; ++j) {
            double s = 0.0f;
            for(int i = 0; i < model.N; ++i){
                s += model.alpha[i] * model.problem.labels[i] * model.problem.datum[i][j];
            }
            model.w[j] = s;
        }
        model.use_w = true;
    } 
    else {
        // Filter out training data that has alpha[i] = 0, as they are irrelevant for future
        std::vector<std::vector<double> > new_datum = std::vector<std::vector<double> >();
        std::vector<int> new_labels = std::vector<int>();
        std::vector<double> new_alpha= std::vector<double>();
        for(int i = 0; i < model.N; ++i) {
            if(model.alpha[i] > param.alpha_tol) {
                new_datum.push_back(model.problem.datum[i]);
                new_labels.push_back(model.problem.labels[i]);
                new_alpha.push_back(model.alpha[i]);
            }
        }
        model.problem.datum = new_datum;
        model.problem.labels = new_labels;
        model.alpha = new_alpha;
        model.N = model.problem.datum.size();
    }

    model.num_iter = iter;

    return model;
}

svm_model_t svm_train(const std::vector<std::vector<double> >& datum, const std::vector<int>& labels)
{
    svm_parameter_t param;
    return svm_train({ datum, labels }, param);
}

double svm_margin(const svm_model_t& model, const std::vector<double>& example) 
{
    double f = model.b;

    // if the linear kernel was used and w was computed and stored, 
    // (i.e. the svm has fully finished training)
    // the internal variable use_w will be set to true.
    if(model.use_w) {
        // we can speed this up a lot by using the computed weights
        // we computed these during train(). This is significantly faster 
        // than the version below
        for(int i = 0; i < model.D; ++i) {
            f += example[i] * model.w[i];
        }
    } 
    else {
        for(int i = 0; i < model.N; ++i) {
            f += model.alpha[i] * model.problem.labels[i] * model.kernel.kernel_function(model.kernel, example, model.problem.datum[i]);
        }
    }

    return f;
}

std::vector<double> svm_margins(const svm_model_t& model, const std::vector<std::vector<double> >& data)
{
    // go over support vectors and accumulate the prediction.
    int N = data.size();
    std::vector<double> margins(N);
    for(int i = 0; i < N; ++i) {
        margins[i] = svm_margin(model, data[i]);
    }
    return margins;
}

int svm_predict(const svm_model_t& model, const std::vector<double>& example)
{
    return svm_margin(model, example) > 0.f ? 1 : -1;
}

std::vector<int> svm_predict(const svm_model_t& model, const std::vector<std::vector<double> >& data)
{
    std::vector<int> predictions(data.size());
    std::vector<double> margins = svm_margins(model, data);
    for(int i = 0; i < margins.size(); ++i) {
        predictions[i] = margins[i] > 0.f ? 1 : -1;
    }
    return predictions;
}

void svm_get_linear_weights(const svm_model_t& model, std::vector<double>* w, double* b)
{
    assert(model.param.kernel_type == LINEAR);
    *w = std::vector<double>(model.D);
    for(int j = 0; j < model.D; ++j) {
        double s = 0.f;
        for(int i = 0; i < model.N; ++i) {
            s += model.alpha[i] * model.problem.labels[i] * model.problem.datum[i][j];
        }
        (*w)[j] = s;
    }
    *b = model.b;
}
