/* ================================================================
 *  priority_queue.c — Min-Heap Implementation
 * ================================================================
 *  This file implements the binary min-heap.
 *
 *  The two core operations are:
 *    1. SIFT UP   — After inserting at the bottom, bubble up
 *                   until the heap property is restored.
 *    2. SIFT DOWN — After removing the root, push the replacement
 *                   down until the heap property is restored.
 *
 *  Visual Example of a push (inserting priority 2):
 *
 *    Before:       After insert:    After sift-up:
 *       3              3                2
 *      / \            / \              / \
 *     5   7          5   7            5   3
 *                   /                /
 *                  2  ← new        7  ← swapped down
 *
 *  The new element (2) is smaller than its parent (5), so it
 *  swaps with parent. Then 2 < 3 (new parent), so swap again.
 *  Now 2 is the root. Heap property restored!
 * ================================================================ */

#include "priority_queue.h"

/* ----------------------------------------------------------------
 *  HELPER: swap — Exchange two PQNode elements.
 *
 *  We use a simple temp variable (no XOR tricks).
 *  This is O(1) — just copying 12 bytes (two ints + one float).
 * ---------------------------------------------------------------- */
static void swap(PQNode *a, PQNode *b)
{
    PQNode temp = *a;
    *a = *b;
    *b = temp;
}

/* ----------------------------------------------------------------
 *  HELPER: sift_up — Restore heap property upward.
 *
 *  Start at the given index and compare with parent.
 *  If current node has LOWER priority than parent, swap them.
 *  Repeat until we reach the root or the parent is smaller.
 *
 *  This is called after every push operation.
 *
 *  Time: O(log n) — the tree height is log₂(n).
 * ---------------------------------------------------------------- */
static void sift_up(PriorityQueue *pq, int index)
{
    while (index > 0) {
        /* Parent index formula: (i - 1) / 2 */
        int parent = (index - 1) / 2;

        /* If current is smaller than parent, swap upward */
        if (pq->nodes[index].priority < pq->nodes[parent].priority) {
            swap(&pq->nodes[index], &pq->nodes[parent]);
            index = parent;  /* Move up to parent's position */
        } else {
            break;  /* Heap property satisfied — stop */
        }
    }
}

/* ----------------------------------------------------------------
 *  HELPER: sift_down — Restore heap property downward.
 *
 *  Start at the given index. Compare with both children.
 *  Swap with the SMALLER child if it's smaller than current.
 *  Repeat until we reach a leaf or both children are larger.
 *
 *  This is called after every pop operation.
 *
 *  Time: O(log n) — the tree height is log₂(n).
 * ---------------------------------------------------------------- */
static void sift_down(PriorityQueue *pq, int index)
{
    while (1) {
        int smallest = index;
        int left     = 2 * index + 1;   /* Left child index  */
        int right    = 2 * index + 2;   /* Right child index */

        /* Check if left child exists and is smaller */
        if (left < pq->size &&
            pq->nodes[left].priority < pq->nodes[smallest].priority)
        {
            smallest = left;
        }

        /* Check if right child exists and is even smaller */
        if (right < pq->size &&
            pq->nodes[right].priority < pq->nodes[smallest].priority)
        {
            smallest = right;
        }

        /* If a child was smaller, swap and continue downward */
        if (smallest != index) {
            swap(&pq->nodes[index], &pq->nodes[smallest]);
            index = smallest;
        } else {
            break;  /* Heap property satisfied — stop */
        }
    }
}

/* ================================================================
 *  PUBLIC FUNCTIONS
 * ================================================================ */

/* ----------------------------------------------------------------
 *  pq_init — Reset the queue to empty.
 *  We just set size to 0. The array data doesn't need clearing.
 * ---------------------------------------------------------------- */
void pq_init(PriorityQueue *pq)
{
    pq->size = 0;
}

/* ----------------------------------------------------------------
 *  pq_is_empty — Check if the queue has no elements.
 *  Simple O(1) check on the size counter.
 * ---------------------------------------------------------------- */
bool pq_is_empty(const PriorityQueue *pq)
{
    return pq->size == 0;
}

/* ----------------------------------------------------------------
 *  pq_push — Insert a new node into the heap.
 *
 *  Steps:
 *    1. Place the new node at the END of the array (bottom of tree)
 *    2. Increment size
 *    3. Sift the new node UP to its correct position
 *
 *  If the heap is full, we silently ignore the push.
 *  (In a 30×40 grid, this should never happen.)
 * ---------------------------------------------------------------- */
void pq_push(PriorityQueue *pq, int row, int col, float priority)
{
    /* Safety check: don't overflow the array */
    if (pq->size >= PQ_MAX_SIZE) {
        return;
    }

    /* Step 1: Place at the end */
    int index = pq->size;
    pq->nodes[index].row      = row;
    pq->nodes[index].col      = col;
    pq->nodes[index].priority = priority;

    /* Step 2: Increment size */
    pq->size++;

    /* Step 3: Restore heap property by sifting up */
    sift_up(pq, index);
}

/* ----------------------------------------------------------------
 *  pq_pop — Remove and return the minimum element (root).
 *
 *  Steps:
 *    1. Save the root node (this is our return value)
 *    2. Move the LAST element to the root position
 *    3. Decrement size
 *    4. Sift the new root DOWN to its correct position
 *
 *  This is the heart of Dijkstra's algorithm — every iteration,
 *  we pop the unvisited node with the smallest distance.
 * ---------------------------------------------------------------- */
PQNode pq_pop(PriorityQueue *pq)
{
    /* Step 1: Save the minimum (root) */
    PQNode min_node = pq->nodes[0];

    /* Step 2: Move last element to root */
    pq->size--;
    if (pq->size > 0) {
        pq->nodes[0] = pq->nodes[pq->size];

        /* Step 3: Restore heap property by sifting down */
        sift_down(pq, 0);
    }

    return min_node;
}
