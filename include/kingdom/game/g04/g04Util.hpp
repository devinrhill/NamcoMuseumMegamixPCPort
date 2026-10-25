#pragma once

#include "common.hpp"

namespace kingdom { class cKingdomModel; }

// Small engine helpers the Gator Panic code uses everywhere.
namespace kingdom::game::g04
{
// FUN_8001dd54: the bind translation of a node (ResNode +0x38..+0x40, relative to its parent; not animated), w = 1. Zero for an unknown node.
Vector4 ResNodeTrans(kingdom::cKingdomModel* m, const char* node);
}
