/* ================================================================
 *  queue_stack.h — Queue (FIFO) and Stack (LIFO) for BFS & DFS
 * ================================================================
 *  BFS and DFS are structurally IDENTICAL algorithms. The only
 *  difference is the data structure used for the "frontier":
 *
 *    BFS uses a QUEUE (FIFO) → explores level-by-level
 *    DFS uses a STACK (LIFO) → explores as deep as possible first
 *
 *  DSA Concept: Queue vs Stack
 *  ───────────────────────────
 *    Queue (FIFO):  First In, First Out
 *      Push: add to BACK    Pop: remove from FRONT
 *      Like a line at a shop — first person in line gets served first.
 *
 *    Stack (LIFO):  Last In, First Out
 *      Push: add to TOP     Pop: remove from TOP
 *      Like a stack of plates — you always take from the top.
 *
 *  Why BFS finds shortest path but DFS doesn't:
 *  ─────────────────────────────────────────────
 *  BFS explores ALL nodes at distance d before any node at d+1.
 *  So the first time BFS reaches a node, it's via the shortest path.
 *
 *  DFS goes deep first — it might reach a node via a LONGER path
 *  before discovering the shorter one. DFS is useful for maze
 *  generation and cycle detection, not shortest paths.
 *
 *  Implementation: Circular array queue, linear array stack.
 *  Both are O(1) push/pop with no malloc needed.
 * ================================================================ */

#ifndef QUEUE_STACK_H
#define QUEUE_STACK_H

#include <stdbool.h>

/* Maximum capacity — 30×40 = 1200 cells, but nodes can be
 * re-enqueued in some implementations. 4800 is generous. */
#define QS_MAX_SIZE 4800

/* ----------------------------------------------------------------
 *  QSNode — A (row, col) pair stored in the queue or stack.
 * ---------------------------------------------------------------- */
typedef struct {
    int row;
    int col;
} QSNode;

/* ================================================================
 *  QUEUE (FIFO) — Used by BFS
 *
 *  Circular buffer implementation:
 *    front → index of next element to dequeue
 *    rear  → index where next element will be enqueued
 *    count → number of elements currently in the queue
 *
 *  Circular means when rear hits the end of the array, it wraps
 *  around to index 0. This avoids wasting space.
 *
 *    Enqueue:  nodes[rear] = item; rear = (rear+1) % MAX
 *    Dequeue:  item = nodes[front]; front = (front+1) % MAX
 *
 *  Both operations are O(1).
 * ================================================================ */
typedef struct {
    QSNode nodes[QS_MAX_SIZE];
    int front;    /* Index of the front element             */
    int rear;     /* Index where next element goes          */
    int count;    /* Number of elements in the queue        */
} Queue;

/* Initialize queue to empty state */
static inline void queue_init(Queue *q) {
    q->front = 0;
    q->rear  = 0;
    q->count = 0;
}

/* Check if queue is empty */
static inline bool queue_is_empty(const Queue *q) {
    return q->count == 0;
}

/* Enqueue: Add element to the BACK */
static inline void queue_push(Queue *q, int row, int col) {
    if (q->count >= QS_MAX_SIZE) return;  /* Overflow guard */
    q->nodes[q->rear].row = row;
    q->nodes[q->rear].col = col;
    q->rear = (q->rear + 1) % QS_MAX_SIZE;  /* Circular wrap */
    q->count++;
}

/* Dequeue: Remove and return element from the FRONT */
static inline QSNode queue_pop(Queue *q) {
    QSNode node = q->nodes[q->front];
    q->front = (q->front + 1) % QS_MAX_SIZE;  /* Circular wrap */
    q->count--;
    return node;
}

/* ================================================================
 *  STACK (LIFO) — Used by DFS
 *
 *  Simple array-backed stack:
 *    top → index of the topmost element (or -1 if empty)
 *
 *    Push:  top++; nodes[top] = item
 *    Pop:   item = nodes[top]; top--
 *
 *  Both operations are O(1).
 * ================================================================ */
typedef struct {
    QSNode nodes[QS_MAX_SIZE];
    int top;     /* Index of top element (-1 = empty)      */
} Stack;

/* Initialize stack to empty state */
static inline void stack_init(Stack *s) {
    s->top = -1;
}

/* Check if stack is empty */
static inline bool stack_is_empty(const Stack *s) {
    return s->top == -1;
}

/* Push: Add element to the TOP */
static inline void stack_push(Stack *s, int row, int col) {
    if (s->top >= QS_MAX_SIZE - 1) return;  /* Overflow guard */
    s->top++;
    s->nodes[s->top].row = row;
    s->nodes[s->top].col = col;
}

/* Pop: Remove and return element from the TOP */
static inline QSNode stack_pop(Stack *s) {
    QSNode node = s->nodes[s->top];
    s->top--;
    return node;
}

#endif /* QUEUE_STACK_H */
