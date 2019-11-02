#include "svm.h"

#include <cassert>
#include <cmath>
#include <cstring>
#include <algorithm>


#include "rng.h"

svm_kernel_type_t get_kernel_type(const char* s)
{
    if(strcmp(s, "rbf") == 0) return RBF;
    if(strcmp(s, "linear") == 0) return LINEAR;
    return LINEAR;
}

float (*get_kernel_function(svm_kernel_type_t type))(const kernel_t&, const std::vector<float>&, const std::vector<float>&)
{
    switch(type) {
        case RBF:
            return kernel_rbf;
        case LINEAR:
            return kernel_linear;
    }
    return kernel_rbf;
}

kernel_t make_kernel(svm_kernel_type_t type, std::vector<std::vector<float> >* x, bool use_cache, float gamma)
{
    kernel_t kernel;
    kernel.x = x;
    kernel.gamma = gamma;
    kernel.kernel_function = get_kernel_function(type);

    if(use_cache) {
        kernel.kernel_cache = std::vector<std::vector<float> >(x->size(), std::vector<float>(x->size(), 0));
        for (int i = 0; i < x->size(); ++i) {
            for (int j = 0; j < x->size(); ++j) {
                kernel.kernel_cache[i][j] = kernel.kernel_function(kernel, (*x)[i],(*x)[j]);
            }
        }
    }

    return kernel;
}

float kernel_compute(const kernel_t& kernel, int i, int j)
{
    if(kernel.kernel_cache.size() > 0) return kernel.kernel_cache[i][j];
    return kernel.kernel_function(kernel, (*kernel.x)[i], (*kernel.x)[j]);
}

float kernel_linear(const kernel_t& kernel, const std::vector<float>& v1, const std::vector<float>& v2)
{
    float result = 0.f;
    for(int i = 0; i < v1.size(); ++i) result += v1[i] * v2[i];
    return result;
}

float kernel_rbf(const kernel_t& kernel, const std::vector<float>& v1, const std::vector<float>& v2)
{
    float dist_squared = 0.f;
    for(int i = 0; i < v1.size(); ++i) dist_squared += (v1[i] - v2[i])*(v1[i] - v2[i]);
    return exp(-kernel.gamma * dist_squared);
}

svm_problem_t svm_make_problem(const std::vector<std::vector<float> >& datum, const std::vector<int>& labels)
{
    svm_problem_t problem;
    problem.datum  = datum;
    problem.labels = labels;
    return problem;
}

svm_model_t svm_make_model(svm_problem_t problem, const svm_parameter_t& param)
{
    svm_model_t model;
    model.param = param;
    model.problem = problem;
    model.kernel = make_kernel(param.kernel_type, &problem.datum, param.do_cache, param.gamma);
    model.N = problem.datum.size(), model.D = problem.datum[0].size();
    model.alpha = std::vector<float>(model.N, 0.f);
    return model;
}

