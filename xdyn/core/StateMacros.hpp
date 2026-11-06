/*
 * StateMacros.hpp
 *
 *  Created on: Jun 16, 2014
 *      Author: cady
 */

#ifndef STATEMACROS_HPP_
#define STATEMACROS_HPP_

#include "StateLayout.hpp"

#define NB_OF_STATES_PER_BODY (state::N)

#define XIDX(i) (state::index((i), state::X))
#define YIDX(i) (state::index((i), state::Y))
#define ZIDX(i) (state::index((i), state::Z))
#define UIDX(i) (state::index((i), state::U))
#define VIDX(i) (state::index((i), state::V))
#define WIDX(i) (state::index((i), state::W))
#define PIDX(i) (state::index((i), state::P))
#define QIDX(i) (state::index((i), state::Q))
#define RIDX(i) (state::index((i), state::R))
#define QRIDX(i) (state::index((i), state::QR))
#define QIIDX(i) (state::index((i), state::QI))
#define QJIDX(i) (state::index((i), state::QJ))
#define QKIDX(i) (state::index((i), state::QK))

#define _X(x,i) (&x[0] + XIDX(i))
#define _Y(x,i) (&x[0] + YIDX(i))
#define _Z(x,i) (&x[0] + ZIDX(i))
#define _U(x,i) (&x[0] + UIDX(i))
#define _V(x,i) (&x[0] + VIDX(i))
#define _W(x,i) (&x[0] + WIDX(i))
#define _P(x,i) (&x[0] + PIDX(i))
#define _Q(x,i) (&x[0] + QIDX(i))
#define _R(x,i) (&x[0] + RIDX(i))
#define _QR(x,i) (&x[0] + QRIDX(i))
#define _QI(x,i) (&x[0] + QIIDX(i))
#define _QJ(x,i) (&x[0] + QJIDX(i))
#define _QK(x,i) (&x[0] + QKIDX(i))


#endif /* STATEMACROS_HPP_ */
