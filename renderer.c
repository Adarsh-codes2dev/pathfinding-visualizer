/* ================================================================
 *  renderer.c — Drawing / Rendering Functions
 * ================================================================
 *  Handles all visual output using Raylib's 2D drawing API.
 *
 *  Drawing order (painter's algorithm — back to front):
 *    1. Background (ClearBackground in main.c)
 *    2. Title bar
 *    3. Grid border
 *    4. Grid cells (colored by state)
 *    5. Hover highlight overlay
 *    6. Start/End node labels ("S" / "E")
 *    7. Sidebar panel (controls, legend, status)
 * ================================================================ */

#include "renderer.h"
#include <math.h>     /* For sinf() — pulse animation */
#include <stdio.h>    /* For snprintf() — formatting stats */

/* ----------------------------------------------------------------
 *  WINDOW DIMENSIONS (used for layout calculations)
 * ---------------------------------------------------------------- */
#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

/* Sidebar layout constants */
#define SIDEBAR_X       860   /* X position of the sidebar         */
#define SIDEBAR_Y       80    /* Y position (aligned with grid)    */
#define SIDEBAR_WIDTH   380   /* Width of the sidebar panel        */
#define SIDEBAR_PADDING 20    /* Inner padding                     */

/* ----------------------------------------------------------------
 *  HELPER: color_lerp — Linearly interpolate between two colors.
 *
 *  This is used for the pulse animation on start/end nodes.
 *  Formula: result = a + (b - a) * t,  where t is in [0, 1].
 * ---------------------------------------------------------------- */
static Color color_lerp(Color a, Color b, float t)
{
    return (Color){
        (unsigned char)(a.r + (int)(b.r - a.r) * t),
        (unsigned char)(a.g + (int)(b.g - a.g) * t),
        (unsigned char)(a.b + (int)(b.b - a.b) * t),
        255
    };
}

/* ----------------------------------------------------------------
 *  HELPER: get_cell_color — Map a CellState to its display color.
 * ---------------------------------------------------------------- */
static Color get_cell_color(CellState state)
{
    switch (state) {
        case CELL_EMPTY:    return COLOR_EMPTY;
        case CELL_WALL:     return COLOR_WALL;
        case CELL_START:    return COLOR_START;
        case CELL_END:      return COLOR_END;
        case CELL_FRONTIER: return COLOR_FRONTIER;
        case CELL_EXPLORED: return COLOR_EXPLORED;
        case CELL_PATH:     return COLOR_PATH;
        default:            return COLOR_EMPTY;
    }
}

/* ----------------------------------------------------------------
 *  draw_title — Render the title bar at the top of the window.
 * ---------------------------------------------------------------- */
void draw_title(void)
{
    /* Main title — centered */
    const char *title = "PATHFINDING VISUALIZER";
    int font_size = 28;
    int text_width = MeasureText(title, font_size);
    int x = (WINDOW_WIDTH - text_width) / 2;
    DrawText(title, x, 20, font_size, COLOR_TITLE);

    /* Subtitle — shows all 4 algorithms */
    const char *subtitle = "DSA Mini Project  |  BFS · DFS · Dijkstra · A*";
    int sub_size = 14;
    int sub_width = MeasureText(subtitle, sub_size);
    int sub_x = (WINDOW_WIDTH - sub_width) / 2;
    DrawText(subtitle, sub_x, 52, sub_size, COLOR_TEXT_DIM);

    /* Thin separator line */
    DrawLineEx((Vector2){20, 72}, (Vector2){WINDOW_WIDTH - 20, 72},
               1.0f, COLOR_GRID_BORDER);
}

/* ----------------------------------------------------------------
 *  draw_grid — Render all cells in the grid.
 * ---------------------------------------------------------------- */
