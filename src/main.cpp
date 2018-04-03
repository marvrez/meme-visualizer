#include "vdb/vdb.h"

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

    VDBB("Hovering");
    {
        struct item_t { float x,y; };
        const int num_items = 5;
        static item_t items[num_items];

        srand(12345);
        for (int i = 0; i < num_items; i++)
        {
            items[i].x = -1.0f + 2.0f*(rand() % 10000)/10000.0f;
            items[i].y = -1.0f + 2.0f*(rand() % 10000)/10000.0f;
        }

        vdb2D(-1, +1, -1, +1);

        glLines(4.0f);
        glColor4f(1,1,1,0.5f);
        vdbGridXY(-1, +1, -1, +1, 2);
        glEnd();

        for (int i = 0; i < num_items; i++)
        {
            float x = items[i].x;
            float y = items[i].y;
            glColor4f(0.3f, 0.7f, 0.3f, 0.0f);

            glPoints(8.0f);
            glVertex2f(x, y);
            glEnd();

            glColor3f(0.3f, 0.7f, 0.3f);
            glLines(1.0f);
            draw_x(x,y,0.008);
            glEnd();

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

            if (i < num_items-1)
            {
                float x2 = items[i+1].x;
                float y2 = items[i+1].y;
                glLines(2.0f);
                glVertex2f(x, y); glVertex2f(x2, y2);
                glEnd();
            }
        }

        TextWrapped("This shows an example of how K-means clustering works.");
    }
    VDBE();

    return 0;
}
