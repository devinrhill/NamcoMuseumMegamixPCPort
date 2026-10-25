#include "kingdom/game/g07/cTaskCharEnemyNabicon.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include "kingdom/game/g07/cMotosGameSys.hpp"

#include <iterator>
#include <numbers>

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskCharEnemyPupaRed.hpp"
#include "kingdom/game/g07/MO_CHAR_PARAMS.hpp"
#include "kingdom/game/g07/NET_RF_PACKET.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* cTaskCharEnemyNabicon::s_StageNormal[38] = {[]() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o1 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o1->m_nEnemyKind = 1; _o1->m_nEnemyMax = 4; _o1->m_nEnemyTotal = 12; return _o1; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o2 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o2->m_nEnemyKind = 1; _o2->m_nEnemyMax = 4; _o2->m_nEnemyTotal = 12; return _o2; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o3 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o3->m_nEnemyKind = 1; _o3->m_nEnemyMax = 4; _o3->m_nEnemyTotal = 12; return _o3; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o4 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o4->m_nEnemyKind = 1; _o4->m_nEnemyMax = 4; _o4->m_nEnemyTotal = 12; return _o4; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o5 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o5->m_nEnemyKind = 1; _o5->m_nEnemyMax = 4; _o5->m_nEnemyTotal = 12; return _o5; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o6 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o6->m_nEnemyKind = 1; _o6->m_nEnemyMax = 4; _o6->m_nEnemyTotal = 12; return _o6; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o7 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o7->m_nEnemyKind = 1; _o7->m_nEnemyMax = 4; _o7->m_nEnemyTotal = 12; return _o7; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o8 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o8->m_nEnemyKind = 1; _o8->m_nEnemyMax = 4; _o8->m_nEnemyTotal = 12; return _o8; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o9 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o9->m_nEnemyKind = 1; _o9->m_nEnemyMax = 4; _o9->m_nEnemyTotal = 12; return _o9; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o10 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o10->m_nEnemyKind = 1; _o10->m_nEnemyMax = 4; _o10->m_nEnemyTotal = 12; return _o10; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o11 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o11->m_nEnemyKind = 1; _o11->m_nEnemyMax = 4; _o11->m_nEnemyTotal = 12; return _o11; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o12 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o12->m_nEnemyKind = 1; _o12->m_nEnemyMax = 4; _o12->m_nEnemyTotal = 12; return _o12; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o13 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o13->m_nEnemyKind = 1; _o13->m_nEnemyMax = 4; _o13->m_nEnemyTotal = 12; return _o13; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o14 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o14->m_nEnemyKind = 1; _o14->m_nEnemyMax = 4; _o14->m_nEnemyTotal = 12; return _o14; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o15 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o15->m_nEnemyKind = 1; _o15->m_nEnemyMax = 4; _o15->m_nEnemyTotal = 12; return _o15; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o16 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o16->m_nEnemyKind = 1; _o16->m_nEnemyMax = 4; _o16->m_nEnemyTotal = 12; return _o16; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o17 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o17->m_nEnemyKind = 1; _o17->m_nEnemyMax = 4; _o17->m_nEnemyTotal = 12; return _o17; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o18 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o18->m_nEnemyKind = 1; _o18->m_nEnemyMax = 4; _o18->m_nEnemyTotal = 12; return _o18; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o19 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o19->m_nEnemyKind = 1; _o19->m_nEnemyMax = 4; _o19->m_nEnemyTotal = 12; return _o19; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o20 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o20->m_nEnemyKind = 1; _o20->m_nEnemyMax = 4; _o20->m_nEnemyTotal = 12; return _o20; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o21 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o21->m_nEnemyKind = 1; _o21->m_nEnemyMax = 4; _o21->m_nEnemyTotal = 12; return _o21; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o22 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o22->m_nEnemyKind = 1; _o22->m_nEnemyMax = 4; _o22->m_nEnemyTotal = 12; return _o22; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o23 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o23->m_nEnemyKind = 1; _o23->m_nEnemyMax = 4; _o23->m_nEnemyTotal = 12; return _o23; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o24 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o24->m_nEnemyKind = 1; _o24->m_nEnemyMax = 4; _o24->m_nEnemyTotal = 12; return _o24; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o25 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o25->m_nEnemyKind = 1; _o25->m_nEnemyMax = 4; _o25->m_nEnemyTotal = 12; return _o25; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o26 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o26->m_nEnemyKind = 1; _o26->m_nEnemyMax = 4; _o26->m_nEnemyTotal = 12; return _o26; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o27 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o27->m_nEnemyKind = 1; _o27->m_nEnemyMax = 4; _o27->m_nEnemyTotal = 12; return _o27; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o28 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o28->m_nEnemyKind = 1; _o28->m_nEnemyMax = 4; _o28->m_nEnemyTotal = 12; return _o28; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o29 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o29->m_nEnemyKind = 1; _o29->m_nEnemyMax = 4; _o29->m_nEnemyTotal = 12; return _o29; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o30 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o30->m_nEnemyKind = 1; _o30->m_nEnemyMax = 4; _o30->m_nEnemyTotal = 12; return _o30; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o31 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o31->m_nEnemyKind = 1; _o31->m_nEnemyMax = 4; _o31->m_nEnemyTotal = 12; return _o31; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o32 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o32->m_nEnemyKind = 1; _o32->m_nEnemyMax = 4; _o32->m_nEnemyTotal = 12; return _o32; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o33 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o33->m_nEnemyKind = 1; _o33->m_nEnemyMax = 4; _o33->m_nEnemyTotal = 12; return _o33; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o34 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o34->m_nEnemyKind = 1; _o34->m_nEnemyMax = 4; _o34->m_nEnemyTotal = 12; return _o34; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o35 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o35->m_nEnemyKind = 1; _o35->m_nEnemyMax = 4; _o35->m_nEnemyTotal = 12; return _o35; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o36 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o36->m_nEnemyKind = 1; _o36->m_nEnemyMax = 4; _o36->m_nEnemyTotal = 12; return _o36; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o37 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o37->m_nEnemyKind = 1; _o37->m_nEnemyMax = 4; _o37->m_nEnemyTotal = 12; return _o37; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o38 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o38->m_nEnemyKind = 1; _o38->m_nEnemyMax = 4; _o38->m_nEnemyTotal = 12; return _o38; }()};
cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* cTaskCharEnemyNabicon::s_StageEasy[38] = {[]() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o39 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o39->m_nEnemyKind = 1; _o39->m_nEnemyMax = 4; _o39->m_nEnemyTotal = 12; return _o39; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o40 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o40->m_nEnemyKind = 1; _o40->m_nEnemyMax = 4; _o40->m_nEnemyTotal = 12; return _o40; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o41 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o41->m_nEnemyKind = 1; _o41->m_nEnemyMax = 4; _o41->m_nEnemyTotal = 12; return _o41; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o42 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o42->m_nEnemyKind = 1; _o42->m_nEnemyMax = 4; _o42->m_nEnemyTotal = 12; return _o42; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o43 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o43->m_nEnemyKind = 1; _o43->m_nEnemyMax = 4; _o43->m_nEnemyTotal = 12; return _o43; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o44 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o44->m_nEnemyKind = 1; _o44->m_nEnemyMax = 4; _o44->m_nEnemyTotal = 12; return _o44; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o45 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o45->m_nEnemyKind = 1; _o45->m_nEnemyMax = 4; _o45->m_nEnemyTotal = 12; return _o45; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o46 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o46->m_nEnemyKind = 1; _o46->m_nEnemyMax = 4; _o46->m_nEnemyTotal = 12; return _o46; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o47 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o47->m_nEnemyKind = 1; _o47->m_nEnemyMax = 4; _o47->m_nEnemyTotal = 12; return _o47; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o48 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o48->m_nEnemyKind = 1; _o48->m_nEnemyMax = 4; _o48->m_nEnemyTotal = 12; return _o48; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o49 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o49->m_nEnemyKind = 1; _o49->m_nEnemyMax = 4; _o49->m_nEnemyTotal = 12; return _o49; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o50 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o50->m_nEnemyKind = 1; _o50->m_nEnemyMax = 4; _o50->m_nEnemyTotal = 12; return _o50; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o51 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o51->m_nEnemyKind = 1; _o51->m_nEnemyMax = 4; _o51->m_nEnemyTotal = 12; return _o51; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o52 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o52->m_nEnemyKind = 1; _o52->m_nEnemyMax = 4; _o52->m_nEnemyTotal = 12; return _o52; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o53 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o53->m_nEnemyKind = 1; _o53->m_nEnemyMax = 4; _o53->m_nEnemyTotal = 12; return _o53; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o54 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o54->m_nEnemyKind = 1; _o54->m_nEnemyMax = 4; _o54->m_nEnemyTotal = 12; return _o54; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o55 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o55->m_nEnemyKind = 1; _o55->m_nEnemyMax = 4; _o55->m_nEnemyTotal = 12; return _o55; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o56 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o56->m_nEnemyKind = 1; _o56->m_nEnemyMax = 4; _o56->m_nEnemyTotal = 12; return _o56; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o57 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o57->m_nEnemyKind = 1; _o57->m_nEnemyMax = 4; _o57->m_nEnemyTotal = 12; return _o57; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o58 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o58->m_nEnemyKind = 1; _o58->m_nEnemyMax = 4; _o58->m_nEnemyTotal = 12; return _o58; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o59 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o59->m_nEnemyKind = 1; _o59->m_nEnemyMax = 4; _o59->m_nEnemyTotal = 12; return _o59; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o60 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o60->m_nEnemyKind = 1; _o60->m_nEnemyMax = 4; _o60->m_nEnemyTotal = 12; return _o60; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o61 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o61->m_nEnemyKind = 1; _o61->m_nEnemyMax = 4; _o61->m_nEnemyTotal = 12; return _o61; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o62 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o62->m_nEnemyKind = 1; _o62->m_nEnemyMax = 4; _o62->m_nEnemyTotal = 12; return _o62; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o63 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o63->m_nEnemyKind = 1; _o63->m_nEnemyMax = 4; _o63->m_nEnemyTotal = 12; return _o63; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o64 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o64->m_nEnemyKind = 1; _o64->m_nEnemyMax = 4; _o64->m_nEnemyTotal = 12; return _o64; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o65 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o65->m_nEnemyKind = 1; _o65->m_nEnemyMax = 4; _o65->m_nEnemyTotal = 12; return _o65; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o66 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o66->m_nEnemyKind = 1; _o66->m_nEnemyMax = 4; _o66->m_nEnemyTotal = 12; return _o66; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o67 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o67->m_nEnemyKind = 1; _o67->m_nEnemyMax = 4; _o67->m_nEnemyTotal = 12; return _o67; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o68 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o68->m_nEnemyKind = 1; _o68->m_nEnemyMax = 4; _o68->m_nEnemyTotal = 12; return _o68; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o69 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o69->m_nEnemyKind = 1; _o69->m_nEnemyMax = 4; _o69->m_nEnemyTotal = 12; return _o69; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o70 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o70->m_nEnemyKind = 1; _o70->m_nEnemyMax = 4; _o70->m_nEnemyTotal = 12; return _o70; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o71 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o71->m_nEnemyKind = 1; _o71->m_nEnemyMax = 4; _o71->m_nEnemyTotal = 12; return _o71; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o72 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o72->m_nEnemyKind = 1; _o72->m_nEnemyMax = 4; _o72->m_nEnemyTotal = 12; return _o72; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o73 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o73->m_nEnemyKind = 1; _o73->m_nEnemyMax = 4; _o73->m_nEnemyTotal = 12; return _o73; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o74 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o74->m_nEnemyKind = 1; _o74->m_nEnemyMax = 4; _o74->m_nEnemyTotal = 12; return _o74; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o75 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o75->m_nEnemyKind = 1; _o75->m_nEnemyMax = 4; _o75->m_nEnemyTotal = 12; return _o75; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o76 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o76->m_nEnemyKind = 1; _o76->m_nEnemyMax = 4; _o76->m_nEnemyTotal = 12; return _o76; }()};
cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* cTaskCharEnemyNabicon::s_StageHard[38] = {[]() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o77 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o77->m_nEnemyKind = 1; _o77->m_nEnemyMax = 4; _o77->m_nEnemyTotal = 12; return _o77; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o78 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o78->m_nEnemyKind = 1; _o78->m_nEnemyMax = 4; _o78->m_nEnemyTotal = 12; return _o78; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o79 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o79->m_nEnemyKind = 1; _o79->m_nEnemyMax = 4; _o79->m_nEnemyTotal = 12; return _o79; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o80 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o80->m_nEnemyKind = 1; _o80->m_nEnemyMax = 4; _o80->m_nEnemyTotal = 12; return _o80; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o81 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o81->m_nEnemyKind = 1; _o81->m_nEnemyMax = 4; _o81->m_nEnemyTotal = 12; return _o81; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o82 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o82->m_nEnemyKind = 1; _o82->m_nEnemyMax = 4; _o82->m_nEnemyTotal = 12; return _o82; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o83 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o83->m_nEnemyKind = 1; _o83->m_nEnemyMax = 4; _o83->m_nEnemyTotal = 12; return _o83; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o84 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o84->m_nEnemyKind = 1; _o84->m_nEnemyMax = 4; _o84->m_nEnemyTotal = 12; return _o84; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o85 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o85->m_nEnemyKind = 1; _o85->m_nEnemyMax = 4; _o85->m_nEnemyTotal = 12; return _o85; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o86 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o86->m_nEnemyKind = 1; _o86->m_nEnemyMax = 4; _o86->m_nEnemyTotal = 12; return _o86; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o87 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o87->m_nEnemyKind = 1; _o87->m_nEnemyMax = 4; _o87->m_nEnemyTotal = 12; return _o87; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o88 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o88->m_nEnemyKind = 1; _o88->m_nEnemyMax = 4; _o88->m_nEnemyTotal = 12; return _o88; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o89 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o89->m_nEnemyKind = 1; _o89->m_nEnemyMax = 4; _o89->m_nEnemyTotal = 12; return _o89; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o90 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o90->m_nEnemyKind = 1; _o90->m_nEnemyMax = 4; _o90->m_nEnemyTotal = 12; return _o90; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o91 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o91->m_nEnemyKind = 1; _o91->m_nEnemyMax = 4; _o91->m_nEnemyTotal = 12; return _o91; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o92 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o92->m_nEnemyKind = 1; _o92->m_nEnemyMax = 4; _o92->m_nEnemyTotal = 12; return _o92; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o93 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o93->m_nEnemyKind = 1; _o93->m_nEnemyMax = 4; _o93->m_nEnemyTotal = 12; return _o93; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o94 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o94->m_nEnemyKind = 1; _o94->m_nEnemyMax = 4; _o94->m_nEnemyTotal = 12; return _o94; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o95 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o95->m_nEnemyKind = 1; _o95->m_nEnemyMax = 4; _o95->m_nEnemyTotal = 12; return _o95; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o96 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o96->m_nEnemyKind = 1; _o96->m_nEnemyMax = 4; _o96->m_nEnemyTotal = 12; return _o96; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o97 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o97->m_nEnemyKind = 1; _o97->m_nEnemyMax = 4; _o97->m_nEnemyTotal = 12; return _o97; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o98 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o98->m_nEnemyKind = 1; _o98->m_nEnemyMax = 4; _o98->m_nEnemyTotal = 12; return _o98; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o99 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o99->m_nEnemyKind = 1; _o99->m_nEnemyMax = 4; _o99->m_nEnemyTotal = 12; return _o99; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o100 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o100->m_nEnemyKind = 1; _o100->m_nEnemyMax = 4; _o100->m_nEnemyTotal = 12; return _o100; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o101 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o101->m_nEnemyKind = 1; _o101->m_nEnemyMax = 4; _o101->m_nEnemyTotal = 12; return _o101; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o102 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o102->m_nEnemyKind = 1; _o102->m_nEnemyMax = 4; _o102->m_nEnemyTotal = 12; return _o102; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o103 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o103->m_nEnemyKind = 1; _o103->m_nEnemyMax = 4; _o103->m_nEnemyTotal = 12; return _o103; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o104 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o104->m_nEnemyKind = 1; _o104->m_nEnemyMax = 4; _o104->m_nEnemyTotal = 12; return _o104; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o105 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o105->m_nEnemyKind = 1; _o105->m_nEnemyMax = 4; _o105->m_nEnemyTotal = 12; return _o105; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o106 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o106->m_nEnemyKind = 1; _o106->m_nEnemyMax = 4; _o106->m_nEnemyTotal = 12; return _o106; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o107 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o107->m_nEnemyKind = 1; _o107->m_nEnemyMax = 4; _o107->m_nEnemyTotal = 12; return _o107; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o108 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o108->m_nEnemyKind = 1; _o108->m_nEnemyMax = 4; _o108->m_nEnemyTotal = 12; return _o108; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o109 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o109->m_nEnemyKind = 1; _o109->m_nEnemyMax = 4; _o109->m_nEnemyTotal = 12; return _o109; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o110 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o110->m_nEnemyKind = 1; _o110->m_nEnemyMax = 4; _o110->m_nEnemyTotal = 12; return _o110; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o111 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o111->m_nEnemyKind = 1; _o111->m_nEnemyMax = 4; _o111->m_nEnemyTotal = 12; return _o111; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o112 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o112->m_nEnemyKind = 1; _o112->m_nEnemyMax = 4; _o112->m_nEnemyTotal = 12; return _o112; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o113 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o113->m_nEnemyKind = 1; _o113->m_nEnemyMax = 4; _o113->m_nEnemyTotal = 12; return _o113; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o114 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o114->m_nEnemyKind = 1; _o114->m_nEnemyMax = 4; _o114->m_nEnemyTotal = 12; return _o114; }()};
cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* cTaskCharEnemyNabicon::s_StageLAN[13] = {[]() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o115 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o115->m_nEnemyKind = 1; _o115->m_nEnemyMax = 4; _o115->m_nEnemyTotal = 12; return _o115; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o116 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o116->m_nEnemyKind = 1; _o116->m_nEnemyMax = 4; _o116->m_nEnemyTotal = 12; return _o116; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o117 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o117->m_nEnemyKind = 1; _o117->m_nEnemyMax = 4; _o117->m_nEnemyTotal = 12; return _o117; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o118 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o118->m_nEnemyKind = 1; _o118->m_nEnemyMax = 4; _o118->m_nEnemyTotal = 12; return _o118; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o119 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o119->m_nEnemyKind = 1; _o119->m_nEnemyMax = 4; _o119->m_nEnemyTotal = 12; return _o119; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o120 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o120->m_nEnemyKind = 1; _o120->m_nEnemyMax = 4; _o120->m_nEnemyTotal = 12; return _o120; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o121 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o121->m_nEnemyKind = 1; _o121->m_nEnemyMax = 4; _o121->m_nEnemyTotal = 12; return _o121; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o122 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o122->m_nEnemyKind = 1; _o122->m_nEnemyMax = 4; _o122->m_nEnemyTotal = 12; return _o122; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o123 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o123->m_nEnemyKind = 1; _o123->m_nEnemyMax = 4; _o123->m_nEnemyTotal = 12; return _o123; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o124 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o124->m_nEnemyKind = 1; _o124->m_nEnemyMax = 4; _o124->m_nEnemyTotal = 12; return _o124; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o125 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o125->m_nEnemyKind = 1; _o125->m_nEnemyMax = 4; _o125->m_nEnemyTotal = 12; return _o125; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o126 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o126->m_nEnemyKind = 1; _o126->m_nEnemyMax = 4; _o126->m_nEnemyTotal = 12; return _o126; }(), []() { cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* _o127 = new cTaskCharEnemyNabicon::NAVICON_STAGE_DATA(); _o127->m_nEnemyKind = 1; _o127->m_nEnemyMax = 4; _o127->m_nEnemyTotal = 12; return _o127; }()};
bool cTaskCharEnemyNabicon::m_nInitFlag = false;
cTaskCharEnemyPupaRed* cTaskCharEnemyNabicon::m_pSpawnChar[16] = {};

