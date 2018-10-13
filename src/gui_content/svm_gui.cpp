VDBB("Binary SVC");
{
    #define POSITIVE_EXAMPLE 1
    #define NEGATIVE_EXAMPLE -1
    static std::vector<int> labels;
    static matrix_t svm_data = my_svm_data_init(&labels);
    static svm_parameter_t param;
    static svm_model_t model;
    static float C = 1.0f, rbf_sigma = 0.1f;
    static bool svm_need_retrain = true;

    vdb2D(-1, +1, -1, +1);

    // Draw grid
    glLines(5.0f);
    glColor4f(1, 1, 1, 0.5f);
    vdbGridXY(-1, +1, -1, +1, 2);
    glEnd();

    // Text
    ImGui::Text("mouse click: add"); ImGui::SameLine();
    ImGui::TextColored(ImVec4(1,0,0,1), "red"); ImGui::SameLine();
    ImGui::Text("data point."); 

    ImGui::AlignFirstTextHeightToWidgets();
    ImGui::Text("shift + mouse click: add"); ImGui::SameLine();
    ImGui::TextColored(ImVec4(0,1,0,1), "green"); ImGui::SameLine();
    ImGui::Text("data point."); 

    ImGui::SliderFloat("C", &C, 0.f, 5.f, "C = %.3f");
    svm_need_retrain = C != param.C ? true : svm_need_retrain;
    param.C = C;

    static const char* items[] = {"rbf", "linear"};
    static int curr_item = 0, prev_item = -1;
    ImGui::Combo("kernel type", &curr_item, items, IM_ARRAYSIZE(items)); 
    if(curr_item != prev_item) {
        param.kernel_type = get_kernel_type(items[curr_item]);
        svm_need_retrain = true;
    }
    prev_item = curr_item;
    if(param.kernel_type == RBF) {
        ImGui::SliderFloat("rbf kernel sigma", &rbf_sigma, 0.f, 2.f, "rbf_sigma = %.3f");
        float gamma = 1/(2*rbf_sigma*rbf_sigma);
        svm_need_retrain = gamma != param.gamma ? true : svm_need_retrain;
        param.gamma = gamma;
    }
    
    if(colored_button("Reset", 0.f)) {
        clear_matrix(&svm_data);
        labels.clear();
        data_types.clear();
        svm_data = my_svm_data_init(&labels);
        svm_need_retrain = true;
    }

    plot_data(svm_data);

    if(vdbLeftPressed()) {
        ImVec2 mouse = ImGui::GetMousePos();
        std::vector<float> mouse_pos(2);
        bool shift_key_down = vdbKeyCodeDown(SDL_SCANCODE_LSHIFT);
        vdbWindowToNDC(mouse.x, mouse.y, &mouse_pos[0], &mouse_pos[1]);
        svm_data = concat_matrix(svm_data, { 1, 2, { mouse_pos } });
        labels.push_back(shift_key_down ? POSITIVE_EXAMPLE : NEGATIVE_EXAMPLE);
        data_types[mouse_pos] = shift_key_down ? SVM_POSITIVE_EXAMPLE : SVM_NEGATIVE_EXAMPLE;
        svm_need_retrain = true;
    }

    if(svm_need_retrain) {
        static svm_problem_t problem;
        problem.datum.clear();
        problem.labels = labels;
        for (auto&& v : svm_data.vals) problem.datum.emplace_back(std::begin(v), std::end(v));

        model = svm_train(problem, param);
        svm_need_retrain = false;
    }

    ImGui::Text("Took %d iterations to finish training.", model.num_iter);

    glPoints(2.f);
    for(int y = 0; y <= vdb__globals.window_h; y += 4) {
        for(int x = 0; x <= vdb__globals.window_w; x += 4) {
            static std::vector<float> grid_point(2);
            vdbWindowToNDC(x, y, &grid_point[0], &grid_point[1]);
            int dec = svm_predict(model, {(double) grid_point[0], (double)grid_point[1]});
            if(dec == POSITIVE_EXAMPLE) glColor4f(150/255.f, 250/255.f, 150/255.f, .5f);
            else glColor4f(250/255.f,150/255.f, 150/255.f, .5f);
            glVertex2f(grid_point[0], grid_point[1]);
        }
    }
    glEnd();

}
VDBE();
