#ifndef REGRESSION_H
#define REGRESSION_H

#include <vector>
#include <cmath>

static inline float logistic(float val){return 1.f/(1.f+expf(-val));}

typedef enum {
    REGRESSION_LOGISTIC,
    REGRESSION_LINEAR
} regression_type_t ;
regression_type_t get_regression_type(const char* s);

float hypothesis_linear(const std::vector<float>& x, const std::vector<float>& theta);
float hypothesis_logistic(const std::vector<float>& x, const std::vector<float>& theta);
float (*get_hypothesis_function(regression_type_t type))(const std::vector<float>&, const std::vector<float>&);

float cross_entropy(float prediction, float target);
float mse(float prediction, float target);
float (*get_cost_function(regression_type_t type))(float, float);

typedef struct {
    regression_type_t type;
    std::vector<float> theta; // theta[0] is the bias
    float learning_rate;
} regression_model_t ;

regression_model_t make_regression_model(float learning_rate, regression_type_t type);
// returns loss from training the model
float regression_train(regression_model_t* model, const std::vector<std::vector<float> >& x, const std::vector<float>& y);
float regression_predict(const regression_model_t& model, const std::vector<float>& example);

#endif
