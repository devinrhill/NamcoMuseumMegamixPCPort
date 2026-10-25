// Stand-ins for the Unity overlay UIs of the C# port (scrapped, see MotosUIBase.hpp): the game tasks null-check them and nothing creates one,
// so every call is a no-op. (Formerly in g03native/HudSupport.cpp.)
#include "MotosUIRollStageName.hpp"
#include "MotosUIRollCaution.hpp"
void MotosUIRollStageName::Init(int) {}
void MotosUIRollStageName::PlayBaseAnim(EBaseAnim) {}
bool MotosUIRollStageName::IsPlayBaseAnim(EBaseAnim) { return false; }
bool MotosUIRollStageName::IsPlayBaseAnim() { return false; }
void MotosUIRollStageName::ResetBaseAnim(bool) {}
void MotosUIRollStageName::OnPause() {}
void MotosUIRollStageName::OnResume() {}
void MotosUIRollCaution::Init(EType) {}
void MotosUIRollCaution::PlayBaseAnim(EBaseAnim) {}
bool MotosUIRollCaution::IsPlayBaseAnim(EBaseAnim) { return false; }
bool MotosUIRollCaution::IsPlayBaseAnim() { return false; }
void MotosUIRollCaution::ResetBaseAnim(bool) {}
void MotosUIRollCaution::OnPause() {}
void MotosUIRollCaution::OnResume() {}

// MotosUIPause / MotosUIRetry: Unity overlay UIs (the C# pause / retry screens text); never created, every call is a no-op
#include "MotosUIPause.hpp"
#include "MotosUIRetry.hpp"
void MotosUIPause::PlayAnim(EButtonType, EAnim) {}
bool MotosUIPause::IsPlayAnim(EButtonType, EAnim) { return false; }
bool MotosUIPause::IsPlayAnim(EButtonType) { return false; }
void MotosUIPause::ResetAnim(EButtonType, bool) {}
void MotosUIPause::ResetAllAnim(bool) {}
void MotosUIPause::OnPause() {}
void MotosUIPause::OnResume() {}
void MotosUIRetry::PlayAnim(EButtonType, EAnim) {}
bool MotosUIRetry::IsPlayAnim(EButtonType, EAnim) { return false; }
bool MotosUIRetry::IsPlayAnim(EButtonType) { return false; }
void MotosUIRetry::ResetAnim(EButtonType, bool) {}
void MotosUIRetry::ResetAllAnim(bool) {}
void MotosUIRetry::OnPause() {}
void MotosUIRetry::OnResume() {}
