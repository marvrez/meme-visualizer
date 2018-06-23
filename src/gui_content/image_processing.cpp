VDBB("Image processing");
{
    vdb2D(-1, +1, +1, -1);
    static ImGuiFs::Dialog dialog;
    static GLenum data_format = GL_RGB;
    static image_t loaded_image = make_image(100,100,3);
    static image_t screen_image = copy_image(loaded_image);

    bool load_button_pressed = colored_button("Load image", 0.125f);

    const char* chosen_path = dialog.chooseFileDialog(load_button_pressed);
    if(strcmp(chosen_path, "")) {
        loaded_image = load_image_rgb(chosen_path);
        screen_image = copy_image(loaded_image);
        save_image_png(screen_image, "lal");
    }

    std::vector<unsigned char> image_data = get_hwc_bytes(screen_image);
    vdbSetTexture(0, image_data.data(), screen_image.w, screen_image.h, data_format, GL_UNSIGNED_BYTE);
    vdbDrawTexture(0);

    static float binary_threshold = 0.f, prev_binary_threshold = 0.f;
    if (ImGui::CollapsingHeader("Binary threshold")) {
        ImGui::SliderFloat("binary_threshold", &binary_threshold, 0.0f, 255.0f);

        if(binary_threshold != prev_binary_threshold) {
            threshold_image(loaded_image, &screen_image, binary_threshold / 255.f);
            data_format = GL_RGB;
        }
        prev_binary_threshold = binary_threshold;
    }

    static float white_threshold_r, white_threshold_g, white_threshold_b, white_threshold_d = 255.f;
    static float prev_threshold_r, prev_threshold_g, prev_threshold_b, prev_threshold_d = 255.f;
    if (ImGui::CollapsingHeader("White threshold")) {
        ImGui::SliderFloat("white_threshold_r", &white_threshold_r, 0.0f, 255.0f);
        ImGui::SliderFloat("white_threshold_g", &white_threshold_g, 0.0f, 255.0f);
        ImGui::SliderFloat("white_threshold_b", &white_threshold_b, 0.0f, 255.0f);
        ImGui::SliderFloat("white_threshold_d", &white_threshold_d, 0.0f, 255.0f);
        if(prev_threshold_r != white_threshold_r || prev_threshold_g != white_threshold_g ||
            prev_threshold_b != white_threshold_b || prev_threshold_d != white_threshold_d)
        {
            threshold_image(loaded_image, 
                            &screen_image, 
                            white_threshold_r / 255.f, 
                            white_threshold_g / 255.f, 
                            white_threshold_b / 255.f, 
                            white_threshold_d / 255.f);
            data_format = GL_LUMINANCE;
        }
        prev_threshold_r = white_threshold_r;
        prev_threshold_g = white_threshold_g;
        prev_threshold_b = white_threshold_b;
        prev_threshold_d = white_threshold_d;
    }

    static float r_g = 1.f, r_b = 1.f, r_n = 0.f;
    if (ImGui::CollapsingHeader("Red thresholds")) {
        ImGui::SliderFloat("red over green (r_g)", &r_g, 1.0f, 10.0f);
        ImGui::SliderFloat("red over blue (r_b)", &r_b, 1.0f, 10.0f);
        ImGui::SliderFloat("minimum brightness (r_n)", &r_n, 0.0f, 255.0f);
    }

    static float g_r = 1.f, g_b = 1.f, g_n = 0.f;
    if (ImGui::CollapsingHeader("Green thresholds")) {
        ImGui::SliderFloat("green over red (g_r)", &g_r, 1.0f, 10.0f);
        ImGui::SliderFloat("green over blue (g_b)", &g_b, 1.0f, 10.0f);
        ImGui::SliderFloat("minimum brightness (g_n)", &g_n, 0.0f, 255.0f);
    }

    static float b_r = 1.f, b_g = 1.f, b_n = 0.f;
    if (ImGui::CollapsingHeader("Blue thresholds")) {
        ImGui::SliderFloat("blue over red (b_r)", &b_r, 1.0f, 10.0f);
        ImGui::SliderFloat("blue over green (b_g)", &b_g, 1.0f, 10.0f);
        ImGui::SliderFloat("minimum brightness (b_n)", &b_n, 0.0f, 255.0f);
    }

    if(colored_button("Reset", 0.f)) {
        cc_options_t opt = { r_g, r_b, r_n/255.f, g_r, g_b, g_n/255.f, b_r, b_g, b_n/255.f };
        std::vector<int> points;
        auto test = connected_components(loaded_image, opt, &points);
        screen_image = loaded_image;
        data_format = GL_RGB;
    }
}
VDBE();