cTaskCharEnemyNabicon::cTaskCharEnemyNabicon()
{
	Init();
}

void cTaskCharEnemyNabicon::InitParam(MO_CHAR_PARAMS* pParam)
{
	cTaskChar::InitParam(pParam);
	m_nHP = (int)fExParam(0);
}

cTaskCharEnemy* cTaskCharEnemyNabicon::Construct()
{
	return new cTaskCharEnemyNabicon();
}

void cTaskCharEnemyNabicon::Init()
{
	int num = 0;
	cTaskCharEnemyNabicon::NAVICON_STAGE_DATA** array = nullptr;
	if (!Constants::IS_LANPLAY())
	{
		array = ((KingdomGlobal::pMotosGameSys->nGameLevel() == 0) ? s_StageNormal : ((KingdomGlobal::pMotosGameSys->nGameLevel() != 1) ? s_StageHard : s_StageEasy));
		num = ((KingdomGlobal::MotosSys->nStageIdx() >= 1 && KingdomGlobal::MotosSys->nStageIdx() <= 35) ? (num + (int)(KingdomGlobal::MotosSys->nStageIdx() - 1)) : ((KingdomGlobal::MotosSys->nStageIdx() != 0) ? (num + 37) : (num + 35)));
	}
	else
	{
		array = s_StageLAN;
		num += (int)(KingdomGlobal::MotosSys->nStageIdx() - 36);
	}
	m_nEnemyKind = array[num]->m_nEnemyKind;
	m_nEnemyMax = array[num]->m_nEnemyMax;
	m_nEnemyTotal = array[num]->m_nEnemyTotal;
	m_nVibraTimer = 0;
	Constants::VEC3_SET(m_vBaseTrans, 0.0f, 0.0f, 0.0f);
	m_bCrushFlag = false;
	m_nEnemyInter = Constants::FPS_TIME(5.0f);
	m_nAcceptDir = 0;
	m_nEnemyTimer = 0;
	if (m_nInitFlag)
	{
		return;
	}
	m_nInitFlag = true;
	for (int i = 0; i < (int)std::size(m_pSpawnChar); i++)
	{
		m_pSpawnChar[i] = nullptr;
	}
	Vector4 v = Vector4::zero;
	::Constants::jetGeoVecUnit(v);
	for (int j = 0; j < m_nEnemyMax; j++)
	{
		Constants::VEC3_SET(v, 0.0f, 0.0f, 50.0f);
		if (m_pSpawnChar[j] == nullptr)
		{
			m_pSpawnChar[j] = new cTaskCharEnemyPupaRed();
			m_pSpawnChar[j]->Create(0u);
		}
		m_pSpawnChar[j]->ChangeTaiki();
		m_pSpawnChar[j]->InitParam(Constants::get_g_MoEnemyParam()[1]);
		::Constants::jetGeoVecCopy(m_pSpawnChar[j]->vMTrans(), v);
		::Constants::jetGeoVecUnit(m_pSpawnChar[j]->vRotate());
		::Constants::jetGeoVecUnit(m_pSpawnChar[j]->vScale());
		m_pSpawnChar[j]->vScale()[0] = 1.0f;
		m_pSpawnChar[j]->vScale()[1] = 1.0f;
		m_pSpawnChar[j]->vScale()[2] = 1.0f;
		m_pSpawnChar[j]->nKind() = 1u;
		m_pSpawnChar[j]->nChipColor() = m_pSpawnChar[j]->Kind2ChipColor(1);
		if (KingdomGlobal::g_pMotosGame->IsFirstPlay())
		{
			KingdomGlobal::MotosSys->bMotosCharDead(m_pSpawnChar[j]->GrobalID()) = true;
		}
		ConnectChild(m_pSpawnChar[j]);
	}
	if (!KingdomGlobal::g_pMotosGame->IsFirstPlay())
	{
		bool flag = false;
		Constants::VEC3_SET(v, 0.0f, 0.0f, 50.0f);
		m_bRetry = false;
		for (uint num2 = 0u; num2 < 32; num2++)
		{
			cTaskCharEnemy* cTaskCharEnemy2 = KingdomGlobal::g_pMotosGame->pEnemys(num2);
			if (cTaskCharEnemy2 != nullptr)
			{
				for (int k = 0; k < m_nEnemyMax; k++)
				{
					if (cTaskCharEnemy2 == m_pSpawnChar[k])
					{
						if (!KingdomGlobal::MotosSys->bMotosCharDead(cTaskCharEnemy2->GrobalID()))
						{
							flag = true;
						}
						break;
					}
				}
			}
			if (flag)
			{
				m_bRetry = true;
				break;
			}
		}
	}
	else
	{
		m_bRetry = false;
	}
}

