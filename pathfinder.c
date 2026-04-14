/* ================================================================
 *  pathfinder.c — BFS, DFS, Dijkstra, & A* Implementations
 * ================================================================
 *
 *  This file implements FOUR pathfinding algorithms:
 *
 *  ┌─────────────┬────────────────┬────────────┬───────────────────┐
 *  │ Algorithm   │ Data Structure │ Shortest?  │ Key Idea          │
 *  ├─────────────┼────────────────┼────────────┼───────────────────┤
 *  │ BFS         │ Queue (FIFO)   │ YES (unwt) │ Level-by-level    │
 *  │ DFS         │ Stack (LIFO)   │ NO         │ Deep first        │
 *  │ Dijkstra    │ Min-Heap       │ YES (wgt)  │ Nearest first     │
 *  │ A*          │ Min-Heap       │ YES (wgt)  │ Guided by h(n)    │
 *  └─────────────┴────────────────┴────────────┴───────────────────┘
 *
 *  ─────────────────────────────────────────────────────────────────
 *  BFS (Breadth-First Search):
 *    Uses a QUEUE. Processes nodes in FIFO order.
 *    Explores all nodes at distance d before distance d+1.
 *    Guaranteed shortest path on UNWEIGHTED graphs.
 *
 *  DFS (Depth-First Search):
 *    Uses a STACK. Processes nodes in LIFO order.
 *    Goes as deep as possible before backtracking.
 *    Does NOT guarantee shortest path — but uses less memory
 *    and is the basis for maze generation.
 *
 *  WHY BFS finds shortest path (Viva Answer):
 *  ──────────────────────────────────────────
 *  BFS explores in "waves" from the source. Wave 1 covers all
 *  nodes at distance 1, wave 2 covers distance 2, etc.
 *  The FIRST time BFS reaches any node, it's via the shortest
 *  path — because all shorter paths were explored in earlier waves.
 *
 *    Wave 1:  . . . .       Wave 3:  . 3 . .
 *             . . 1 .                3 2 1 .
 *             . . S .                3 2 S .
 *             . . . .                . 3 . .
 *
 *  WHY DFS does NOT find shortest path:
 *  ────────────────────────────────────
 *  DFS follows ONE path deeply before exploring alternatives.
 *  It might reach a node via a LONG detour before discovering
 *  the short path. DFS is useful for other purposes:
 *    - Maze generation (recursive backtracker)
 *    - Cycle detection
 *    - Topological sort
 *    - Finding connected components
 *
 *  Dijkstra & A* remain unchanged from the original implementation.
 *  See the detailed comments below for those.
 * ================================================================ */

#include "pathfinder.h"
#include <math.h>     /* For INFINITY, abs() */
#include <stdlib.h>   /* For abs() */

/* ----------------------------------------------------------------
 *  DIRECTION ARRAYS
 *  Define 4-directional movement: Up, Down, Left, Right.
 * ---------------------------------------------------------------- */
static const int DR[4] = {-1,  1,  0,  0};
static const int DC[4] = { 0,  0, -1,  1};

/* ----------------------------------------------------------------
 *  manhattan_distance — The A* heuristic function.
 *
 *  h(n) = |n.row - goal.row| + |n.col - goal.col|
 *
 *  Admissible (never overestimates) for 4-directional movement.
 *  Guarantees A* finds the optimal path.
 * ---------------------------------------------------------------- */
static float manhattan_distance(int row, int col,
                                 int goal_row, int goal_col)
{
    return (float)(abs(row - goal_row) + abs(col - goal_col));
}

/* ----------------------------------------------------------------
 *  reconstruct_path — Trace back from end to start via parents.
 *
 *  After any algorithm finishes, cells on the path have parent
 *  pointers. We follow them backwards: END → ... → START.
 *  Each cell is marked as CELL_PATH (gold).
 *  Returns: the number of cells in the path.
 * ---------------------------------------------------------------- */
static int reconstruct_path(Cell grid[ROWS][COLS],
                             int end_row, int end_col)
{
    int length = 0;
    int r = end_row;
    int c = end_col;

    while (r != -1 && c != -1) {
        if (grid[r][c].state != CELL_START &&
            grid[r][c].state != CELL_END)
        {
            grid[r][c].state = CELL_PATH;
        }
        length++;

        int pr = grid[r][c].parent_row;
        int pc = grid[r][c].parent_col;
        r = pr;
        c = pc;
    }

    return length;
}