svm_model_t svm_train(svm_problem_t problem, const svm_parameter_t& param)
{
    svm_model_t model = svm_make_model(problem, param);
    
    // The SMO algorithm
    int iter = 0, passes = 0;
    RNG rng(0, model.N - 1);
    while(passes < param.num_passes && iter < param.max_iter) {
        int num_alpha_changed = 0;
        for(int i = 0; i < model.N; ++i) {
            float e_i = svm_margin(model, problem.datum[i]) - problem.labels[i];
            if( (problem.labels[i]*e_i < -param.tol && model.alpha[i] < param.C)
            || (problem.labels[i]*e_i > param.tol && model.alpha[i] > 0) ) {
                // alpha_i needs update. Pick a j to update it with
                int j = i;
                while(j == i) j = rng.getInt(); 
                float e_j = svm_margin(model, problem.datum[j]) - problem.labels[j]; // can think of this as error between SVM output and j-th example

                float a_i = model.alpha[i], a_j = model.alpha[j]; // store old alphas
                // calculate L and H bounds for j to ensure we're in [0,C]x[0,C] box
                float L = std::max((float)0, problem.labels[i] == problem.labels[j] ? a_i + a_j - param.C : a_j - a_i),
                       H = std::min(param.C, problem.labels[i] == problem.labels[j] ? a_i + a_j : a_j - a_i + param.C);

                if(fabs(L - H) < 1e-4) continue;

                float eta = 2*kernel_compute(model.kernel, i,j) - kernel_compute(model.kernel, i,i) - kernel_compute(model.kernel, j,j);
                if(eta >= 0) continue;

                // compute new alpha_j and clip it inside [0,C]x[0,C] box
                // then compute alpha_i based on it.
                float new_a_j = a_j - problem.labels[j]*(e_i-e_j) / eta;
                if (new_a_j > H)  new_a_j = H;
                if (new_a_j < L)  new_a_j = L;
                if(fabs(new_a_j - a_j) < 1e-4) continue;
                float new_a_i = a_i + problem.labels[i]*problem.labels[j]*(a_j - new_a_j);
                model.alpha[i] = new_a_i, model.alpha[j] = new_a_j;

                // update the bias term
                float expr1 = problem.labels[i]*(new_a_i-a_i), expr2 = problem.labels[j]*(new_a_j-a_j);
                float b1 = model.b - e_i - expr1*kernel_compute(model.kernel, i,i) - expr2*kernel_compute(model.kernel, i,j);
                float b2 = model.b - e_j - expr1*kernel_compute(model.kernel, i,j) - expr2*kernel_compute(model.kernel, j,j);
                model.b = (b1 + b2) / 2;
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
        model.w = std::vector<float>(model.D);
        for(int j = 0; j < model.D; ++j) {
            float s = 0.0f;
            for(int i = 0; i < model.N; ++i){
                s += model.alpha[i] * model.problem.labels[i] * model.problem.datum[i][j];
            }
            model.w[j] = s;
        }
        model.use_w = true;
    } 
    else {
        // Filter out training data that has alpha[i] less than alpha_tol, as they are irrelevant for future
        std::vector<std::vector<float> > new_datum = std::vector<std::vector<float> >();
        std::vector<int> new_labels = std::vector<int>();
        std::vector<float> new_alpha= std::vector<float>();
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

svm_model_t svm_train(const std::vector<std::vector<float> >& datum, const std::vector<int>& labels)
{
    svm_parameter_t param;
    return svm_train({ datum, labels }, param);
}

float svm_margin(const svm_model_t& model, const std::vector<float>& example)
{
    float f = model.b;

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
            f += model.alpha[i] * model.problem.labels[i] 
                * model.kernel.kernel_function(model.kernel, example, model.problem.datum[i]);
        }
    }

    return f;
}

std::vector<float> svm_margins(const svm_model_t& model, const std::vector<std::vector<float> >& data)
{
    // go over support vectors and accumulate the prediction.
    int N = data.size();
    std::vector<float> margins(N);
    for(int i = 0; i < N; ++i) {
        margins[i] = svm_margin(model, data[i]);
    }
    return margins;
}

int svm_predict(const svm_model_t& model, const std::vector<float>& example)
{
    return svm_margin(model, example) > 0.f ? 1 : -1;
}

std::vector<int> svm_predict(const svm_model_t& model, const std::vector<std::vector<float> >& data)
{
    std::vector<int> predictions(data.size());
    std::vector<float> margins = svm_margins(model, data);
    for(int i = 0; i < margins.size(); ++i) {
        predictions[i] = margins[i] > 0.f ? 1 : -1;
    }
    return predictions;
}

void svm_get_linear_weights(const svm_model_t& model, std::vector<float>* w, float* b)
{
    assert(model.param.kernel_type == LINEAR);
    *w = std::vector<float>(model.D);
    for(int j = 0; j < model.D; ++j) {
        float s = 0.f;
        for(int i = 0; i < model.N; ++i) {
            s += model.alpha[i] * model.problem.labels[i] * model.problem.datum[i][j];
        }
        (*w)[j] = s;
    }
    *b = model.b;
}