void cTaskCharEnemyNabicon::Term()
{
	m_nInitFlag = false;
	cTaskCharEnemy::Term();
}

bool cTaskCharEnemyNabicon::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_NAVICON");
	string array[3] = {"TSA_CHAR_ENEMY_NAVICON", "CLA_CHAR_ENEMY_NAVICON", "CHA_CHAR_ENEMY_NAVICON"};
	int num = ::Constants::ARRAY_SIZE(array);
	for (int i = 0; i < num; i++)
	{
		if (!KingdomGlobal::KingSys->SearchData(array[i]).empty())
		{
			cKingdomModel2->RegistAnm(0u, array[i]);
		}
	}
	cKingdomModel2->SetLightSetIdx(2);
	cKingdomModel2->SetFogIdx(2);
	cKingdomModel2->IsDrawShadowDepth(true);
	return true;
}

uint cTaskCharEnemyNabicon::Input()
{
	return 0u;
}

cTaskChar* cTaskCharEnemyNabicon::m_SpawnEnemy(int dir)
{
	cTaskChar* cTaskChar2 = nullptr;
	if (m_nEnemyTotal != 0)
	{
		Vector4 v;
		::Constants::jetGeoVecCopy(v, Constants::motosAxis2Vector(dir));
		cTaskChar2 = m_GetTaikiEnemy();
		if (cTaskChar2 != nullptr)
		{
			::Constants::jetGeoVecCopy(cTaskChar2->vMTrans(), vMTrans());
			Constants::VEC3_ADD(cTaskChar2->vMTrans(), cTaskChar2->vMTrans(), v);
			OffFlag(16385u);
			KingdomGlobal::MotosSys->bMotosCharDead(cTaskChar2->GrobalID()) = false;
		}
		m_nEnemyTotal--;
	}
	return cTaskChar2;
}