/* ----------------------------------------------------------------
 *  reset_grid_for_pathfinding — Clear overlays + reset metadata.
 *
 *  Shared by all four start functions.
 *  Clears FRONTIER/EXPLORED/PATH colors and resets all costs.
 * ---------------------------------------------------------------- */
static void reset_grid_for_pathfinding(Cell grid[ROWS][COLS])
{
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (grid[r][c].state == CELL_FRONTIER ||
                grid[r][c].state == CELL_EXPLORED ||
                grid[r][c].state == CELL_PATH)
            {
                grid[r][c].state = CELL_EMPTY;
            }
            grid[r][c].g_cost     = INFINITY;
            grid[r][c].f_cost     = INFINITY;
            grid[r][c].parent_row = -1;
            grid[r][c].parent_col = -1;
            grid[r][c].visited    = false;
        }
    }
}

/* ================================================================
 *  PUBLIC FUNCTIONS
 * ================================================================ */

/* ----------------------------------------------------------------
 *  pathfinder_init — Set the pathfinder to a clean idle state.
 * ---------------------------------------------------------------- */
void pathfinder_init(Pathfinder *pf)
{
    pq_init(&pf->pq);
    queue_init(&pf->queue);
    stack_init(&pf->stack);
    pf->state           = ALGO_IDLE;
    pf->steps_per_frame = 5;
    pf->nodes_explored  = 0;
    pf->path_length     = 0;
}

/* ================================================================
 *  BFS — Breadth-First Search
 * ================================================================
 *
 *  BFS Algorithm (pseudocode):
 *    1. Enqueue the start node
 *    2. While queue is not empty:
 *       a. Dequeue front node (FIFO)
 *       b. If it's the goal → reconstruct path, done!
 *       c. For each unvisited neighbor:
 *          - Mark as visited
 *          - Set parent pointer
 *          - Enqueue it
 *
 *  BFS does NOT need g_cost or priority — it naturally explores
 *  in order of distance because of the FIFO property.
 *
 *  Time:  O(V + E) = O(ROWS × COLS) for a grid
 *  Space: O(V) for the queue and visited array
 * ================================================================ */
void pathfinder_start_bfs(Pathfinder *pf, Cell grid[ROWS][COLS],
                           int start_row, int start_col)
{
    reset_grid_for_pathfinding(grid);

    /* Mark start as visited and enqueue it */
    grid[start_row][start_col].visited = true;

    queue_init(&pf->queue);
    queue_push(&pf->queue, start_row, start_col);

    pf->algo_type      = ALGO_TYPE_BFS;
    pf->state          = ALGO_RUNNING;
    pf->nodes_explored = 0;
    pf->path_length    = 0;
}

/* ================================================================
 *  DFS — Depth-First Search
 * ================================================================
 *
 *  DFS Algorithm (pseudocode):
 *    1. Push the start node onto the stack
 *    2. While stack is not empty:
 *       a. Pop top node (LIFO — most recent first)
 *       b. If already visited, skip
 *       c. Mark as visited
 *       d. If it's the goal → reconstruct path, done!
 *       e. For each unvisited neighbor:
 *          - Set parent pointer
 *          - Push onto stack
 *
 *  DFS explores IN DEPTH — it follows one path as far as possible
 *  before backtracking. The LIFO stack causes this behavior:
 *  the most recently discovered node is explored next.
 *
 *  Time:  O(V + E) = O(ROWS × COLS) for a grid
 *  Space: O(V) for the stack
 * ================================================================ */
void pathfinder_start_dfs(Pathfinder *pf, Cell grid[ROWS][COLS],
                           int start_row, int start_col)
{
    reset_grid_for_pathfinding(grid);

    stack_init(&pf->stack);
    stack_push(&pf->stack, start_row, start_col);

    pf->algo_type      = ALGO_TYPE_DFS;
    pf->state          = ALGO_RUNNING;
    pf->nodes_explored = 0;
    pf->path_length    = 0;
}

/* ----------------------------------------------------------------
 *  pathfinder_start_dijkstra — Begin Dijkstra's algorithm.
 *  Dijkstra uses priority = g(n) only (no heuristic).
 * ---------------------------------------------------------------- */
