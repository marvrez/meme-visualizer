VDBB("Image processing");
{
    static ImGuiFs::Dialog dialog;
    static image_t loaded_image = make_image(100,100,3);
    static image_t screen_image = copy_image(loaded_image);

    bool load_button_pressed = colored_button("Load image", 0.125f);

    const char* chosen_path = dialog.chooseFileDialog(load_button_pressed);
    if(strcmp(chosen_path, "")) {
        loaded_image = load_image_rgb(chosen_path);
        screen_image = copy_image(loaded_image);
    }

    unsigned char* image_data = get_hwc_bytes(screen_image).data();
    vdbSetTexture(0, image_data, screen_image.w, screen_image.h, GL_RGB);
    vdbDrawTexture(0);

    static float binary_threshold = 0.f, prev_binary_threshold = 0.f;
    static float white_threshold_r, white_threshold_g, white_threshold_b, white_threshold_a;
    if (ImGui::CollapsingHeader("Binary threshold")) {
        ImGui::SliderFloat("binary_threshold", &binary_threshold, 0.0f, 255.0f);
        if(prev_binary_threshold != binary_threshold) {
            threshold_image(loaded_image, &screen_image, binary_threshold / 255.f);
        }
        prev_binary_threshold = binary_threshold;
    }

    if (ImGui::CollapsingHeader("White threshold")) {
        ImGui::SliderFloat("white_threshold_r", &white_threshold_r, 0.0f, 255.0f);
        ImGui::SliderFloat("white_threshold_g", &white_threshold_g, 0.0f, 255.0f);
        ImGui::SliderFloat("white_threshold_b", &white_threshold_b, 0.0f, 255.0f);
        ImGui::SliderFloat("white_threshold_a", &white_threshold_a, 0.0f, 255.0f);
    }

    static float r_g, r_b, r_n;
    if (ImGui::CollapsingHeader("Red thresholds"))
    {
        ImGui::SliderFloat("red over green (r_g)", &r_g, 1.0f, 10.0f);
        ImGui::SliderFloat("red over blue (r_b)", &r_b, 1.0f, 10.0f);
        ImGui::SliderFloat("minimum brightness (r_n)", &r_n, 0.0f, 255.0f);
    }

    static float g_r, g_b, g_n;
    if (ImGui::CollapsingHeader("Green thresholds"))
    {
        ImGui::SliderFloat("green over red (g_r)", &g_r, 1.0f, 10.0f);
        ImGui::SliderFloat("green over blue (g_b)", &g_b, 1.0f, 10.0f);
        ImGui::SliderFloat("minimum brightness (g_n)", &g_n, 0.0f, 255.0f);
    }

    static float b_r, b_g, b_n;
    if (ImGui::CollapsingHeader("Blue thresholds"))
    {
        ImGui::SliderFloat("blue over red (b_r)", &b_r, 1.0f, 10.0f);
        ImGui::SliderFloat("blue over green (b_g)", &b_g, 1.0f, 10.0f);
        ImGui::SliderFloat("minimum brightness (b_n)", &b_n, 0.0f, 255.0f);
    }

    if(colored_button("Reset", 0.75f)) {
        screen_image = loaded_image;
    }
}
VDBE();
