/* ================================================================
 *  grid.h — Grid Data Structures and Declarations
 * ================================================================
 *  This header defines the core data structures for our pathfinding
 *  grid. Every cell in the grid is represented by a `Cell` struct,
 *  and the grid itself is a simple 2D array: Cell grid[ROWS][COLS].
 *
 *  DSA Concept: We use a 2D array for O(1) lookup by (row, col).
 *  This is far more efficient than a linked list for grid operations.
 * ================================================================ */

#ifndef GRID_H
#define GRID_H

#include <stdbool.h>

/* ----------------------------------------------------------------
 *  GRID CONSTANTS
 *  These define the dimensions of our pathfinding grid and how
 *  it is positioned on the screen.
 * ---------------------------------------------------------------- */
#define ROWS         30     /* Number of rows in the grid             */
#define COLS         40     /* Number of columns in the grid          */
#define CELL_SIZE    20     /* Size of each cell in pixels (square)   */
#define CELL_GAP     1      /* Gap between cells (forms grid lines)   */

/* Where the top-left corner of the grid is drawn on screen */
#define GRID_OFFSET_X  20
#define GRID_OFFSET_Y  80

/* ----------------------------------------------------------------
 *  CELL STATE ENUM
 *  Each cell can be in exactly ONE of these states at any time.
 *  The renderer uses this state to decide the cell's color.
 *
 *  States are ordered logically:
 *    - EMPTY/WALL/START/END are set by the user
 *    - FRONTIER/EXPLORED/PATH are set by the algorithm
 * ---------------------------------------------------------------- */
typedef enum {
    CELL_EMPTY,         /* Default — walkable, no special role       */
    CELL_WALL,          /* Obstacle — algorithm cannot pass through  */
    CELL_START,         /* The source node (only one allowed)        */
    CELL_END,           /* The destination node (only one allowed)   */
    CELL_FRONTIER,      /* In the "open set" — being considered      */
    CELL_EXPLORED,      /* In the "closed set" — already finalized   */
    CELL_PATH           /* Part of the final shortest path           */
} CellState;

/* ----------------------------------------------------------------
 *  CELL STRUCT
 *  Represents a single cell/node in the grid.
 *
 *  Fields:
 *    row, col    — Position in the grid (redundant but convenient)
 *    state       — Current visual/logical state (see enum above)
 *    g_cost      — Distance from start to this cell (Dijkstra/A*)
 *    f_cost      — Total estimated cost: g + h (A* only)
 *    parent_row  — Row of predecessor on shortest path (-1 = none)
 *    parent_col  — Col of predecessor on shortest path (-1 = none)
 *    visited     — Has the algorithm finalized this cell?
 *
 *  NOTE: g_cost, f_cost, parent, and visited are used in Steps 2-3.
 *        They are included now so the struct doesn't change later.
 * ---------------------------------------------------------------- */
typedef struct {
    int row;
    int col;
    CellState state;

    /* Pathfinding metadata (used in Steps 2 & 3) */
    float g_cost;       /* Cost from start node to this cell         */
    float f_cost;       /* f = g + h (heuristic, for A* only)        */
    int parent_row;     /* Which cell did we come from? (-1 = none)  */
    int parent_col;
    bool visited;       /* True once the cell is "finalized"         */
} Cell;

/* ----------------------------------------------------------------
 *  PLACEMENT MODE
 *  A simple state machine for user interaction:
 *    1. First, the user places the START node
 *    2. Then, the user places the END node
 *    3. Finally, the user can draw WALLS freely
 *
 *  Erasing a start/end node resets the mode appropriately.
 * ---------------------------------------------------------------- */
typedef enum {
    PLACE_START,        /* Waiting for user to place start node      */
    PLACE_END,          /* Waiting for user to place end node        */
    PLACE_WALL          /* User can now draw walls (or press SPACE)  */
} PlacementMode;

