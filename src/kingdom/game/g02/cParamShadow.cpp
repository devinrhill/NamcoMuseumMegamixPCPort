#include "kingdom/game/g02/cParamShadow.hpp"

#include "kingdom/cKingdomCSV.hpp"

namespace kingdom::game::g02
{

// DOL 0x8008120c [audited] (CSV_SHADOW, column 1 = the value: cJetCSV::F32(csv, row, 1, out))
bool cParamShadow::Load()
{
	kingdom::cKingdomCSV csv;
	if (!csv.RegistData("CSV_SHADOW")) return false;   // 0xaf402e71
	const float DEG = 0.017453292f;                     // FLOAT_8047aa24
	vLight[0] = DEG * csv.F32(0, 1, 0.0f);
	vLight[1] = DEG * csv.F32(1, 1, 0.0f);
	vLight[2] = DEG * csv.F32(2, 1, 0.0f);
	vLight[3] = 1.0f;
	fPosY = csv.F32(3, 1, 0.0f);
	fScaleY = csv.F32(4, 1, 0.0f);
	return true;
}

}