void pathfinder_start_dijkstra(Pathfinder *pf, Cell grid[ROWS][COLS],
                                int start_row, int start_col)
{
    reset_grid_for_pathfinding(grid);

    grid[start_row][start_col].g_cost = 0.0f;

    pq_init(&pf->pq);
    pq_push(&pf->pq, start_row, start_col, 0.0f);

    pf->algo_type      = ALGO_TYPE_DIJKSTRA;
    pf->state          = ALGO_RUNNING;
    pf->nodes_explored = 0;
    pf->path_length    = 0;
}

/* ----------------------------------------------------------------
 *  pathfinder_start_astar — Begin A* algorithm.
 *  A* uses priority = f(n) = g(n) + h(n).
 * ---------------------------------------------------------------- */
void pathfinder_start_astar(Pathfinder *pf, Cell grid[ROWS][COLS],
                            int start_row, int start_col,
                            int end_row, int end_col)
{
    reset_grid_for_pathfinding(grid);

    grid[start_row][start_col].g_cost = 0.0f;
    float h = manhattan_distance(start_row, start_col, end_row, end_col);
    grid[start_row][start_col].f_cost = h;

    pq_init(&pf->pq);
    pq_push(&pf->pq, start_row, start_col, h);

    pf->algo_type      = ALGO_TYPE_ASTAR;
    pf->state          = ALGO_RUNNING;
    pf->nodes_explored = 0;
    pf->path_length    = 0;
}

/* ================================================================
 *  pathfinder_step — Process one batch of nodes per frame.
 *
 *  This is the HEART of the visualizer. It dispatches to the
 *  correct algorithm based on pf->algo_type.
 *
 *  Frame-by-frame execution creates the animation:
 *    Frame 1: Process N nodes → draw → user sees frontier growing
 *    Frame 2: Process N more  → draw → exploration continues
 *    Frame N: End node found  → reconstruct path → gold!
 * ================================================================ */
