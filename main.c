/* ================================================================
 *  main.c — Pathfinding Visualizer Entry Point
 * ================================================================
 *  This is the entry point of our DSA mini-project.
 *
 *  Architecture Overview:
 *  ┌──────────┐     ┌──────────┐     ┌──────────────┐
 *  │  main.c  │────▶│  grid.c  │     │  renderer.c  │
 *  │  (loop)  │     │  (data)  │     │  (drawing)   │
 *  └──────────┘     └──────────┘     └──────────────┘
 *       │                │                   ▲
 *       │           ┌────────────┐           │
 *       └──────────▶│pathfinder.c│───────────┘
 *                   └────────────┘
 *
 *  The main loop follows Raylib's standard pattern:
 *    1. Handle input (keyboard + mouse)
 *    2. Update state (grid + algorithm step)
 *    3. Draw everything (grid, sidebar, effects)
 *
 *  This is called the "Game Loop" pattern — one of the most
 *  fundamental patterns in real-time interactive applications.
 *
 *  KEYBOARD CONTROLS:
 *  ═══════════════════
 *    B     — Select BFS algorithm
 *    F     — Select DFS algorithm
 *    D     — Select Dijkstra's algorithm
 *    A     — Select A* algorithm
 *    M     — Generate a random maze (recursive backtracker)
 *    SPACE — Start algorithm / Clear results
 *    R     — Full reset (grid + pathfinder)
 *    C     — Clear pathfinding overlay only (keep walls)
 *    UP/DN — Adjust animation speed
 * ================================================================ */

#include "raylib.h"
#include "grid.h"
#include "renderer.h"
#include "pathfinder.h"

/* Window configuration */
#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE  "Pathfinding Visualizer - DSA Mini Project"
#define TARGET_FPS    60

/* Speed limits for animation */
#define MIN_SPEED  1
#define MAX_SPEED  50

/* ================================================================
 *  main — Program entry point.
 *
 *  Responsibilities:
 *    1. Initialize the Raylib window and set the frame rate
 *    2. Initialize the grid and pathfinder
 *    3. Run the main loop (input → update → draw)
 *    4. Cleanup and close the window
 * ================================================================ */
