#include "vdb/vdb.h"
#include "image.h"
#include "filter_image.h"
#include "color_utils.h"
#include "utilities.h"
#include "pca.h"
#include "matrix.h"
#include "data_gen.h"
#include "kmeans.h"
#include "svm.h"
#include "regression.h"
#include "rng.h"
#include "connected_components.h"

#include "vdb/imguifilesystem.h"

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <map>

typedef enum {
    KMEANS_CLUSTER,
    KMEANS_CENTROID,
    SVM_POSITIVE_EXAMPLE,
    SVM_NEGATIVE_EXAMPLE,
    REGRESSION_POSITIVE_EXAMPLE,
    REGRESSION_NEGATIVE_EXAMPLE,
    PLAIN_DATA
} data_type_t;

static std::map<std::vector<float>, data_type_t> data_types;
static std::map<std::vector<float>, color_t> centroid_colors, cluster_data_colors;
constexpr float RADIUS = 4.0f, HIGHLIGHT_RADIUS = RADIUS * 2, X_RADIUS = 0.01f, LINE_THICKNESS = 2.5f;

matrix_t my_svm_data_init(std::vector<int>* labels)
{
    matrix_t data = make_matrix(10, 2);
    *labels = std::vector<int>(data.rows);
    data.vals[0]={-0.4326, 1.1909}; data.vals[1]={3.0, 4.0}; data.vals[2]={0.1253, -0.0376};
    data.vals[3]={0.2877, 0.3273}; data.vals[4]={-1.1465, 0.1746}; data.vals[5]={1.8133, 2.1139};
    data.vals[6]={2.7258, 3.0668}; data.vals[7]={1.4117, 2.0593}; data.vals[8]={4.1832, 1.9044};
    data.vals[9]={1.8636, 1.1677};

    float norm = 0.f;
    for(int i = 0; i < data.rows; ++i) {
        for(int j = 0; j < data.cols; ++j) {
            norm += data.vals[i][j]*data.vals[i][j];
        }
    }
    norm = sqrtf(norm);
    for(int i = 0; i < data.rows; ++i) {
        for(int j = 0; j < data.cols; ++j) {
            data.vals[i][j] /= norm;
        }
        (*labels)[i] = i < data.rows / 2 ? 1 : -1;
        data_types[data.vals[i]] = i < 5 ? SVM_POSITIVE_EXAMPLE : SVM_NEGATIVE_EXAMPLE;
    }
    return data;
}

void draw_line(float x1, float y1, float x2, float y2, color_t c) 
{
    glColor4fv(c.data);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
}

void draw_x(float x, float y, float radius, color_t c) 
{
    glColor4fv(c.data);
    glLines(LINE_THICKNESS);
    draw_line(x-radius, y-radius, x+radius, y+radius, c);
    draw_line(x+radius, y-radius, x-radius, y+radius, c);
    glEnd();
}

void draw_point(float x, float y, float r, color_t c)
{
    glColor4fv(c.data);
    glPoints(r);
    glVertex2f(x, y);
    glEnd();
}

void draw_arrow(float x1, float y1, float x2, float y2, color_t c)
{
    float x_len = x2-x1, y_len = y2-y1;
    float line_angle = x_len ? atan(y_len/x_len) : M_PI / 2;
    float s = ((x2 >= x1 && y2 > y1) || (x2 > x1 && y2 <= y1)) ? 0.05 : -0.05;

    if(x_len == 0 && y_len == 0) return;

    glColor4fv(c.data);
    glLines(3.0f);
    draw_line(x1, y1, x2, y2, c);
    draw_line(x2, y2, x2 - s*cosf(line_angle + M_PI/6), y2 - s*sinf(line_angle + M_PI/6), c);
    draw_line(x2, y2, x2 - s*cosf(line_angle - M_PI/6), y2 - s*sinf(line_angle - M_PI/6), c);
    glEnd();
}

void draw_principal_components(principal_components_t pca)
{
    auto pc = pca.eigen_vecs.vals;

    //normalizing factors
    float alpha_1 = sqrtf(pc[0][0]*pc[0][0] + pc[0][1]*pc[0][1]);
    float alpha_2 = sqrtf(pc[1][0]*pc[1][0] + pc[1][1]*pc[1][1]);

    //multiply vectors by 0.4 to get visually nice lengths
    draw_arrow(0,0, 0.4*pc[0][0] / alpha_1, 0.4*pc[0][1] / alpha_1, { 0.99f, 0.30f, 0.97f, 1.f });
    draw_arrow(0,0, 0.4*pc[1][0] / alpha_2, 0.4*pc[1][1] / alpha_2, { 0.30f, 0.99f, 0.79f, 1.f });
}

