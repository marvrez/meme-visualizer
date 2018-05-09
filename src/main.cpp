#include "vdb/vdb.h"
#include "matrix.h"
#include "data_gen.h"
#include "kmeans.h"

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cmath>

typedef struct {
    float r, g, b, a;
} color;

void draw_line(float x1, float y1, float x2, float y2) 
{
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
}

void draw_x(float x, float y, float radius) 
{
    draw_line(x-radius, y-radius, x+radius, y+radius);
    draw_line(x+radius, y-radius, x-radius, y+radius);
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

    VDBB("k-means clustering");
    {
        vdb2D(-1, +1, -1, +1);

        // Draw grid
        glLines(5.0f);
        glColor4f(1,1,1,0.5f);
        vdbGridXY(-1, +1, -1, +1, 2);
        glEnd();

        for (int i = 0; i < data.rows; i++)
        {
            float x = data.vals[i][0];
            float y = data.vals[i][1];

            // Draw the data points
            glColor4f(0.3f, 0.7f, 0.3f, 1.0f);
            glPoints(4.0f);
            glVertex2f(x, y);
            glEnd();

            // Draw the highlight of the hovered data point
            glColor3f(0.3f, 0.7f, 0.3f);
            if (vdbIsPointHovered(x, y)) 
            {
                SetTooltip("Hovered point\nx = %.2f\ny = %.2f", x, y);
                glPoints(8.0f);
                glColor4f(1.0f, 0.9f, 0.2f, 0.5f);
                glVertex2f(x, y);
                glEnd();
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
                matrix_t centroids = create_random_uniform_matrix(num_clusters, 2);
                auto clusters = generate_clusters(centroids, 50, sigma);
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

            if(ImGui::Button("Run K-means"))
            {
                matrix_t centers = make_matrix(k, 2);
                random_centers(data, &centers);
                print_matrix(centers);
                data.rows = k;
                data = centers;
            }
        }
    }
    VDBE();

    return 0;
}
