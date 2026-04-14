/* ================================================================
 *  priority_queue.h — Min-Heap Priority Queue
 * ================================================================
 *  A priority queue is THE key data structure for Dijkstra's and
 *  A* algorithms. It always gives us the lowest-cost node first.
 *
 *  DSA Concept: Binary Min-Heap
 *  ────────────────────────────
 *  We implement it as an ARRAY-BACKED BINARY TREE:
 *
 *       Index:    0        ← Root (minimum element)
 *                / \
 *               1   2
 *              / \ / \
 *             3  4 5  6
 *
 *  For any node at index i:
 *    Parent:      (i - 1) / 2
 *    Left child:  2 * i + 1
 *    Right child: 2 * i + 2
 *
 *  The HEAP PROPERTY: Every parent is ≤ its children.
 *  This guarantees the root is always the minimum.
 *
 *  Time Complexities:
 *    push (insert):     O(log n) — sift up from bottom
 *    pop  (extract min): O(log n) — sift down from top
 *    peek (get min):     O(1)     — just read the root
 *    is_empty:           O(1)     — check size
 *
 *  Why not a sorted array?
 *    Insertion into a sorted array is O(n) due to shifting.
 *    A heap gives us O(log n) for both insert and extract-min,
 *    which is much better when we have many nodes.
 * ================================================================ */

#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stdbool.h>

/* Maximum number of elements the heap can hold.
 * In the worst case, each cell could be pushed multiple times
 * (when we discover shorter paths), so we use a generous size.
 * For a 30×40 grid = 1200 cells, 4800 is more than enough.      */
#define PQ_MAX_SIZE 4800

/* ----------------------------------------------------------------
 *  PQNode — A single element in the priority queue.
 *
 *  Stores the grid coordinates (row, col) of a cell and its
 *  priority value (lower = higher priority).
 *
 *  For Dijkstra: priority = g_cost (distance from start)
 *  For A*:       priority = f_cost = g_cost + heuristic
 * ---------------------------------------------------------------- */
typedef struct {
    int   row;       /* Row of the cell in the grid        */
    int   col;       /* Column of the cell in the grid     */
    float priority;  /* Lower value = popped first          */
} PQNode;

/* ----------------------------------------------------------------
 *  PriorityQueue — The min-heap container.
 *
 *  nodes[] — Array storing all elements in heap order.
 *  size    — Number of elements currently in the heap.
 *
 *  The heap is stored in a flat array (no pointers, no malloc).
 *  This makes it cache-friendly and easy to understand.
 * ---------------------------------------------------------------- */
typedef struct {
    PQNode nodes[PQ_MAX_SIZE];
    int    size;
} PriorityQueue;

/* ----------------------------------------------------------------
 *  FUNCTION DECLARATIONS
 * ---------------------------------------------------------------- */

/*
 * pq_init — Initialize the priority queue to empty.
 *   Must call this before using the queue.
 */
void pq_init(PriorityQueue *pq);

/*
 * pq_is_empty — Check if the queue has no elements.
 *   Returns: true if empty, false otherwise.
 */
bool pq_is_empty(const PriorityQueue *pq);

/*
 * pq_push — Insert a new element into the heap.
 *   Adds the element at the bottom and "sifts up" to restore
 *   the heap property.
 *
 *   Time: O(log n) — at most log₂(n) swaps up the tree.
 *
 * Parameters:
 *   pq       — Pointer to the priority queue.
 *   row, col — Grid coordinates of the cell.
 *   priority — The cost/priority value (lower = better).
 */
void pq_push(PriorityQueue *pq, int row, int col, float priority);

/*
 * pq_pop — Remove and return the minimum-priority element.
 *   Swaps the root with the last element, removes the last,
 *   then "sifts down" to restore the heap property.
 *
 *   Time: O(log n) — at most log₂(n) swaps down the tree.
 *
 *   PRECONDITION: The queue must not be empty.
 *   Returns: The PQNode with the lowest priority value.
 */
PQNode pq_pop(PriorityQueue *pq);

#endif /* PRIORITY_QUEUE_H */
