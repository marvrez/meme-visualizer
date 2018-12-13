VDBB("Regression");
{
    static int num_sample_points = 500;
    static float beta = 1.f;
    static float mu = 0.f, sigma = 0.25f;
    static matrix_t data = generate_linear_data_2d(num_sample_points, beta, mu, sigma);
    static regression_model_t model;
    static regression_type_t reg_type = REGRESSION_LOGISTIC;
    static bool is_trained = false;
    static float learning_rate = 0.1f;

    vdb2D(-1, +1, -1, +1);

    // Draw grid
    glLines(5.0f);
    glColor4f(1, 1, 1, 0.5f);
    vdbGridXY(-1, +1, -1, +1, 2);
    glEnd();

    ImGui::Text("Input data parameters:");
    ImGui::SliderFloat("beta (scale factor)", &beta, -1.f, 1.f, "beta = %.3f");
    ImGui::SliderInt("num points", &num_sample_points, 0, 5000);

    ImGui::Text("Error term parameters:");
    ImGui::SliderFloat("mean", &mu, -1.f, 1.f, "mu = %.3f");
    ImGui::SliderFloat("variance", &sigma, 0.f, 0.5f, "sigma = %.3f");

    ImGui::Text("Regression model parameters:");
    ImGui::SliderFloat("learning rate", &learning_rate, 0.f, 1.f, "lr = %.3f");
    static const char* items[] = {"logistic", "linear"};
    static int curr_item = 0, prev_item = -1;
    ImGui::Combo("regression type", &curr_item, items, IM_ARRAYSIZE(items)); 
    if(curr_item != prev_item) {
        reg_type = get_regression_type(items[curr_item]);
    }
    prev_item = curr_item;

    if(colored_button("Generate", 4.5f/7.f)) {
        data = generate_linear_data_2d(num_sample_points, beta, mu, sigma);
        is_trained = false;
    }
    ImGui::SameLine();
    if(colored_button("Run", 2.f/7.f)) {
        model = make_regression_model(learning_rate, reg_type);
        //regression_train(&model, );
        is_trained = true;
    }

    plot_data(data);

    if(is_trained) {
        if(reg_type == REGRESSION_LOGISTIC) {
        }
        else if(reg_type == REGRESSION_LINEAR) {
        }
    }
}
VDBE();