/* ----------------------------------------------------------------
 *  FUNCTION DECLARATIONS
 * ---------------------------------------------------------------- */

/*
 * grid_init — Initialize every cell in the grid.
 *   Sets all cells to CELL_EMPTY with default values.
 *   Call this once when the program starts.
 *
 * Parameters:
 *   grid — The 2D array of cells to initialize.
 */
void grid_init(Cell grid[ROWS][COLS]);

/*
 * grid_reset — Reset the entire grid to its initial state.
 *   Clears all cells, resets placement mode, and clears
 *   the stored start/end positions.
 *
 * Parameters:
 *   grid      — The grid to reset.
 *   mode      — Pointer to placement mode (reset to PLACE_START).
 *   start_row, start_col — Pointers to start position (reset to -1).
 *   end_row, end_col     — Pointers to end position (reset to -1).
 */
void grid_reset(Cell grid[ROWS][COLS], PlacementMode *mode,
                int *start_row, int *start_col,
                int *end_row, int *end_col);

/*
 * grid_get_cell_from_mouse — Convert screen coordinates to grid cell.
 *   Given mouse (x, y) in pixels, compute which grid cell (row, col)
 *   the mouse is hovering over.
 *
 * Parameters:
 *   mouse_x, mouse_y — Screen coordinates of the mouse cursor.
 *   out_row, out_col  — Output: the grid cell under the mouse.
 *
 * Returns:
 *   true  if the mouse is within the grid bounds.
 *   false if the mouse is outside the grid (outputs are undefined).
 */
bool grid_get_cell_from_mouse(int mouse_x, int mouse_y,
                               int *out_row, int *out_col);

/*
 * grid_handle_input — Process mouse clicks to place/erase cells.
 *   Left-click: places start → end → walls (in order).
 *   Right-click: erases the clicked cell.
 *   Supports click-and-drag for drawing/erasing walls.
 *
 * Parameters:
 *   grid      — The grid to modify.
 *   mode      — Pointer to current placement mode.
 *   start_row, start_col — Pointers to start node position.
 *   end_row, end_col     — Pointers to end node position.
 */
void grid_handle_input(Cell grid[ROWS][COLS], PlacementMode *mode,
                       int *start_row, int *start_col,
                       int *end_row, int *end_col);

/*
 * grid_clear_pathfinding — Clear only the algorithm's visual state.
 *   Resets FRONTIER, EXPLORED, and PATH cells back to EMPTY.
 *   Keeps walls, start, and end intact.
 *   Also resets all pathfinding metadata (g_cost, parent, etc.).
 *
 * Parameters:
 *   grid — The grid to clear.
 */
void grid_clear_pathfinding(Cell grid[ROWS][COLS]);

/*
 * grid_generate_maze — Generate a maze using Recursive Backtracker.
 *
 *   DSA Concept: Recursive Backtracker (DFS-based maze generation)
 *   ──────────────────────────────────────────────────────────────
 *   This is DFS APPLIED to maze generation. The algorithm:
 *     1. Start with a grid full of walls
 *     2. Pick a starting cell, mark it as passage
 *     3. Randomly choose an unvisited neighbor 2 cells away
 *     4. Carve a passage to it (remove the wall between)
 *     5. Recurse (repeat from the new cell)
 *     6. If no unvisited neighbors → backtrack (DFS!)
 *
 *   The result is a "perfect maze" — exactly ONE path between
 *   any two cells, with NO loops and NO isolated regions.
 *
 *   Why 2 cells at a time?
 *   ─────────────────────
 *   We step 2 cells to leave room for walls between passages.
 *   Odd-indexed cells are passages; even-indexed cells are walls
 *   (or vice versa). This creates the classic maze look.
 *
 * Parameters:
 *   grid — The grid to fill with a maze pattern.
 */
void grid_generate_maze(Cell grid[ROWS][COLS]);

#endif /* GRID_H */