void pathfinder_step(Pathfinder *pf, Cell grid[ROWS][COLS],
                     int end_row, int end_col)
{
    if (pf->state != ALGO_RUNNING) {
        return;
    }

    for (int step = 0; step < pf->steps_per_frame; step++) {

        /* ============================================================
         *  BFS STEP — Queue-based exploration
         *
         *  BFS marks nodes as visited when they're ENQUEUED (not when
         *  dequeued). This prevents duplicate entries and is what
         *  guarantees shortest-path discovery.
         * ============================================================ */
        if (pf->algo_type == ALGO_TYPE_BFS)
        {
            if (queue_is_empty(&pf->queue)) {
                pf->state = ALGO_NO_PATH;
                return;
            }

            QSNode current = queue_pop(&pf->queue);
            int r = current.row;
            int c = current.col;

            pf->nodes_explored++;

            /* Mark as explored (visually) */
            if (grid[r][c].state != CELL_START &&
                grid[r][c].state != CELL_END)
            {
                grid[r][c].state = CELL_EXPLORED;
            }

            /* Check if we've reached the goal */
            if (r == end_row && c == end_col) {
                pf->path_length = reconstruct_path(grid, end_row, end_col);
                pf->state = ALGO_FOUND;
                return;
            }

            /* Explore all 4 neighbors */
            for (int d = 0; d < 4; d++) {
                int nr = r + DR[d];
                int nc = c + DC[d];

                /* Bounds check */
                if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
                /* Wall check */
                if (grid[nr][nc].state == CELL_WALL) continue;
                /* Already visited check — BFS marks on enqueue! */
                if (grid[nr][nc].visited) continue;

                /* Mark as visited NOW (prevents duplicates in queue) */
                grid[nr][nc].visited    = true;
                grid[nr][nc].parent_row = r;
                grid[nr][nc].parent_col = c;

                /* Visual: mark as frontier */
                if (grid[nr][nc].state != CELL_END) {
                    grid[nr][nc].state = CELL_FRONTIER;
                }

                queue_push(&pf->queue, nr, nc);
            }
        }

        /* ============================================================
         *  DFS STEP — Stack-based exploration
         *
         *  DFS marks nodes as visited when they're POPPED (not when
         *  pushed). This is the classic iterative DFS approach.
         *  Multiple entries of the same node may exist on the stack,
         *  but we skip already-visited ones.
         * ============================================================ */
        else if (pf->algo_type == ALGO_TYPE_DFS)
        {
            if (stack_is_empty(&pf->stack)) {
                pf->state = ALGO_NO_PATH;
                return;
            }

            QSNode current = stack_pop(&pf->stack);
            int r = current.row;
            int c = current.col;

            /* Skip if already visited (DFS may have duplicates) */
            if (grid[r][c].visited) {
                step--;  /* Don't count this as a real step */
                continue;
            }

            /* Mark as visited */
            grid[r][c].visited = true;
            pf->nodes_explored++;

            /* Mark as explored (visually) */
            if (grid[r][c].state != CELL_START &&
                grid[r][c].state != CELL_END)
            {
                grid[r][c].state = CELL_EXPLORED;
            }

            /* Check if we've reached the goal */
            if (r == end_row && c == end_col) {
                pf->path_length = reconstruct_path(grid, end_row, end_col);
                pf->state = ALGO_FOUND;
                return;
            }

            /* Push all 4 neighbors onto the stack */
            for (int d = 0; d < 4; d++) {
                int nr = r + DR[d];
                int nc = c + DC[d];

                if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
                if (grid[nr][nc].state == CELL_WALL) continue;
                if (grid[nr][nc].visited) continue;

                /* Set parent ONLY if not yet set (first discovery) */
                if (grid[nr][nc].parent_row == -1) {
                    grid[nr][nc].parent_row = r;
                    grid[nr][nc].parent_col = c;
                }

                /* Visual: mark as frontier */
                if (grid[nr][nc].state != CELL_END &&
                    grid[nr][nc].state != CELL_FRONTIER)
                {
                    grid[nr][nc].state = CELL_FRONTIER;
                }

                stack_push(&pf->stack, nr, nc);
            }
        }

        /* ============================================================
         *  DIJKSTRA / A* STEP — Priority-queue-based exploration
         *
         *  These two algorithms share the same code path.
         *  The only difference is the priority calculation:
         *    Dijkstra: priority = g(n)
         *    A*:       priority = g(n) + h(n)
         * ============================================================ */
        else /* ALGO_TYPE_DIJKSTRA or ALGO_TYPE_ASTAR */
        {
            if (pq_is_empty(&pf->pq)) {
                pf->state = ALGO_NO_PATH;
                return;
            }

            PQNode current = pq_pop(&pf->pq);
            int r = current.row;
            int c = current.col;

            /* Skip duplicates */
            if (grid[r][c].visited) {
                step--;
                continue;
            }

            /* Mark as visited */
            grid[r][c].visited = true;
            pf->nodes_explored++;

            if (grid[r][c].state != CELL_START &&
                grid[r][c].state != CELL_END)
            {
                grid[r][c].state = CELL_EXPLORED;
            }

            /* Check if we've reached the goal */
            if (r == end_row && c == end_col) {
                pf->path_length = reconstruct_path(grid, end_row, end_col);
                pf->state = ALGO_FOUND;
                return;
            }

            /* Explore all 4 neighbors with relaxation */
            for (int d = 0; d < 4; d++) {
                int nr = r + DR[d];
                int nc = c + DC[d];

                if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
                if (grid[nr][nc].state == CELL_WALL) continue;
                if (grid[nr][nc].visited) continue;

                /* Relaxation step */
                float new_g = grid[r][c].g_cost + 1.0f;

                if (new_g < grid[nr][nc].g_cost) {
                    grid[nr][nc].g_cost     = new_g;
                    grid[nr][nc].parent_row = r;
                    grid[nr][nc].parent_col = c;

                    if (grid[nr][nc].state != CELL_END) {
                        grid[nr][nc].state = CELL_FRONTIER;
                    }

                    /* THE KEY DIFFERENCE: Priority Calculation
                     *  Dijkstra: priority = g
                     *  A*:       priority = g + h               */
                    float priority;
                    if (pf->algo_type == ALGO_TYPE_ASTAR) {
                        float h = manhattan_distance(nr, nc,
                                                      end_row, end_col);
                        grid[nr][nc].f_cost = new_g + h;
                        priority = new_g + h;
                    } else {
                        priority = new_g;
                    }

                    pq_push(&pf->pq, nr, nc, priority);
                }
            }
        }
    }
}
