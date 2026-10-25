#include "KingdomGlobal.hpp"

cSysCoreStructure* KingdomGlobal::SysCore{};

kingdom::cKingdomSys* KingdomGlobal::KingSys{};

kingdom::game::g01::cG01GalagaSys* KingdomGlobal::GalagaSys{};
kingdom::game::g02::cG02RallyXSys* KingdomGlobal::RallyXSys{};
kingdom::game::g11::cG11GrobdaSys* KingdomGlobal::GrobdaSys{};
kingdom::game::g04::cG04WaniWaniSys* KingdomGlobal::WaniWaniSys{};

kingdom::game::g01::cAppG01GalagaGame* KingdomGlobal::g_pGalagaGame{};

kingdom::game::g03::cG03PacnRollSys* KingdomGlobal::PacnRollSys{};

kingdom::game::g03::cAppG03PacnRollGame* KingdomGlobal::g_pRollGame{};

kingdom::game::g07::cG07MotosSys* KingdomGlobal::MotosSys{};

kingdom::game::g07::cMotosGameSys* KingdomGlobal::pMotosGameSys{};

kingdom::game::g07::cAppG07MotosGame* KingdomGlobal::g_pMotosGame{};

float KingdomGlobal::ShaderInGameTime{};
