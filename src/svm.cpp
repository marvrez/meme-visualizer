#include "svm.h"

#include <cassert>
#include <iostream>

svm_kernel_type_t get_kernel_type(const char* s)
{
    if(strcmp(s, "rbf") == 0) return RBF;
    if(strcmp(s, "linear") == 0) return LINEAR;
    return LINEAR;
}

double kernel_linear(const std::vector<double>& v1, const std::vector<double>& v2)
{
    double result = 0;
    for(int i = 0; i< v1.size(); ++i) result += v1[i] * v2[i];
    return result;
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
            f += model.alpha[i] * model.problem.labels[i] * model.kernel(example, model.problem.datum[i]);
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