void draw_grid(Cell grid[ROWS][COLS])
{
    /* --- Draw grid border --- */
    DrawRectangleLinesEx(
        (Rectangle){
            GRID_OFFSET_X - 2,
            GRID_OFFSET_Y - 2,
            COLS * CELL_SIZE + 4,
            ROWS * CELL_SIZE + 4
        },
        2.0f, COLOR_GRID_BORDER
    );

    /* --- Determine hover cell --- */
    int mouse_x = GetMouseX();
    int mouse_y = GetMouseY();
    int hover_row = -1, hover_col = -1;
    bool has_hover = grid_get_cell_from_mouse(mouse_x, mouse_y,
                                               &hover_row, &hover_col);

    /* --- Compute pulse factor for start/end animation --- */
    float pulse = (sinf((float)GetTime() * 4.0f) + 1.0f) / 2.0f;

    /* --- Draw each cell --- */
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            /* Screen position of this cell's top-left corner */
            int x = GRID_OFFSET_X + c * CELL_SIZE;
            int y = GRID_OFFSET_Y + r * CELL_SIZE;

            /* Base color from the cell's state */
            Color color = get_cell_color(grid[r][c].state);

            /* Pulse animation: start and end nodes gently glow */
            if (grid[r][c].state == CELL_START ||
                grid[r][c].state == CELL_END)
            {
                color = color_lerp(color, WHITE, pulse * 0.25f);
            }

            /* Draw the cell rectangle (with gap for grid lines) */
            DrawRectangle(x, y,
                          CELL_SIZE - CELL_GAP,
                          CELL_SIZE - CELL_GAP,
                          color);

            /* Draw "S" / "E" labels on start/end nodes */
            if (grid[r][c].state == CELL_START) {
                int fs = CELL_SIZE - 6;
                int tw = MeasureText("S", fs);
                DrawText("S",
                         x + (CELL_SIZE - CELL_GAP - tw) / 2,
                         y + (CELL_SIZE - CELL_GAP - fs) / 2 + 1,
                         fs, WHITE);
            }
            else if (grid[r][c].state == CELL_END) {
                int fs = CELL_SIZE - 6;
                int tw = MeasureText("E", fs);
                DrawText("E",
                         x + (CELL_SIZE - CELL_GAP - tw) / 2,
                         y + (CELL_SIZE - CELL_GAP - fs) / 2 + 1,
                         fs, WHITE);
            }

            /* Hover highlight: subtle white overlay */
            if (has_hover && r == hover_row && c == hover_col) {
                DrawRectangle(x, y,
                              CELL_SIZE - CELL_GAP,
                              CELL_SIZE - CELL_GAP,
                              (Color){255, 255, 255, 35});
            }
        }
    }
}

/* ----------------------------------------------------------------
 *  HELPER: draw_legend_item — Draw a colored square + label.
 * ---------------------------------------------------------------- */
static void draw_legend_item(int x, int y, Color color, const char *label)
{
    /* Colored square (12×12 pixels) */
    DrawRectangle(x, y, 12, 12, color);
    DrawRectangleLinesEx((Rectangle){x, y, 12, 12}, 1.0f,
                          (Color){255, 255, 255, 50});

    /* Label text to the right */
    DrawText(label, x + 20, y - 1, 14, COLOR_TEXT);
}

/* ----------------------------------------------------------------
 *  draw_sidebar — Render the right-side information panel.
 *
 *  Contains four sections:
 *    1. CONTROLS — Keyboard/mouse shortcuts
 *    2. LEGEND   — What each color means
 *    3. ALGORITHM — Current algorithm info & stats
 *    4. STATUS   — Current mode & instructions
 * ---------------------------------------------------------------- */
