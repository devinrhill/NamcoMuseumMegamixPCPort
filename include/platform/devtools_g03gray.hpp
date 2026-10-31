#pragma once

// The Pac-N-Roll editor's graybox primitives (src/platform/devtools_g03gray.cpp; page "Graybox" of the dev window's Editor tab): boxes, ramps and
// cylinders that are moved, turned about three axes and scaled in the game view, drawn as flat grey surfaces and used as collision (Pac-Man rolls on
// them, live: no restart needed), optionally in place of the whole stage ("graybox stage only"). Kept out of devtools.hpp so a change here does not
// rebuild the game. NOT part of the 1:1 game.
// Game side: G03GrayStage (cAppG03PacnRollGame::Dispose) makes the collision objects, G03GrayDraw3D (devtools::DrawOverlay3D) draws the surfaces,
// G03GrayOnly hides the stage's own data and models, G03GrayStart / G03GrayFell put Pac-Man on the first primitive and back when he falls.
// Editor side (called from devtools_g03edit.cpp): GrayPage is the page, the Gray* functions the game view's mouse and keys.

#include <functional>
#include <string>
#include <vector>

namespace kingdom::game::g03 { class cTaskStage; class cTaskBgHit; class cTaskCharPacman; }
namespace devui { class UI; struct Input; }

namespace devtools
{
// --g03-gray FILE (a graybox.json written by the editor's Save) / --g03-gray-only
void SetG03GrayFile(const std::string& path);
void SetG03GrayOnly(bool on);
bool G03GrayOnly();
bool G03GrayStart(float out[3]);
void G03GrayStage(kingdom::game::g03::cTaskStage* stage, std::vector<kingdom::game::g03::cTaskBgHit*>& hits);
bool G03GrayFell(kingdom::game::g03::cTaskCharPacman* pac);
void G03GrayDraw3D();

// editor side
void GrayPage(devui::UI& ui, const float placeAt[3], const std::string& dir, const std::function<void()>& restart);
// game view: the gizmo of the selected primitive, then the primitive under the mouse (or placing a new one); true = the press was used
bool GrayPressGizmo(const devui::Input& in, const float vp[16], const float o[3], const float d[3]);
bool GrayPressBody(const devui::Input& in, const float o[3], const float d[3], const std::function<bool(float[3])>& ground);
void GrayDrag(const devui::Input& in, const float o[3], const float d[3]);   // every frame
bool GrayDragging();
bool GrayKeys(const devui::Input& in);                                        // Delete, Ctrl+D, PgUp / PgDn, 1 / 2 / 3: true = a key was used
void GrayDeselect();
bool GrayHasSelection();
bool GrayShown();                                                             // the Graybox page was drawn in the last frames
int GrayLastEditFrame();
bool GrayCanUndo();
bool GrayCanRedo();
void GrayUndo();
void GrayRedo();
void GraySave(const std::string& dir);
void GrayLoad(const std::string& dir);
}
