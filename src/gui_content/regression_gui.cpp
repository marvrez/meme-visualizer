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
        is_trained = false;
    }
    prev_item = curr_item;

    if(colored_button("Generate", 4.5f/7.f)) {
        data = generate_linear_data_2d(num_sample_points, beta, mu, sigma);
        is_trained = false;
    }
    ImGui::SameLine();
    if (colored_button("Load..", 0.0f)) ImGui::OpenPopup("Load data from CSV?");
    if (ImGui::BeginPopupModal("Load data from CSV?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        static char filename[1024];
        static bool init_filename = true;
        ImGui::InputText("Filename", filename, sizeof(filename));
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120,0))) {
            data = csv_to_matrix(filename);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120,0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if(colored_button("Run", 2.f/7.f)) {
        model = make_regression_model(learning_rate, reg_type);
        std::vector<std::vector<float> > x(data.rows);
        std::vector<float> y(data.rows);

        if(reg_type == REGRESSION_LINEAR) {
            for(int i = 0; i < data.rows; ++i) {
                x[i] = { data.vals[i][0] };
                y[i] = data.vals[i][1];
            }
        }
        else if(reg_type == REGRESSION_LOGISTIC) {
            x = data.vals;
            for(int i = 0; i < data.rows; ++i) {
                y[i] = (x[i][1] > beta*x[i][0]) ? 1 : 0;
            }
        }
        double t1 = time_now();
        float error = regression_train(&model, x, y);
        printf("converged after %.3lf seconds\n", time_now() - t1);
        printf("Final error: %.3f\n", error);
        if(reg_type==REGRESSION_LOGISTIC) {
            printf("bias=%.3f, weight=%.3f, weight2=%.3f\n", model.theta[0], model.theta[1], model.theta[2]);
        }
        else if(reg_type==REGRESSION_LINEAR) {
            printf("bias=%.3f, weight=%.3f\n", model.theta[0], model.theta[1]);
        }
        is_trained = true;
    }

    plot_data(data);

    if(is_trained) {
        if(reg_type == REGRESSION_LOGISTIC) {
            glPoints(2.f);
            for(int y = 0; y <= vdb__globals.window_h; y += 4) {
                for(int x = 0; x <= vdb__globals.window_w; x += 4) {
                    static std::vector<float> grid_point(2);
                    vdbWindowToNDC(x, y, &grid_point[0], &grid_point[1]);
                    float pred = regression_predict(model, grid_point);
                    if(pred >= 0.5) glColor4f(150/255.f, 250/255.f, 150/255.f, .5f);
                    else glColor4f(250/255.f,150/255.f, 150/255.f, .5f);
                    glVertex2f(grid_point[0], grid_point[1]);
                }
            }
            glEnd();
        }
        else if(reg_type == REGRESSION_LINEAR) {
            glPoints(2.f);
            glColor4f(1.f, 0.f, 0.f, 1.f);
            for(int x = 0; x <= vdb__globals.window_w; x += 2) {
                float x_normalized = -1.0f + 2.0f * x / vdb__globals.window_w;
                float y_pred = regression_predict(model, { x_normalized });
                glVertex2f(x_normalized, y_pred);
            }
            glEnd();
        }
    }
}
VDBE();