void draw_sidebar(PlacementMode mode,
                  int start_row, int start_col,
                  int end_row, int end_col,
                  const Pathfinder *pf)
{
    /* Suppress unused parameter warnings */
    (void)start_col;
    (void)end_col;

    int x = SIDEBAR_X + SIDEBAR_PADDING;
    int y = SIDEBAR_Y;

    /* --- Panel background (rounded rectangle) --- */
    DrawRectangleRounded(
        (Rectangle){SIDEBAR_X, SIDEBAR_Y - 10,
                     SIDEBAR_WIDTH, 620},
        0.03f, 8, COLOR_PANEL
    );

    /* ============================================================
     *  SECTION 1: CONTROLS
     * ============================================================ */
    DrawText("CONTROLS", x, y, 18, COLOR_HEADING);
    y += 28;
    DrawLineEx((Vector2){x, y}, (Vector2){x + 160, y},
               1.0f, COLOR_GRID_BORDER);
    y += 12;

    DrawText("LMB",   x,       y, 14, COLOR_HIGHLIGHT);
    DrawText("Place nodes / walls", x + 60, y, 14, COLOR_TEXT);
    y += 20;

    DrawText("RMB",   x,       y, 14, COLOR_HIGHLIGHT);
    DrawText("Erase cells",         x + 60, y, 14, COLOR_TEXT);
    y += 20;

    DrawText("R",     x,       y, 14, COLOR_HIGHLIGHT);
    DrawText("Reset grid",          x + 60, y, 14, COLOR_TEXT);
    y += 20;

    DrawText("C",     x,       y, 14, COLOR_HIGHLIGHT);
    DrawText("Clear path only",     x + 60, y, 14, COLOR_TEXT);
    y += 20;

    DrawText("M",     x,       y, 14, COLOR_HIGHLIGHT);
    DrawText("Generate maze",       x + 60, y, 14, COLOR_TEXT);
    y += 20;

    DrawText("SPACE", x,       y, 14, COLOR_HIGHLIGHT);
    DrawText("Run / Clear algo",    x + 60, y, 14, COLOR_TEXT);
    y += 20;

    DrawText("B/F",   x,       y, 14, COLOR_HIGHLIGHT);
    DrawText("BFS / DFS",           x + 60, y, 14, COLOR_TEXT);
    y += 20;

    DrawText("D/A",   x,       y, 14, COLOR_HIGHLIGHT);
    DrawText("Dijkstra / A*",       x + 60, y, 14, COLOR_TEXT);
    y += 20;

    DrawText("UP/DN", x,       y, 14, COLOR_HIGHLIGHT);
    DrawText("Adjust speed",        x + 60, y, 14, COLOR_TEXT);
    y += 30;

    /* ============================================================
     *  SECTION 2: LEGEND
     * ============================================================ */
    DrawText("LEGEND", x, y, 18, COLOR_HEADING);
    y += 28;
    DrawLineEx((Vector2){x, y}, (Vector2){x + 160, y},
               1.0f, COLOR_GRID_BORDER);
    y += 12;

    draw_legend_item(x, y, COLOR_EMPTY,    "Empty (walkable)");
    y += 18;
    draw_legend_item(x, y, COLOR_WALL,     "Wall (obstacle)");
    y += 18;
    draw_legend_item(x, y, COLOR_START,    "Start node");
    y += 18;
    draw_legend_item(x, y, COLOR_END,      "End node");
    y += 18;
    draw_legend_item(x, y, COLOR_FRONTIER, "Frontier (open set)");
    y += 18;
    draw_legend_item(x, y, COLOR_EXPLORED, "Explored (closed set)");
    y += 18;
    draw_legend_item(x, y, COLOR_PATH,     "Path found");
    y += 30;

    /* ============================================================
     *  SECTION 3: ALGORITHM INFO
     * ============================================================ */
    DrawText("ALGORITHM", x, y, 18, COLOR_HEADING);
    y += 28;
    DrawLineEx((Vector2){x, y}, (Vector2){x + 160, y},
               1.0f, COLOR_GRID_BORDER);
    y += 12;

    /* Algorithm name — changes based on key selection */
    const char *algo_name;
    const char *algo_note;
    switch (pf->algo_type) {
        case ALGO_TYPE_BFS:
            algo_name = "BFS";
            algo_note = "Queue — Shortest (unweighted)";
            break;
        case ALGO_TYPE_DFS:
            algo_name = "DFS";
            algo_note = "Stack — NOT shortest path!";
            break;
        case ALGO_TYPE_ASTAR:
            algo_name = "A* (A-Star)";
            algo_note = "PQ + Heuristic — Fastest";
            break;
        default:
            algo_name = "Dijkstra";
            algo_note = "PQ — Shortest (weighted)";
            break;
    }
    DrawText("Mode:", x, y, 14, COLOR_TEXT_DIM);
    DrawText(algo_name, x + 60, y, 14, COLOR_HIGHLIGHT);
    y += 18;
    DrawText(algo_note, x, y, 12, COLOR_TEXT_DIM);
    y += 20;

    /* Animation speed */
    char speed_buf[32];
    snprintf(speed_buf, sizeof(speed_buf), "%d nodes/frame",
             pf->steps_per_frame);
    DrawText("Speed:", x, y, 14, COLOR_TEXT_DIM);
    DrawText(speed_buf, x + 60, y, 14, COLOR_TEXT);
    y += 18;

    /* Nodes explored counter */
    char explored_buf[32];
    snprintf(explored_buf, sizeof(explored_buf), "%d",
             pf->nodes_explored);
    DrawText("Explored:", x, y, 14, COLOR_TEXT_DIM);
    DrawText(explored_buf, x + 80, y, 14, COLOR_EXPLORED);
    y += 18;

    /* Path length (only shown when a path is found) */
    if (pf->state == ALGO_FOUND) {
        char path_buf[32];
        snprintf(path_buf, sizeof(path_buf), "%d cells",
                 pf->path_length);
        DrawText("Path:", x, y, 14, COLOR_TEXT_DIM);
        DrawText(path_buf, x + 60, y, 14, COLOR_PATH);
    }
    y += 30;

    /* ============================================================
     *  SECTION 4: STATUS
     * ============================================================ */
    DrawText("STATUS", x, y, 18, COLOR_HEADING);
    y += 28;
    DrawLineEx((Vector2){x, y}, (Vector2){x + 160, y},
               1.0f, COLOR_GRID_BORDER);
    y += 12;

    /* Determine status text and color based on current state */
    const char *status_text;
    Color status_color;

    if (pf->state == ALGO_RUNNING) {
        status_text  = "Searching...";
        status_color = COLOR_FRONTIER;
    }
    else if (pf->state == ALGO_FOUND) {
        status_text  = "PATH FOUND!";
        status_color = COLOR_SUCCESS;
    }
    else if (pf->state == ALGO_NO_PATH) {
        status_text  = "NO PATH EXISTS";
        status_color = COLOR_ERROR;
    }
    else if (mode == PLACE_START) {
        status_text  = "Click to place START node";
        status_color = COLOR_START;
    }
    else if (mode == PLACE_END) {
        status_text  = "Click to place END node";
        status_color = COLOR_END;
    }
    else {
        if (start_row != -1 && end_row != -1) {
            status_text  = "Draw walls / M maze, SPACE run";
            status_color = COLOR_TEXT;
        } else {
            status_text  = "Draw walls on the grid";
            status_color = COLOR_TEXT;
        }
    }

    /* Blinking indicator dot */
    float blink = (sinf((float)GetTime() * 3.0f) + 1.0f) / 2.0f;
    Color dot_color = color_lerp(COLOR_BG, status_color, blink);
    DrawCircle(x + 6, y + 7, 5, dot_color);

    DrawText(status_text, x + 18, y, 14, status_color);
    y += 24;

    /* Contextual help text */
    if (pf->state == ALGO_FOUND || pf->state == ALGO_NO_PATH) {
        DrawText("SPACE to clear, R to reset", x, y, 12, COLOR_TEXT_DIM);
    }
    else if (pf->state == ALGO_RUNNING) {
        DrawText("Algorithm is running...", x, y, 12, COLOR_TEXT_DIM);
    }
    else {
        DrawText("Press R to reset the grid", x, y, 12, COLOR_TEXT_DIM);
    }
}
