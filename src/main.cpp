#include "vdb/vdb.h"
#include "matrix.h"
#include "data_gen.h"
#include "kmeans.h"
#include "rng.h"

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <map>

typedef struct {
    union {
        float data[4];
        struct { float r, g, b, a; };
    };
} color_t;

typedef enum {
    KMEANS_CLUSTER,
    KMEANS_CENTROID,
    PLAIN_DATA
} data_type_t;

static std::map<std::vector<float>, data_type_t> data_types;
static std::map<std::vector<float>, color_t> centroid_colors, cluster_data_colors;
constexpr float RADIUS = 4.0f, HIGHLIGHT_RADIUS = RADIUS * 2, X_RADIUS = 0.01f, LINE_THICKNESS = 2.5f;

color_t hsv_to_rgb(float h, float s, float v)
{
    float r, g, b, a = 1.0f;
    float f, p, q, t;
    if (s == 0) {
        r = g = b = v;
    } 
    else {
        int index = floor(h * 6);
        f = h * 6 - index;
        p = v*(1-s);
        q = v*(1-s*f);
        t = v*(1-s*(1-f));
        switch (index)
        {
            case 0:
                r = v; g = t; b = p;
                break;
            case 1:
                r = q; g = v; b = p;
                break;
            case 2:
                r = p; g = v; b = t;
                break;
            case 3:
                r = p; g = q; b = v;
                break;
            case 4:
                r = t; g = p; b = v;
                break;
            default:
                r = v; g = p; b = q;
                break;
        }
    }
    return {r, g, b, a};
}

// https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
std::vector<color_t> get_colors(const int n) 
{
    std::vector<color_t> colors;
    const float golden_ratio_conjugate = 0.618033988749895f;
    const float s = 0.7f, v = 0.99f;
    for (int i = 0; i < n; ++i) {
        const float h = std::fmod(rng0.getFloat() + golden_ratio_conjugate,
                                  1.0f);
        colors.push_back(hsv_to_rgb(h, s, v));
    }
    return colors;
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

    const int rows = 1000, cols = 2;
    static matrix_t data = create_random_uniform_matrix(rows, cols);
    static matrix_t centroids = make_matrix(0,0);

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

                model_t model = kmeans(data, k, (kmeans_metric_t)metric, use_smart_centers);
                centroids = model.centers;

                auto colors = get_colors(k);
                for(int i = 0; i < centroids.vals.size(); ++i) {
                    centroid_colors[centroids.vals[i]] = colors[i];
                    data_types[centroids.vals[i]] = KMEANS_CENTROID;
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
                float tex_w = (float)ImGui::GetIO().Fonts->TexWidth;
                float tex_h = (float)ImGui::GetIO().Fonts->TexHeight;
                ImTextureID tex_id = ImGui::GetIO().Fonts->TexID;
                ImGui::Text("%.0fx%.0f", tex_w, tex_h);
                ImGui::Image(tex_id, ImVec2(tex_w, tex_h), ImVec2(0,0), ImVec2(1,1), ImColor(255,255,255,255), ImColor(255,255,255,128));
                if (ImGui::IsItemHovered()) 
                {
                    ImGui::BeginTooltip();
                    float focus_sz = 32.0f;
                    float focus_x = ImGui::GetMousePos().x - tex_screen_pos.x - focus_sz * 0.5f; if (focus_x < 0.0f) focus_x = 0.0f; else if (focus_x > tex_w - focus_sz) focus_x = tex_w - focus_sz;
                    float focus_y = ImGui::GetMousePos().y - tex_screen_pos.y - focus_sz * 0.5f; if (focus_y < 0.0f) focus_y = 0.0f; else if (focus_y > tex_h - focus_sz) focus_y = tex_h - focus_sz;
                    ImGui::Text("Min: (%.2f, %.2f)", focus_x, focus_y);
                    ImGui::Text("Max: (%.2f, %.2f)", focus_x + focus_sz, focus_y + focus_sz);
                    ImVec2 uv0 = ImVec2((focus_x) / tex_w, (focus_y) / tex_h);
                    ImVec2 uv1 = ImVec2((focus_x + focus_sz) / tex_w, (focus_y + focus_sz) / tex_h);
                    ImGui::Image(tex_id, ImVec2(128,128), uv0, uv1, ImColor(255,255,255,255), ImColor(255,255,255,128));
                    ImGui::EndTooltip();
                }
            }
        }
    }
    VDBE();

    return 0;
}
