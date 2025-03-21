//
// ALPHA-g experiment
//
// Unpack ALPHA-g data from MIDAS event banks into C++ structures
//
// K.Olchanski
//

#ifndef UNPACK_H
#define UNPACK_H

#include "Trig.h"
#include "Alpha16.h"
#include "Feam.h"
#include "FeamEVB.h"
#include "midasio.h"

TrigEvent* UnpackTrigEvent(TMEvent* event, const TMBank* atat_bank);
Alpha16Event* UnpackAlpha16Event(Alpha16Asm* adcasm, TMEvent* me);
FeamEvent* UnpackFeamEvent(FeamEVB* evb, TMEvent* me, const std::vector<std::string>& banks, bool short_tpc);

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */


