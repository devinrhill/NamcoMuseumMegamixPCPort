// The Unity UI of Pac-Motos (menu buttons, boss text, start text: MotosUI*.cs drove Unity uGUI animations) is not ported: natively the
// game draws its Anita layouts. The converted code calls these through `UnityUI`; they accept everything and report "nothing playing".
#include "MotosUIMotosHome.hpp"
#include "MotosUIMotosMultiHome.hpp"
#include "MotosUIMotosBossText.hpp"
#include "MotosUIMotosStartText.hpp"

void MotosUIMotosHome::PlayAnim(EButtonType, EAnim) {}
void MotosUIMotosHome::PlayBaseAnim(EBaseAnim) {}
bool MotosUIMotosHome::IsPlayAnim(EButtonType, EAnim) { return false; }
bool MotosUIMotosHome::IsPlayBaseAnim(EBaseAnim) { return false; }
void MotosUIMotosHome::ResetAnim(EButtonType, bool) {}
void MotosUIMotosHome::ResetBaseAnim(bool) {}

void MotosUIMotosBossText::PlayBaseAnim(EBaseAnim) {}
bool MotosUIMotosBossText::IsPlayBaseAnim(EBaseAnim) { return false; }
void MotosUIMotosBossText::ResetBaseAnim(bool) {}

void MotosUIMotosStartText::SetPlayerNo(int) {}

void MotosUIMotosMultiHome::PlayAnim(EButtonType, EAnim) {}
void MotosUIMotosMultiHome::PlayBaseAnim(EBaseAnim) {}
bool MotosUIMotosMultiHome::IsPlayAnim(EButtonType, EAnim) { return false; }
bool MotosUIMotosMultiHome::IsPlayBaseAnim(EBaseAnim) { return false; }
void MotosUIMotosMultiHome::ResetAnim(EButtonType, bool) {}
