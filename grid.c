/* ================================================================
 *  grid.c — Grid Logic and Input Handling
 * ================================================================
 *  Implements grid initialization, resetting, coordinate conversion,
 *  and all mouse-based interaction for placing/erasing cells.
 *
 *  Key DSA Concept: The grid is a 2D array (adjacency matrix-like
 *  structure). Each cell knows its neighbors implicitly —
 *  cell (r,c) has neighbors at (r±1, c) and (r, c±1).
 *  No explicit adjacency list is needed for a uniform grid.
 * ================================================================ */

#include "grid.h"
#include "raylib.h"
#include <math.h>     /* For INFINITY in pathfinding reset */
#include <stdlib.h>   /* For rand(), srand() in maze generation */
#include <time.h>     /* For time() to seed random number generator */

/* ----------------------------------------------------------------
 *  grid_init — Set every cell to a clean default state.
 *
 *  We iterate over all ROWS × COLS cells and initialize them.
 *  Time complexity: O(ROWS × COLS) = O(n) where n = total cells.
 * ---------------------------------------------------------------- */
void grid_init(Cell grid[ROWS][COLS])
{
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            grid[r][c].row        = r;
            grid[r][c].col        = c;
            grid[r][c].state      = CELL_EMPTY;

            /* Pathfinding defaults (used in Steps 2 & 3) */
            grid[r][c].g_cost     = INFINITY;  /* "Not yet reached" */
            grid[r][c].f_cost     = INFINITY;
            grid[r][c].parent_row = -1;        /* No parent yet     */
            grid[r][c].parent_col = -1;
            grid[r][c].visited    = false;
        }
    }
}

/* ----------------------------------------------------------------
 *  grid_reset — Full reset: clear grid + reset placement state.
 *
 *  This is called when the user presses 'R'. It restores everything
 *  to the initial state so the user can start fresh.
 * ---------------------------------------------------------------- */
void grid_reset(Cell grid[ROWS][COLS], PlacementMode *mode,
                int *start_row, int *start_col,
                int *end_row, int *end_col)
{
    /* Re-initialize every cell */
    grid_init(grid);

    /* Reset the placement state machine */
    *mode      = PLACE_START;
    *start_row = -1;
    *start_col = -1;
    *end_row   = -1;
    *end_col   = -1;
}

/* ----------------------------------------------------------------
 *  grid_get_cell_from_mouse — Screen coordinates → Grid cell.
 *
 *  Given the mouse position in pixels, we subtract the grid's
 *  offset and divide by CELL_SIZE to get the row and column.
 *
 *  Math:
 *    col = (mouse_x - GRID_OFFSET_X) / CELL_SIZE
 *    row = (mouse_y - GRID_OFFSET_Y) / CELL_SIZE
 *
 *  Returns false if the mouse is outside the grid area.
 * ---------------------------------------------------------------- */
bool grid_get_cell_from_mouse(int mouse_x, int mouse_y,
                               int *out_row, int *out_col)
{
    /* Check if mouse is within the grid's pixel bounds */
    if (mouse_x < GRID_OFFSET_X || mouse_y < GRID_OFFSET_Y) {
        return false;
    }

    /* Integer division gives us the cell index */
    int col = (mouse_x - GRID_OFFSET_X) / CELL_SIZE;
    int row = (mouse_y - GRID_OFFSET_Y) / CELL_SIZE;

    /* Check bounds */
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS) {
        return false;
    }

    *out_row = row;
    *out_col = col;
    return true;
}

/* ----------------------------------------------------------------
 *  grid_handle_input — Process mouse input for cell placement.
 *
 *  Interaction State Machine:
 *    1. If start not placed: left-click places START
 *    2. If end not placed:   left-click places END
 *    3. Otherwise:           left-click/drag draws WALLS
 *
 *  Right-click always erases the clicked cell:
 *    - If it was START → go back to PLACE_START mode
 *    - If it was END   → go back to PLACE_END mode
 *    - If it was WALL  → just set to EMPTY
 *
 *  Design Note: We use IsMouseButtonPressed() for start/end
 *  (single click) and IsMouseButtonDown() for walls (drag).
 * ---------------------------------------------------------------- */
