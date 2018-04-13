#include "vdb/vdb.h"

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "matrix.h"
#include "cluster_generator.h"

typedef struct {
    float r, g, b;
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

    const int rows = 2000, cols = 2;
    static matrix_t data = create_random_matrix(rows, cols);

    VDBB("k-means clustering");
    {
        vdb2D(-1, +1, -1, +1);

        glLines(5.0f);
        glColor4f(1,1,1,0.5f);
        vdbGridXY(-1, +1, -1, +1, 2);
        glEnd();

        for (int i = 0; i < data.rows; i++)
        {
            float x = data.vals[i][0];
            float y = data.vals[i][1];
            glColor4f(0.3f, 0.7f, 0.3f, 1.0f);

            glPoints(8.0f);
            glVertex2f(x, y);
            glEnd();

            glColor3f(0.3f, 0.7f, 0.3f);
            if (vdbIsPointHovered(x, y))
                SetTooltip("Hovered point\nx = %.2f\ny = %.2f", x, y);
        }

        {
            float x,y;
            int i = vdbGetHoveredPoint(&x, &y);
            glPoints(16.0f);
            glColor4f(1.0f, 0.9f, 0.2f, 0.5f);
            glVertex2f(x, y);
            glEnd();

            if (i < data.rows-1)
            {
                float x2 = data.vals[i+1][0];
                float y2 = data.vals[i+1][1];
                glLines(2.0f);
                glVertex2f(x, y); glVertex2f(x2, y2);
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
        data = create_random_matrix(rows, cols);
    }

    if(ImGui::Button("Cluster"))
    {
        std::vector<std::vector<float> > centroids;
    }

    }
    VDBE();

    return 0;
}