int main(void)
{
    /* ============================================================
     *  INITIALIZATION
     * ============================================================ */

    /* Create the window — this must be called before any drawing */
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(TARGET_FPS);

    /* --- Grid Initialization ---
     * We declare the grid as a local 2D array on the stack.
     * Size: 30 rows × 40 cols × sizeof(Cell) ≈ 34 KB — very safe.
     */
    Cell grid[ROWS][COLS];
    grid_init(grid);

    /* --- Placement State ---
     * These variables track the interaction state machine and
     * the positions of the start/end nodes.
     * -1 means "not yet placed."
     */
    PlacementMode mode = PLACE_START;
    int start_row = -1, start_col = -1;
    int end_row   = -1, end_col   = -1;

    /* --- Pathfinder Initialization ---
     * The pathfinder holds the algorithm state (priority queue,
     * queue, stack, running status, animation speed, etc.)
     * between frames.
     */
    Pathfinder pf;
    pathfinder_init(&pf);
    pf.algo_type = ALGO_TYPE_BFS;  /* Default algorithm: BFS */

    /* ============================================================
     *  MAIN LOOP
     *  Runs at 60 FPS until the user closes the window (ESC or ✕).
     *
     *  Each iteration:
     *    Phase 1: INPUT  — Read keyboard and mouse events
     *    Phase 2: UPDATE — Run algorithm step (if active)
     *    Phase 3: DRAW   — Render everything to the screen
     * ============================================================ */
    while (!WindowShouldClose())
    {
        /* ---- PHASE 1: INPUT ---- */

        /* R key: Full reset — grid + pathfinder */
        if (IsKeyPressed(KEY_R)) {
            AlgoType saved_type = pf.algo_type;
            grid_reset(grid, &mode,
                       &start_row, &start_col,
                       &end_row, &end_col);
            pathfinder_init(&pf);
            pf.algo_type = saved_type;  /* Preserve algorithm selection */
        }

        /* C key: Clear pathfinding overlay only (keep walls) */
        if (IsKeyPressed(KEY_C)) {
            AlgoType saved_type = pf.algo_type;
            grid_clear_pathfinding(grid);
            pathfinder_init(&pf);
            pf.algo_type = saved_type;  /* Preserve algorithm selection */
        }

        /* M key: Generate a random maze */
        if (IsKeyPressed(KEY_M) && pf.state == ALGO_IDLE) {
            /* Reset everything first */
            AlgoType saved_type = pf.algo_type;
            grid_reset(grid, &mode,
                       &start_row, &start_col,
                       &end_row, &end_col);
            pathfinder_init(&pf);
            pf.algo_type = saved_type;

            /* Generate the maze */
            grid_generate_maze(grid);

            /* Auto-place start and end at maze corners
             * (row 1, col 1) and (ROWS-2, COLS-2) — guaranteed
             * to be passage cells in the recursive backtracker maze */
            start_row = 1;
            start_col = 1;
            grid[start_row][start_col].state = CELL_START;

            end_row = ROWS - 2;
            end_col = COLS - 2;
            grid[end_row][end_col].state = CELL_END;

            mode = PLACE_WALL;  /* Both placed, ready to run */
        }

        /* SPACE key: Start algorithm or clear results */
        if (IsKeyPressed(KEY_SPACE)) {
            if (pf.state == ALGO_IDLE) {
                /* Start the selected algorithm */
                if (start_row != -1 && end_row != -1) {
                    if (pf.algo_type == ALGO_TYPE_ASTAR) {
                        pathfinder_start_astar(&pf, grid,
                                               start_row, start_col,
                                               end_row, end_col);
                    } else if (pf.algo_type == ALGO_TYPE_BFS) {
                        pathfinder_start_bfs(&pf, grid,
                                             start_row, start_col);
                    } else if (pf.algo_type == ALGO_TYPE_DFS) {
                        pathfinder_start_dfs(&pf, grid,
                                             start_row, start_col);
                    } else {
                        pathfinder_start_dijkstra(&pf, grid,
                                                  start_row, start_col);
                    }
                }
            }
            else if (pf.state == ALGO_FOUND ||
                     pf.state == ALGO_NO_PATH)
            {
                /* Clear results and go back to drawing mode */
                AlgoType saved_type = pf.algo_type;
                grid_clear_pathfinding(grid);
                pathfinder_init(&pf);
                pf.algo_type = saved_type;
            }
        }

        /* Algorithm selection keys (only when idle) */
        if (pf.state == ALGO_IDLE) {
            if (IsKeyPressed(KEY_B)) pf.algo_type = ALGO_TYPE_BFS;
            if (IsKeyPressed(KEY_F)) pf.algo_type = ALGO_TYPE_DFS;
            if (IsKeyPressed(KEY_D)) pf.algo_type = ALGO_TYPE_DIJKSTRA;
            if (IsKeyPressed(KEY_A)) pf.algo_type = ALGO_TYPE_ASTAR;
        }

        /* UP/DOWN arrows: Adjust animation speed */
        if (IsKeyPressed(KEY_UP)) {
            pf.steps_per_frame += 2;
            if (pf.steps_per_frame > MAX_SPEED) {
                pf.steps_per_frame = MAX_SPEED;
            }
        }
        if (IsKeyPressed(KEY_DOWN)) {
            pf.steps_per_frame -= 2;
            if (pf.steps_per_frame < MIN_SPEED) {
                pf.steps_per_frame = MIN_SPEED;
            }
        }

        /* Mouse input: only allow grid editing when algorithm is idle */
        if (pf.state == ALGO_IDLE) {
            grid_handle_input(grid, &mode,
                              &start_row, &start_col,
                              &end_row, &end_col);
        }

        /* ---- PHASE 2: UPDATE (Algorithm Step) ---- */

        /* If the algorithm is running, process a batch of nodes.
         * This is what creates the animation — a few nodes per frame,
         * so the user can see the frontier expanding in real-time. */
        if (pf.state == ALGO_RUNNING) {
            pathfinder_step(&pf, grid, end_row, end_col);
        }

        /* ---- PHASE 3: DRAW ---- */

        BeginDrawing();

            /* Clear the screen with our dark background color */
            ClearBackground(COLOR_BG);

            /* Draw the title bar at the top */
            draw_title();

            /* Draw the grid with all cell colors and effects */
            draw_grid(grid);

            /* Draw the sidebar (controls, legend, status + algo info) */
            draw_sidebar(mode, start_row, start_col,
                         end_row, end_col, &pf);

        EndDrawing();
    }

    /* ============================================================
     *  CLEANUP
     *  Raylib requires CloseWindow() to release GPU resources.
     * ============================================================ */
    CloseWindow();

    return 0;
}