void grid_handle_input(Cell grid[ROWS][COLS], PlacementMode *mode,
                       int *start_row, int *start_col,
                       int *end_row, int *end_col)
{
    int row, col;

    /* --- LEFT CLICK: Place nodes --- */
    if (*mode == PLACE_START || *mode == PLACE_END) {
        /* For start/end, respond only to a single click (not drag) */
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (grid_get_cell_from_mouse(GetMouseX(), GetMouseY(),
                                          &row, &col)) {
                /* Only place on empty cells */
                if (grid[row][col].state == CELL_EMPTY) {
                    if (*mode == PLACE_START) {
                        grid[row][col].state = CELL_START;
                        *start_row = row;
                        *start_col = col;
                        *mode = PLACE_END;  /* Advance state machine */
                    }
                    else if (*mode == PLACE_END) {
                        grid[row][col].state = CELL_END;
                        *end_row = row;
                        *end_col = col;
                        *mode = PLACE_WALL; /* Advance state machine */
                    }
                }
            }
        }
    }
    else if (*mode == PLACE_WALL) {
        /* For walls, respond to click-and-drag (held button) */
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (grid_get_cell_from_mouse(GetMouseX(), GetMouseY(),
                                          &row, &col)) {
                /* Only draw walls on empty cells */
                if (grid[row][col].state == CELL_EMPTY) {
                    grid[row][col].state = CELL_WALL;
                }
            }
        }
    }

    /* --- RIGHT CLICK: Erase cells --- */
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        if (grid_get_cell_from_mouse(GetMouseX(), GetMouseY(),
                                      &row, &col)) {
            CellState current = grid[row][col].state;

            if (current == CELL_START) {
                /* Erasing start → go back to PLACE_START mode */
                grid[row][col].state = CELL_EMPTY;
                *start_row = -1;
                *start_col = -1;
                *mode = PLACE_START;
            }
            else if (current == CELL_END) {
                /* Erasing end → go back to PLACE_END mode */
                grid[row][col].state = CELL_EMPTY;
                *end_row = -1;
                *end_col = -1;
                /* Only revert to PLACE_END if start is still placed */
                if (*start_row != -1) {
                    *mode = PLACE_END;
                } else {
                    *mode = PLACE_START;
                }
            }
            else if (current == CELL_WALL) {
                grid[row][col].state = CELL_EMPTY;
            }
        }
    }
}

/* ----------------------------------------------------------------
 *  grid_clear_pathfinding — Remove algorithm visuals only.
 *
 *  After running an algorithm, this clears the colored overlay
 *  (frontier, explored, path) without removing walls or start/end.
 *  Useful for running a different algorithm on the same maze.
 * ---------------------------------------------------------------- */
void grid_clear_pathfinding(Cell grid[ROWS][COLS])
{
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            /* Only clear algorithm-related states */
            if (grid[r][c].state == CELL_FRONTIER ||
                grid[r][c].state == CELL_EXPLORED ||
                grid[r][c].state == CELL_PATH)
            {
                grid[r][c].state = CELL_EMPTY;
            }

            /* Reset pathfinding metadata regardless */
            grid[r][c].g_cost     = INFINITY;
            grid[r][c].f_cost     = INFINITY;
            grid[r][c].parent_row = -1;
            grid[r][c].parent_col = -1;
            grid[r][c].visited    = false;
        }
    }
}

/* ================================================================
 *  MAZE GENERATION — Recursive Backtracker (Iterative DFS)
 * ================================================================
 *
 *  This generates a "perfect maze" using DFS on a grid where
 *  we step 2 cells at a time (so walls fit between passages).
 *
 *  Algorithm:
 *    1. Fill the entire grid with walls
 *    2. Pick a starting cell (odd row, odd col) → mark as passage
 *    3. Push it onto a stack
 *    4. While stack is not empty:
 *       a. Look at the top cell on the stack
 *       b. Find all unvisited neighbors 2 cells away
 *       c. If neighbors exist:
 *          - Pick one at random
 *          - Carve the wall between current and neighbor
 *          - Mark neighbor as passage
 *          - Push neighbor onto stack
 *       d. If no neighbors → pop (backtrack!)
 *
 *  WHY ITERATIVE instead of recursive? (Viva Answer)
 *  ──────────────────────────────────────────────────
 *  A 30×40 grid with step-2 has ~300 passage cells.
 *  Recursive DFS could go ~300 frames deep, which is fine on
 *  most systems, but using an explicit stack is:
 *    - More predictable (no stack overflow risk)
 *    - Same algorithmic behavior (DFS is DFS)
 *    - Easier to understand the call stack behavior
 *
 *  Fisher-Yates Shuffle:
 *  ─────────────────────
 *  To randomize direction order, we use the Fisher-Yates shuffle.
 *  This gives each permutation equal probability → unbiased maze.
 *
 *  Time:  O(ROWS × COLS) — visits every cell once
 *  Space: O(ROWS × COLS) — stack + visited array
 * ================================================================ */

