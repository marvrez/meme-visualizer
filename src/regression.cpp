#include "regression.h"

#include <cassert>
#include <cstring>

float hypothesis_linear(const std::vector<float>& x, const std::vector<float>& theta)
{
    float sum = theta[0];
    for(int i = 0; i < x.size(); ++i) {
        sum += x[i]*theta[i+1];
    }
    return sum;
}

float hypothesis_logistic(const std::vector<float>& x, const std::vector<float>& theta)
{
    return logistic(hypothesis_linear(x, theta));
}

float(*get_hypothesis_function(regression_type_t type))(const std::vector<float>&, const std::vector<float>&)
{
    switch(type) {
        case REGRESSION_LOGISTIC:
            return hypothesis_logistic;
        case REGRESSION_LINEAR:
            return hypothesis_linear;
    }
    return hypothesis_logistic;
}

float cross_entropy(float prediction, float target)
{
    return -(target*log(prediction) + (1-target)*log(1-prediction));
}

float mse(float prediction, float target)
{
    return (prediction - target)*(prediction - target);
}

float (*get_cost_function(regression_type_t type))(float, float)
{
    switch(type) {
        case REGRESSION_LOGISTIC:
            return cross_entropy;
        case REGRESSION_LINEAR:
            return mse;
    }
    return cross_entropy;
}

regression_model_t make_regression_model(float learning_rate, regression_type_t type)
{
    regression_model_t model;
    model.learning_rate = learning_rate;
    model.type = type;
    model.theta = std::vector<float>(0);
    return model;
}

regression_type_t get_regression_type(const char* s)
{
    if(strcmp(s, "logistic") == 0) return REGRESSION_LOGISTIC;
    if(strcmp(s, "linear") == 0) return REGRESSION_LINEAR;
    return REGRESSION_LOGISTIC;
}

float regression_train(regression_model_t* model, const std::vector<std::vector<float> >& x, const std::vector<float>& y)
{
    assert(x.size() > 0);
    assert(x.size() == y.size());
    int n = x.size(), d = x[0].size();
    float loss_sum = 0, curr_loss = 0, prev_loss = -1, tol = 1e-4;

    model->theta = std::vector<float>(d+1, 0.f); // +1 for bias

    float (*hypothesis)(const std::vector<float>&,
                const std::vector<float>&) = get_hypothesis_function(model->type);
    float (*cost_function)(float, float) = get_cost_function(model->type);

    while(fabsf(curr_loss - prev_loss) >= tol) {
        prev_loss = curr_loss;
        for (int i = 0; i < n; ++i) {
            float pred = hypothesis(x[i], model->theta);
            float error = pred - y[i];
            // Gradient descent: update each parameters to minimize error
            model->theta[0] = model->theta[0] - model->learning_rate*error; // update bias separately
            for(int j = 1; j < d+1; ++j) {
                model->theta[j] = model->theta[j] - model->learning_rate*error*x[i][j-1];
            }
            curr_loss = cost_function(pred, y[i]);
            loss_sum += curr_loss;
        }
    }

	return loss_sum / n;
}

float regression_predict(const regression_model_t& model, const std::vector<float>& example)
{
    float (*hypothesis)(const std::vector<float>&, const std::vector<float>&) = get_hypothesis_function(model.type);
    return hypothesis(example, model.theta);
}
