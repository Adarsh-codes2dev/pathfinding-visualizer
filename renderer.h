/* ================================================================
 *  renderer.h — Rendering Declarations and Color Palette
 * ================================================================
 *  This header defines:
 *    1. The color palette for our dark-themed visualizer.
 *    2. Function prototypes for all drawing operations.
 *
 *  Color Design:
 *    We use a modern dark theme with carefully chosen colors.
 *    Each algorithm state has a distinct, high-contrast color
 *    so the visualization is easy to read during presentations.
 * ================================================================ */

#ifndef RENDERER_H
#define RENDERER_H

#include "raylib.h"
#include "grid.h"
#include "pathfinder.h"

/* ----------------------------------------------------------------
 *  COLOR PALETTE
 *  All colors are defined as Raylib Color structs: {R, G, B, A}.
 *
 *  Design rationale:
 *    - Background/empty cells are dark → eyes focus on the action
 *    - Walls use purple → distinct from all algorithm colors
 *    - Start (green) and End (orange) → universal go/stop colors
 *    - Frontier (cyan) vs Explored (indigo) → cool tones, distinct
 *    - Path (gold) → stands out against everything
 * ---------------------------------------------------------------- */

/* Background and grid structure */
#define COLOR_BG          (Color){ 26,  26,  46, 255}  /* #1a1a2e */
#define COLOR_GRID_BORDER (Color){ 40,  50,  80, 255}  /* subtle border */

/* Cell state colors */
#define COLOR_EMPTY       (Color){ 15,  52,  96, 255}  /* #0f3460 */
#define COLOR_WALL        (Color){ 83,  52, 131, 255}  /* #533483 */
#define COLOR_START       (Color){  0, 230, 118, 255}  /* #00e676 */
#define COLOR_END         (Color){255, 109,   0, 255}  /* #ff6d00 */
#define COLOR_FRONTIER    (Color){  0, 188, 212, 255}  /* #00bcd4 */
#define COLOR_EXPLORED    (Color){124,  77, 255, 255}  /* #7c4dff */
#define COLOR_PATH        (Color){255, 214,   0, 255}  /* #ffd600 */

/* UI text colors */
#define COLOR_TITLE       (Color){255, 255, 255, 255}  /* White    */
#define COLOR_HEADING     (Color){  0, 188, 212, 255}  /* Cyan     */
#define COLOR_TEXT         (Color){176, 190, 197, 255}  /* #b0bec5  */
#define COLOR_TEXT_DIM     (Color){ 96, 125, 139, 255}  /* #607d8b  */
#define COLOR_HIGHLIGHT   (Color){255, 214,   0, 255}  /* Gold     */

/* Sidebar panel background */
#define COLOR_PANEL       (Color){ 22,  33,  62, 180}  /* semi-transparent */

/* Additional UI colors for algorithm state */
#define COLOR_SUCCESS     (Color){  0, 230, 118, 255}  /* Green    */
#define COLOR_ERROR       (Color){255,  82,  82, 255}  /* Red      */

/* ----------------------------------------------------------------
 *  FUNCTION DECLARATIONS
 * ---------------------------------------------------------------- */

/*
 * draw_title — Draw the main title bar at the top of the window.
 */
void draw_title(void);

/*
 * draw_grid — Render the entire grid with cell colors.
 *   Includes: cell coloring by state, grid border, hover highlight,
 *   and pulse animations on start/end nodes.
 *
 * Parameters:
 *   grid — The 2D array of cells to draw.
 */
void draw_grid(Cell grid[ROWS][COLS]);

/*
 * draw_sidebar — Render the right-side panel with controls,
 *   color legend, algorithm info, and current status.
 *
 * Parameters:
 *   mode      — Current placement mode (for status display).
 *   start_row — Start node row (-1 if not placed).
 *   start_col — Start node col (-1 if not placed).
 *   end_row   — End node row (-1 if not placed).
 *   end_col   — End node col (-1 if not placed).
 *   pf        — Pointer to the pathfinder (for algo state/stats).
 */
void draw_sidebar(PlacementMode mode,
                  int start_row, int start_col,
                  int end_row, int end_col,
                  const Pathfinder *pf);

#endif /* RENDERER_H */
