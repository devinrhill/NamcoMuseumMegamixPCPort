#pragma once

// The dev window's "Pac-Man" tab (src/platform/devtools_g03pac.cpp, only inside a Pac-N-Roll stage): his cTaskCharPacman members (position, velocity,
// radius, HP / lives, form, power-up and the state flags, the movement parameters of his current form) as fields, a handle in the game view that
// drags him to a new place, and a few actions. Kept out of devtools.hpp so a change here does not rebuild the game. NOT part of the 1:1 game.
// The Editor tab's game view uses PacPress / PacDrag too, so he can be dragged while editing.

namespace devui { struct Input; }

namespace devtools
{
void RegisterG03PacPanel();
void G03PacDraw3D();   // his handle (devtools::DrawOverlay3D), only while the Pac-Man or Editor tab is shown
// cTaskCharPacman::Draw: factor on his model scale = radius / 0.38 while the tab's "scale his model with the radius" box is on (1 otherwise)
float G03PacModelScale(float radius);
void PacTouch();       // the Editor tab is shown this frame: draw the handle there too
// game view: a press within a few pixels of him (or his projected radius) starts a drag on the plane at his height (Alt: up / down, Ctrl: snaps to 0.5)
bool PacPress(const devui::Input& in, const float vp[16], const float o[3], const float d[3]);
void PacDrag(const devui::Input& in, const float o[3], const float d[3]);   // every frame
bool PacDragging();
}
