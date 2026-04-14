/* ================================================================
 *  pathfinder.h — Pathfinding Algorithm Declarations
 * ================================================================
 *  This header defines the Pathfinder "engine" that runs
 *  Dijkstra's, A*, BFS, and DFS algorithms on our grid.
 *
 *  Key Design: STEP-BY-STEP ANIMATION
 *  ───────────────────────────────────
 *  Instead of running the entire algorithm in one function call,
 *  we process a few nodes PER FRAME. This lets Raylib draw the
 *  grid between steps, creating a smooth animation.
 *
 *  The Pathfinder struct holds all algorithm state between frames:
 *    - The priority queue / queue / stack (worklist of nodes)
 *    - The current state (running, found, etc.)
 *    - Animation speed (how many nodes per frame)
 *    - Statistics (nodes explored, path length)
 *
 *  ALGORITHM COMPARISON (Viva Answer):
 *  ════════════════════════════════════
 *   Algorithm   │ Data Structure │ Shortest Path? │ Time      │ Explores
 *  ─────────────┼────────────────┼────────────────┼───────────┼──────────
 *   BFS         │ Queue (FIFO)   │ YES (unwgt.)   │ O(V + E)  │ Level by level
 *   DFS         │ Stack (LIFO)   │ NO             │ O(V + E)  │ Deep first
 *   Dijkstra    │ Min-Heap (PQ)  │ YES (weighted) │ O(E logV) │ Nearest first
 *   A*          │ Min-Heap (PQ)  │ YES (weighted) │ O(E logV) │ Toward goal
 *
 *  BFS vs DFS — Same code, different frontier!
 *    BFS uses a queue  → explores in "waves" outward
 *    DFS uses a stack  → dives deep, then backtracks
 * ================================================================ */

#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "grid.h"
#include "priority_queue.h"
#include "queue_stack.h"

/* ----------------------------------------------------------------
 *  ALGORITHM STATE
 *  Tracks whether the algorithm is idle, running, or finished.
 * ---------------------------------------------------------------- */
typedef enum {
    ALGO_IDLE,       /* Not running — user is drawing walls         */
    ALGO_RUNNING,    /* Algorithm is animating (processing nodes)   */
    ALGO_FOUND,      /* Algorithm finished — path found             */
    ALGO_NO_PATH     /* Algorithm finished — no path exists         */
} AlgoState;

/* ----------------------------------------------------------------
 *  ALGORITHM TYPE
 *  Which algorithm to run. The user toggles with keyboard keys.
 *
 *  BFS and DFS:
 *    BFS uses a QUEUE → First In First Out → level-by-level
 *    DFS uses a STACK → Last In First Out  → deep first
 *    Both are O(V + E) — no priority queue overhead!
 *
 *  Dijkstra and A*:
 *    Both use a min-heap priority queue.
 *    Dijkstra: priority = g(n)
 *    A*:       priority = g(n) + h(n)
 * ---------------------------------------------------------------- */
typedef enum {
    ALGO_TYPE_BFS,       /* Queue — level-by-level, shortest path   */
    ALGO_TYPE_DFS,       /* Stack — deep first, NO shortest path    */
    ALGO_TYPE_DIJKSTRA,  /* Min-heap — shortest path (weighted)     */
    ALGO_TYPE_ASTAR      /* Min-heap + heuristic — fastest optimal  */
} AlgoType;

/* ----------------------------------------------------------------
 *  PATHFINDER STRUCT
 *  Holds the complete state of the running algorithm.
 *
 *  This struct persists between frames so the algorithm can
 *  "pause" after each batch of steps and let the renderer draw.
 *
 *  We store ALL three data structures but only USE one at a time:
 *    - pq    for Dijkstra / A*
 *    - queue for BFS
 *    - stack for DFS
 *
 *  This avoids unions and keeps the code simple.
 * ---------------------------------------------------------------- */
typedef struct {
    PriorityQueue pq;       /* Min-heap for Dijkstra / A*          */
    Queue         queue;    /* FIFO queue for BFS                  */
    Stack         stack;    /* LIFO stack for DFS                  */
    AlgoState     state;    /* Current algorithm state              */
    AlgoType      algo_type;/* Which algorithm is selected          */
    int steps_per_frame;    /* Animation speed (adjustable)         */
    int nodes_explored;     /* Counter: how many nodes finalized    */
    int path_length;        /* Length of the path found             */
} Pathfinder;

/* ----------------------------------------------------------------
 *  FUNCTION DECLARATIONS
 * ---------------------------------------------------------------- */

/*
 * pathfinder_init — Initialize the pathfinder to idle state.
 *   Sets default animation speed and zeroes all counters.
 */
void pathfinder_init(Pathfinder *pf);

/*
 * pathfinder_start_bfs — Begin BFS (Breadth-First Search).
 *   Uses a QUEUE (FIFO). Explores level by level.
 *   Guaranteed to find the SHORTEST PATH on unweighted grids.
 *
 *   BFS VISUALIZATION:
 *     The frontier expands in "waves" — like dropping a stone
 *     in water. Each wave covers all nodes at the same distance.
 *
 *       Wave 1: ....1....
 *       Wave 2: ...121...
 *       Wave 3: ..12321..
 *       Wave 4: .1234321.
 *
 * Parameters:
 *   pf        — The pathfinder engine.
 *   grid      — The grid to search.
 *   start_row, start_col — Start position.
 */
void pathfinder_start_bfs(Pathfinder *pf, Cell grid[ROWS][COLS],
                           int start_row, int start_col);

/*
 * pathfinder_start_dfs — Begin DFS (Depth-First Search).
 *   Uses a STACK (LIFO). Dives deep, then backtracks.
 *   Does NOT guarantee shortest path!
 *
 *   DFS VISUALIZATION:
 *     The frontier snakes through the grid in one direction
 *     until it hits a dead end, then backtracks.
 *
 *   DFS is useful for:
 *     - Maze generation (recursive backtracker)
 *     - Cycle detection
 *     - Topological sorting
 *     - Connected component detection
 *
 * Parameters:
 *   pf        — The pathfinder engine.
 *   grid      — The grid to search.
 *   start_row, start_col — Start position.
 */
void pathfinder_start_dfs(Pathfinder *pf, Cell grid[ROWS][COLS],
                           int start_row, int start_col);

/*
 * pathfinder_start_dijkstra — Begin Dijkstra's algorithm.
 *   Uses a min-heap. Explores nearest-first.
 *   Guaranteed to find the shortest path (even with weights).
 */
void pathfinder_start_dijkstra(Pathfinder *pf, Cell grid[ROWS][COLS],
                                int start_row, int start_col);

/*
 * pathfinder_start_astar — Begin A* algorithm.
 *   Uses a min-heap + Manhattan distance heuristic.
 *   Explores toward the goal. Fastest optimal pathfinder.
 */
void pathfinder_start_astar(Pathfinder *pf, Cell grid[ROWS][COLS],
                            int start_row, int start_col,
                            int end_row, int end_col);

/*
 * pathfinder_step — Process one batch of nodes.
 *   Called once per frame. Dispatches to the correct algorithm
 *   based on pf->algo_type.
 */
void pathfinder_step(Pathfinder *pf, Cell grid[ROWS][COLS],
                     int end_row, int end_col);

#endif /* PATHFINDER_H */