void plot_data_point(const std::vector<float>& data_point)
{
    float x = data_point[0], y = data_point[1];
    auto it = data_types.find(data_point);
    data_type_t type = it != data_types.end() ? it->second : PLAIN_DATA;

    color_t color;
    switch(type) {
        case KMEANS_CENTROID: 
            color = centroid_colors[data_point]; //{1.0f, 0.5f, 0.2f, 1.0f});
            break;
        case KMEANS_CLUSTER:
            color = cluster_data_colors[data_point]; 
            break;
        case SVM_POSITIVE_EXAMPLE:
            color = {0.2f, 0.8f, 0.2f, 1.0f};  // green
            break;
        case SVM_NEGATIVE_EXAMPLE:
            color = {0.8f, 0.2f, 0.2f, 1.0f};  // red
            break;
        case PLAIN_DATA:
        default:
            color = {0.3f, 0.7f, 0.3f, 1.0f};
    }
    if(type == KMEANS_CENTROID) {
        draw_x(x, y, X_RADIUS, color);
        return;
    }
    draw_point(x, y, RADIUS, color);
}

void plot_data(const matrix_t& data) 
{
    for (int i = 0; i < data.rows; ++i) {
        std::vector<float> data_point = data.vals[i];
        float x = data_point[0], y = data_point[1];
        plot_data_point(data_point);

        // Draw the highlight of the hovered data point
        if (vdbIsPointHovered(x, y)) {
            ImGui::SetTooltip("Hovered point\nx = %.2f\ny = %.2f", x, y);
            draw_point(x, y, HIGHLIGHT_RADIUS, {1.0f, 0.9f, 0.2f, 0.5f});
        }
    }
}

void draw_connected_components(image_t* image, const std::vector<int>& label, const std::vector<std::pair<int,int> >& points)
{
    for (int i = 0; i < points.size(); ++i) {
        std::pair<int,int> p = points[i];
        int x = p.first, y = p.second;
        int l = label[y*image->w + x];

        vdb_color c = vdbPalette(l);
        set_pixel(image, x, y, 0, c.r);
        set_pixel(image, x, y, 1, c.g);
        set_pixel(image, x, y, 2, c.b);
    }
}