bool cTaskCharEnemyNabicon::Status()
{
	if (!bTestFlag(16384u) && m_bCrushFlag && !CustomKingdomUtil::IF(m_nHP))
	{
		for (int i = 0; i < 5; i++)
		{
			Vector4 v;
			::Constants::jetGeoVecCopy(v, vMTrans());
			v[0] += Constants::MOTOS_RANDRANGEF(-0.5f, 0.5f);
			v[2] += Constants::MOTOS_RANDRANGEF(-0.5f, 0.5f);
			Constants::SpawnEffect(22, v, nullptr, i * 7);
		}
		SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE007_BOSS_DEATH);
		if (!CustomKingdomUtil::IF(KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nEnemyTaikiBits & (1 << (int)ID())))
		{
			ExecGivePoints((int)KingdomGlobal::g_pMotosGame->pPlayers(0u)->GrobalID(), 50000);
		}
		vMTrans()[2] = 50.0f;
	}
	return cTaskCharEnemy::Status();
}

bool cTaskCharEnemyNabicon::ActionBoth()
{
	return true;
}

bool cTaskCharEnemyNabicon::ActionOther()
{
	return true;
}

bool cTaskCharEnemyNabicon::ActionLocal()
{
	switch (Step())
	{
	case 0:
		{
			if ((double)Constants::MOTOS_P_ABS(vRotate()[1] - (float)std::numbers::pi) < 0.01)
			{
				vRotate()[1] = 0.0f;
			}
			if (vRotate()[1] == 0.0f)
			{
				m_nAcceptDir = 170;
			}
			else
			{
				m_nAcceptDir = 85;
			}
			for (uint num2 = 0u; num2 < 32; num2++)
			{
				cTaskCharEnemy* cTaskCharEnemy2 = KingdomGlobal::g_pMotosGame->pEnemys(num2);
				if (cTaskCharEnemy2 == nullptr || cTaskCharEnemy2->ID() == ID() || cTaskCharEnemy2->nKind() != 15)
				{
					continue;
				}
				if (cTaskCharEnemy2->vMTrans()[2] == vMTrans()[2] && Constants::MOTOS_P_ABS(cTaskCharEnemy2->vMTrans()[0] - vMTrans()[0]) <= 3.0f)
				{
					if (cTaskCharEnemy2->vMTrans()[0] > vMTrans()[0])
					{
						m_nAcceptDir &= -5;
					}
					else
					{
						m_nAcceptDir &= -65;
					}
				}
				if (cTaskCharEnemy2->vMTrans()[0] == vMTrans()[0] && Constants::MOTOS_P_ABS(cTaskCharEnemy2->vMTrans()[2] - vMTrans()[2]) <= 3.0f)
				{
					if (cTaskCharEnemy2->vMTrans()[2] > vMTrans()[2])
					{
						m_nAcceptDir &= -2;
					}
					else
					{
						m_nAcceptDir &= -17;
					}
				}
			}
			if (m_bRetry)
			{
				m_SpawnEnemy(1);
				m_bRetry = false;
			}
			::Constants::jetGeoVecCopy(m_vBaseTrans, vMTrans());
			::Constants::jetGeoVecCopy(m_vTransTemp, vMTrans());
			Step()++;
			break;
		}
	case 1:
		if (m_nEnemyTimer++ == m_nEnemyInter)
		{
			bool flag = true;
			for (uint num = 0u; num < 32; num++)
			{
				cTaskCharEnemy* cTaskCharEnemy2 = KingdomGlobal::g_pMotosGame->pEnemys(num);
				if (cTaskCharEnemy2 != nullptr && cTaskCharEnemy2->bTestFlag(4u) && cTaskCharEnemy2->nKind() != 14 && cTaskCharEnemy2->nKind() != 15 && cTaskCharEnemy2->nKind() != 22 && cTaskCharEnemy2->nKind() != 23 && !cTaskCharEnemy2->bTestFlag(64u) && !cTaskCharEnemy2->bTestFlag(16384u))
				{
					flag = false;
					break;
				}
			}
			if (!flag || KingdomGlobal::g_pMotosGame->bMultiPlay())
			{
				int dir = Random::Range(0, 4);
				for (int i = 0; i < 32; i++)
				{
					if (CustomKingdomUtil::IF(m_nAcceptDir & (1 << (i & 7))) && !CustomKingdomUtil::IF(dir--))
					{
						dir = i & 7;
						break;
					}
				}
				m_SpawnEnemy(dir);
			}
			m_nEnemyTimer = 0;
		}
		if (m_nVibraTimer != 0)
		{
			m_nVibraTimer--;
		}
		break;
	}
	if (!Constants::MOTOS_IS_PAUSE() && m_nVibraTimer != 0)
	{
		::Constants::jetGeoVecCopy(vMTrans(), m_vTransTemp);
		vMTrans()[0] += Constants::MOTOS_RANDRANGEF(-0.1f, 0.1f);
		vMTrans()[2] += Constants::MOTOS_RANDRANGEF(-0.1f, 0.1f);
	}
	else
	{
		::Constants::jetGeoVecCopy(vMTrans(), m_vTransTemp);
	}
	nInputDir() = (ushort)Input();
	return true;
}

