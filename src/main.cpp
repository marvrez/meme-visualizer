#include "vdb/vdb.h"
#include "image.h"
#include "color_utils.h"
#include "matrix.h"
#include "data_gen.h"
#include "kmeans.h"
#include "rng.h"

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <map>

typedef enum {
    KMEANS_CLUSTER,
    KMEANS_CENTROID,
    PLAIN_DATA
} data_type_t;

static std::map<std::vector<float>, data_type_t> data_types;
static std::map<std::vector<float>, color_t> centroid_colors, cluster_data_colors;
constexpr float RADIUS = 4.0f, HIGHLIGHT_RADIUS = RADIUS * 2, X_RADIUS = 0.01f, LINE_THICKNESS = 2.5f;

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

void plot_data_point(const std::vector<float>& data_point)
{
    float x = data_point[0], y = data_point[1];
    auto it = data_types.find(data_point);
    data_type_t type = it != data_types.end() ? it->second : PLAIN_DATA;

    switch(type)
    {
        case KMEANS_CENTROID: 
            draw_x(x, y, X_RADIUS, centroid_colors[data_point]); //{1.0f, 0.5f, 0.2f, 1.0f});
            break;
        case KMEANS_CLUSTER:
            draw_point(x, y, RADIUS, cluster_data_colors[data_point]); 
            break;
        case PLAIN_DATA:
        default:
            draw_point(x, y, RADIUS, {0.3f, 0.7f, 0.3f, 1.0f});
    }
}

int main(int, char **) 
{
    // Controls
    // F10 : Step once
    // F5 : Step over
    // Ctrl+V : Record video
    // Ctrl+R : Show ruler
    // Ctrl+W : Set window size
    // Escape : Close window
    
    srand(time(0));

    const int rows = 1000, cols = 2;
    static matrix_t data = create_random_uniform_matrix(rows, cols);
    static matrix_t centroids = make_matrix(0, 0);
    static image_t image = make_image(100, 100, 3);

    VDBB("k-means clustering");
    {
        vdb2D(-1, +1, -1, +1);

        // Draw grid
        glLines(5.0f);
        glColor4f(1,1,1,0.5f);
        vdbGridXY(-1, +1, -1, +1, 2);
        glEnd();

        for (int i = 0; i < data.rows; i++) {
            std::vector<float> data_point = data.vals[i];
            float x = data_point[0], y = data_point[1];
            plot_data_point(data_point);

            // Draw the highlight of the hovered data point
            if (vdbIsPointHovered(x, y)) {
                SetTooltip("Hovered point\nx = %.2f\ny = %.2f", x, y);
                draw_point(x, y, HIGHLIGHT_RADIUS, {1.0f, 0.9f, 0.2f, 0.5f});
            }
        }

        for(int i = 0; i < centroids.rows; ++i) {
            std::vector<float> centroid = centroids.vals[i];
            float x = centroid[0], y = centroid[1];
            plot_data_point(centroid);

            // Draw the highlight of the hovered data point
            if (vdbIsPointHovered(x, y)) {
                SetTooltip("Hovered centroid \nx = %.2f\ny = %.2f", x, y);
                draw_point(x, y, HIGHLIGHT_RADIUS, {1.0f, 0.9f, 0.2f, 0.5f});
            }
        }

        TextWrapped("This shows an example of how k-means clustering works.");

        if (ImGui::Button("Load..")) ImGui::OpenPopup("Load data from CSV?");
        if (ImGui::BeginPopupModal("Load data from CSV?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char filename[1024];
            static bool init_filename = true;
            ImGui::InputText("Filename", filename, sizeof(filename));
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120,0))) 
            {
                data = csv_to_matrix(filename);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120,0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Random"))
        {
            data = create_random_uniform_matrix(rng0.Rand(rows), cols);
        }

        if (ImGui::CollapsingHeader("Covariance data"))
        {
            static float var[2] = {0.2f, 0.2f};
            ImGui::SliderFloat2("x- and y-variance", var, -0.5f, 0.5f);
            ImGui::SameLine(); ShowHelpMarker("CTRL+click to input value.");

            static float covar[2] = {1.0f, 1.0f};
            ImGui::SliderFloat2("xy- and yx-covariance", covar, -5.0f, 5.0f);
            ImGui::SameLine(); ShowHelpMarker("CTRL+click to input value.");

            if(ImGui::Button("Generate covariance data"))
            {
                matrix_t cov_mat = {2, 2, {{var[0], covar[0]}, {covar[1], var[1]}} };
                data = generate_covariance_data(cov_mat);
            }
        }

        if (ImGui::CollapsingHeader("Cluster data"))
        {
            static int num_clusters = 2;
            ImGui::SliderInt("number of clusters", &num_clusters, 0, 100);
            ImGui::SameLine(); ShowHelpMarker("CTRL+click to input value.");

            static float sigma = 0.1f;
            ImGui::SliderFloat("sigma", &sigma, 0.0f, 1.0f, "sigma = %.3f");
            ImGui::SameLine(); ShowHelpMarker("CTRL+click to input value.");

            if(ImGui::Button("Generate cluster data"))
            {
                matrix_t centers = create_random_uniform_matrix(num_clusters, 2);
                auto clusters = generate_clusters(centers, 50, sigma);
                clear_matrix(&data);
                data.cols = 2;
                for(const auto& cluster : clusters) data = concat_matrix(data, cluster);
            }
        }

        if (ImGui::CollapsingHeader("K-means"))
        {
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

            if(ImGui::Button("Run K-means"))
            {
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

            if(metric == IOU) 
            {
                ImGui::TextWrapped("Below is a visualization of the anchor boxes. Hover for a zoomed view!");
                ImVec2 tex_screen_pos = ImGui::GetCursorScreenPos();

                // Convert from CHW(channels separate) to HWC(channels interleaved)
                std::vector<unsigned char> byte_image(image.c*image.h*image.w, 0);
                for(int k = 0; k < image.c; ++k) {
                    for(int i = 0; i < image.w*image.h; ++i) {
                        byte_image[i*image.c+k] = (unsigned char) (255*image.data[i + k*image.w*image.h]);
                    }
                }

                GLuint texture = vdbTexImage2D(byte_image.data(), image.w, image.h, GL_RGB);
                ImGui::Image((GLuint*)texture, ImVec2(image.w, image.h), ImVec2(0,0), ImVec2(1,1), ImColor(255,255,255,255), ImColor(255,255,255,128));
                if (ImGui::IsItemHovered()) 
                {
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

    return 0;
}