bool colored_button(const char* text, float hue)
{
    ImGui::PushID(0);
    ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(hue, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(hue, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(hue, 0.8f, 0.8f));
    bool button_pressed = ImGui::Button(text);
    ImGui::PopStyleColor(3);
    ImGui::PopID();

    return button_pressed;
}

int main(int, char **) 
{
    // Controls
    // F10 : Step once
    // F5 : Step over
    // Ctrl+R : Show ruler
    // Ctrl+W : Set window size
    // Escape : Close window
    
    srand(time(0));

    VDBB("unsupervised ml memes");
    {
        const int rows = 1000, cols = 2;
        static matrix_t data = create_random_uniform_matrix(rows, cols);
        static matrix_t centroids = make_matrix(0, 0);
        static image_t image = make_image(100, 100, 3);
        static principal_components_t pcs { 0, 0 };

        vdb2D(-1, +1, -1, +1);

        // Draw grid
        glLines(5.0f);
        glColor4f(1,1,1,0.5f);
        vdbGridXY(-1, +1, -1, +1, 2);
        glEnd();

        plot_data(data);
        plot_data(centroids);

        TextWrapped("This shows an example of how k-means clustering works.");

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

        ImGui::SameLine();
        if (ImGui::Button("Random")) {
            data = create_random_uniform_matrix(rng0.Rand(rows), cols);
        }

        if (ImGui::CollapsingHeader("Covariance data")) {
            static float var[2] = {0.2f, 0.2f};
            ImGui::SliderFloat2("x- and y-variance", var, 0.f, 2.f);
            ImGui::SameLine(); ShowHelpMarker("CTRL+click to input value.");

            static float covar[2] = {1.0f, 1.0f};
            ImGui::SliderFloat2("xy- and yx-covariance", covar, -5.0f, 5.0f);
            ImGui::SameLine(); ShowHelpMarker("CTRL+click to input value.");

            if(colored_button("Generate covariance data", 5.f/7.f)) {
                matrix_t cov_mat = {2, 2, {{var[0], covar[0]}, {covar[1], var[1]}} };
                data = generate_covariance_data(cov_mat);
            }

            ImGui::SameLine();
            if(colored_button("PCA", 6.f / 7.f)) {
                pcs = pca(data);
            }
        }

        if(pcs.eigen_vals.size() > 0) {
            draw_principal_components(pcs);
        }

        if (ImGui::CollapsingHeader("Cluster data")) {
            static int num_clusters = 2;
            ImGui::SliderInt("number of clusters", &num_clusters, 0, 100);
            ImGui::SameLine(); ShowHelpMarker("CTRL+click to input value.");

            static float sigma = 0.1f;
            ImGui::SliderFloat("sigma", &sigma, 0.0f, 1.0f, "sigma = %.3f");
            ImGui::SameLine(); ShowHelpMarker("CTRL+click to input value.");

            if(colored_button("Generate cluster data", 4.f/7.f)) {
                matrix_t centers = create_random_uniform_matrix(num_clusters, 2);
                auto clusters = generate_clusters(centers, 50, sigma);
                clear_matrix(&data);
                data.cols = 2;
                for(const auto& cluster : clusters) data = concat_matrix(data, cluster);
            }
        }

        if (ImGui::CollapsingHeader("K-means")) {
            static int k = 2;
            ImGui::SliderInt("k", &k, 0, 7);
            ImGui::SameLine(); ShowHelpMarker("CTRL+click to input value.");

            static bool use_smart_centers = false;
            ImGui::Checkbox("K-means++", &use_smart_centers);

            // choose metrics
            static int metric = L2;
            ImGui::RadioButton("L1",  &metric, L1); ImGui::SameLine();
            ImGui::RadioButton("L2",  &metric, L2); ImGui::SameLine();
            ImGui::RadioButton("IOU", &metric, IOU);

            if(colored_button("Run K-means", 2.f/7.f)) {
                data_types.clear();
                centroid_colors.clear();
                cluster_data_colors.clear();
                if(metric == IOU) clear_image(&image);

                model_t model = kmeans(data, k, (kmeans_metric_t)metric, use_smart_centers);
                centroids = model.centers;

                auto colors = get_colors(k);
                for(int i = 0; i < centroids.vals.size(); ++i) {
                    color_t c = colors[i];
                    auto centroid = centroids.vals[i];

                    centroid_colors[centroid] = c;
                    data_types[centroid] = KMEANS_CENTROID;
                    if(metric == IOU) {
                        float width = centroid[0], height = centroid[1];
                        draw_box(&image, 
                                 image.w/2 - (image.w*width)/2, image.h/2 - (image.h*height)/2, 
                                 image.w/2 + (image.w*width)/2, image.h/2 + (image.h*height)/2,
                                 c.r, c.g, c.b);
                    }
                }

                for(int i = 0; i < model.assignments.size(); ++i) {
                    cluster_data_colors[data.vals[i]] = colors[model.assignments[i]];
                    data_types[data.vals[i]] = KMEANS_CLUSTER;
                }
            }

            if(metric == IOU) {
                ImGui::TextWrapped("Below is a visualization of the anchor boxes. Hover for a zoomed view!");
                ImVec2 tex_screen_pos = ImGui::GetCursorScreenPos();

                // Convert from CHW(channels separate) to HWC(channels interleaved)
                std::vector<unsigned char> byte_image = get_hwc_bytes(image);

                GLuint texture = vdbTexImage2D(byte_image.data(), image.w, image.h, GL_RGB);
                ImGui::Image((GLuint*)texture, ImVec2(image.w, image.h), ImVec2(0,0), ImVec2(1,1), ImColor(255,255,255,255), ImColor(255,255,255,128));
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    const float region_sz = 32.0f, zoom = 4.0f;
                    float region_x = ImGui::GetMousePos().x - tex_screen_pos.x - region_sz * 0.5f; if (region_x < 0.0f) region_x = 0.0f; else if (region_x > image.w - region_sz) region_x = image.w - region_sz;
                    float region_y = ImGui::GetMousePos().y - tex_screen_pos.y - region_sz * 0.5f; if (region_y < 0.0f) region_y = 0.0f; else if (region_y > image.h - region_sz) region_y = image.h - region_sz;
                    ImVec2 uv0 = ImVec2((region_x) / image.w, (region_y) / image.h);
                    ImVec2 uv1 = ImVec2((region_x + region_sz) / image.w, (region_y + region_sz) / image.h);
                    ImGui::Image((GLuint*)texture, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImColor(255,255,255,255), ImColor(255,255,255,128));
                    ImGui::EndTooltip();
                }
            }
        }
    }
    VDBE();
    data_types.clear();

    #include "gui_content/image_processing.cpp"

    #include "gui_content/svm_gui.cpp"

    #include "gui_content/regression_gui.cpp"

    return 0;
}