void cTaskCharEnemyNabicon::ExecReflect(NET_RF_PACKET* pPacket)
{
	cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars(pPacket->m_nChar[0]);
	cTaskChar* cTaskChar3 = KingdomGlobal::g_pMotosGame->pChars(pPacket->m_nChar[1]);
	cTaskChar* cTaskChar4 = ((cTaskChar2->GrobalID() != GrobalID()) ? cTaskChar2 : cTaskChar3);
	if (!cTaskChar4->bTestFlag(4u))
	{
		cTaskChar::ExecReflect(pPacket);
	}
}

void cTaskCharEnemyNabicon::ExecReflectSub(Vector4 pVec, float hit_power, float charge_pow)
{
	if (m_bCrushFlag)
	{
		if (m_nHP != 0)
		{
			m_nHP--;
		}
		m_nVibraTimer = 10;
	}
}

cTaskChar* cTaskCharEnemyNabicon::m_GetTaikiEnemy()
{
	for (int i = 0; i < 16; i++)
	{
		if (m_pSpawnChar[i] != nullptr && m_pSpawnChar[i]->bTestFlag(16384u) && m_pSpawnChar[i]->vMTrans()[2] >= 50.0f)
		{
			return m_pSpawnChar[i];
		}
	}
	return nullptr;
}

void cTaskCharEnemyNabicon::SetMMSParam(int para1, int para2, int para3)
{
	if (para1 != 0)
	{
		m_nEnemyInter = para1;
	}
	else
	{
		m_nEnemyInter = Constants::FPS_TIME(5.0f);
	}
	m_nEnemyTimer = para2;
	m_bCrushFlag = (CustomKingdomUtil::IF(para3) ? true : false);
}

}