/* Maze stack capacity: at most (ROWS/2) * (COLS/2) passage cells */
#define MAZE_STACK_SIZE ((ROWS / 2 + 1) * (COLS / 2 + 1))

/* Direction deltas for maze carving: step 2 cells at a time */
static const int MAZE_DR[4] = {-2,  2,  0,  0};
static const int MAZE_DC[4] = { 0,  0, -2,  2};

void grid_generate_maze(Cell grid[ROWS][COLS])
{
    /* Seed the random number generator with current time */
    srand((unsigned int)time(0));

    /* ---- Step 1: Fill entire grid with walls ---- */
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            grid[r][c].state      = CELL_WALL;
            grid[r][c].g_cost     = INFINITY;
            grid[r][c].f_cost     = INFINITY;
            grid[r][c].parent_row = -1;
            grid[r][c].parent_col = -1;
            grid[r][c].visited    = false;
        }
    }

    /* ---- Step 2: Create a visited array for maze generation ---- */
    /* We reuse the Cell's visited flag for this purpose */

    /* ---- Step 3: Pick a starting cell (must be on odd indices) ---- */
    int start_r = 1;
    int start_c = 1;
    grid[start_r][start_c].state   = CELL_EMPTY;
    grid[start_r][start_c].visited = true;

    /* ---- Step 4: Iterative DFS using explicit stack ---- */
    /* Simple stack of (row, col) pairs */
    int stack_rows[MAZE_STACK_SIZE];
    int stack_cols[MAZE_STACK_SIZE];
    int stack_top = 0;

    stack_rows[0] = start_r;
    stack_cols[0] = start_c;

    while (stack_top >= 0) {
        int r = stack_rows[stack_top];
        int c = stack_cols[stack_top];

        /* ---- Collect unvisited neighbors 2 cells away ---- */
        int neighbors[4];    /* Stores direction indices */
        int num_neighbors = 0;

        for (int d = 0; d < 4; d++) {
            int nr = r + MAZE_DR[d];
            int nc = c + MAZE_DC[d];

            /* Check bounds (must be within grid) */
            if (nr >= 1 && nr < ROWS - 1 &&
                nc >= 1 && nc < COLS - 1)
            {
                /* Check if this cell hasn't been carved yet */
                if (!grid[nr][nc].visited) {
                    neighbors[num_neighbors++] = d;
                }
            }
        }

        if (num_neighbors > 0) {
            /* ---- Fisher-Yates: pick a random neighbor ---- */
            int pick = rand() % num_neighbors;
            int dir = neighbors[pick];

            int nr = r + MAZE_DR[dir];
            int nc = c + MAZE_DC[dir];

            /* Carve the wall BETWEEN current and neighbor */
            int wall_r = r + MAZE_DR[dir] / 2;
            int wall_c = c + MAZE_DC[dir] / 2;
            grid[wall_r][wall_c].state = CELL_EMPTY;

            /* Carve the neighbor cell itself */
            grid[nr][nc].state   = CELL_EMPTY;
            grid[nr][nc].visited = true;

            /* Push neighbor onto the stack */
            stack_top++;
            stack_rows[stack_top] = nr;
            stack_cols[stack_top] = nc;
        }
        else {
            /* ---- Backtrack! (No unvisited neighbors) ---- */
            /* This IS the DFS backtracking step. When we pop,
             * we go back to the previous cell and try its other
             * unvisited neighbors. This creates the winding
             * corridors characteristic of DFS mazes.          */
            stack_top--;
        }
    }

    /* Reset visited flags so pathfinding can use them later */
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            grid[r][c].visited = false;
        }
    }
}

