VDBB("Binary SVC");
{
    #define POSITIVE_EXAMPLE 1
    #define NEGATIVE_EXAMPLE -1
    static std::vector<int> labels;
    static matrix_t svm_data = my_svm_data_init(&labels);
    static svm_parameter_t param;
    static svm_kernel_type_t kernel_type;
    static float C = 1.0f, rbf_sigma = 1.f;
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

    ImGui::SliderFloat("C", &C, 0.01f, 100.0f, "C = %.3lf");
    param.C = C;

    static const char* items[] = {"rbf", "linear"};
    static int curr_item = -1, prev_item = -1;
    ImGui::Combo("kernel type", &curr_item, items, IM_ARRAYSIZE(items)); 
    if(curr_item != prev_item) {
        kernel_type = get_kernel_type(items[curr_item]);
        svm_need_retrain = true;
    }
    prev_item = curr_item;
    if(kernel_type == RBF) {
        ImGui::SliderFloat("rbf kernel sigma", &rbf_sigma, 0.01f, 100.0f, "rbf_sigma = %.3f");
        float gamma = 1/(2*rbf_sigma*rbf_sigma);
        svm_need_retrain = gamma != param.gamma ? true : false;
        param.gamma = gamma;
    }
    
    if(colored_button("Reset", 0.f)) {
        clear_matrix(&svm_data);
        labels.clear();
        data_types.clear();
        svm_data = my_svm_data_init(&labels);
        svm_need_retrain = true;
    }

    param.labels = labels;
    param.datum.reserve(svm_data.rows);
    for (auto&& v : svm_data.vals) param.datum.emplace_back(std::begin(v), std::end(v));
    plot_data(svm_data);

    if(vdbLeftPressed()) {
        ImVec2 mouse = ImGui::GetMousePos();
        static std::vector<float> mouse_pos(2);
        bool shift_key_down = vdbKeyCodeDown(SDL_SCANCODE_LSHIFT);
        vdbWindowToNDC(mouse.x, mouse.y, &mouse_pos[0], &mouse_pos[1]);
        svm_data = concat_matrix(svm_data, { 1, 2, { mouse_pos } });
        labels.push_back(shift_key_down ? POSITIVE_EXAMPLE : NEGATIVE_EXAMPLE);
        data_types[mouse_pos] = shift_key_down ? SVM_POSITIVE_EXAMPLE : SVM_NEGATIVE_EXAMPLE;
        svm_need_retrain = true;
    }

    if(svm_need_retrain) {
        //svm_train(param);
        svm_need_retrain = false;
    }
}
VDBE();
