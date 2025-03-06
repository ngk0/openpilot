#include "car.h"

namespace {
#define DIM 9
#define EDIM 9
#define MEDIM 9
typedef void (*Hfun)(double *, double *, double *);

double mass;

void set_mass(double x){ mass = x;}

double rotational_inertia;

void set_rotational_inertia(double x){ rotational_inertia = x;}

double center_to_front;

void set_center_to_front(double x){ center_to_front = x;}

double center_to_rear;

void set_center_to_rear(double x){ center_to_rear = x;}

double stiffness_front;

void set_stiffness_front(double x){ stiffness_front = x;}

double stiffness_rear;

void set_stiffness_rear(double x){ stiffness_rear = x;}
const static double MAHA_THRESH_25 = 3.8414588206941227;
const static double MAHA_THRESH_24 = 5.991464547107981;
const static double MAHA_THRESH_30 = 3.8414588206941227;
const static double MAHA_THRESH_26 = 3.8414588206941227;
const static double MAHA_THRESH_27 = 3.8414588206941227;
const static double MAHA_THRESH_29 = 3.8414588206941227;
const static double MAHA_THRESH_28 = 3.8414588206941227;
const static double MAHA_THRESH_31 = 3.8414588206941227;

/******************************************************************************
 *                       Code generated with SymPy 1.12                       *
 *                                                                            *
 *              See http://www.sympy.org/ for more information.               *
 *                                                                            *
 *                         This file is part of 'ekf'                         *
 ******************************************************************************/
void err_fun(double *nom_x, double *delta_x, double *out_6723881171663706926) {
   out_6723881171663706926[0] = delta_x[0] + nom_x[0];
   out_6723881171663706926[1] = delta_x[1] + nom_x[1];
   out_6723881171663706926[2] = delta_x[2] + nom_x[2];
   out_6723881171663706926[3] = delta_x[3] + nom_x[3];
   out_6723881171663706926[4] = delta_x[4] + nom_x[4];
   out_6723881171663706926[5] = delta_x[5] + nom_x[5];
   out_6723881171663706926[6] = delta_x[6] + nom_x[6];
   out_6723881171663706926[7] = delta_x[7] + nom_x[7];
   out_6723881171663706926[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_2739404126537862093) {
   out_2739404126537862093[0] = -nom_x[0] + true_x[0];
   out_2739404126537862093[1] = -nom_x[1] + true_x[1];
   out_2739404126537862093[2] = -nom_x[2] + true_x[2];
   out_2739404126537862093[3] = -nom_x[3] + true_x[3];
   out_2739404126537862093[4] = -nom_x[4] + true_x[4];
   out_2739404126537862093[5] = -nom_x[5] + true_x[5];
   out_2739404126537862093[6] = -nom_x[6] + true_x[6];
   out_2739404126537862093[7] = -nom_x[7] + true_x[7];
   out_2739404126537862093[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_8598021807935865210) {
   out_8598021807935865210[0] = 1.0;
   out_8598021807935865210[1] = 0;
   out_8598021807935865210[2] = 0;
   out_8598021807935865210[3] = 0;
   out_8598021807935865210[4] = 0;
   out_8598021807935865210[5] = 0;
   out_8598021807935865210[6] = 0;
   out_8598021807935865210[7] = 0;
   out_8598021807935865210[8] = 0;
   out_8598021807935865210[9] = 0;
   out_8598021807935865210[10] = 1.0;
   out_8598021807935865210[11] = 0;
   out_8598021807935865210[12] = 0;
   out_8598021807935865210[13] = 0;
   out_8598021807935865210[14] = 0;
   out_8598021807935865210[15] = 0;
   out_8598021807935865210[16] = 0;
   out_8598021807935865210[17] = 0;
   out_8598021807935865210[18] = 0;
   out_8598021807935865210[19] = 0;
   out_8598021807935865210[20] = 1.0;
   out_8598021807935865210[21] = 0;
   out_8598021807935865210[22] = 0;
   out_8598021807935865210[23] = 0;
   out_8598021807935865210[24] = 0;
   out_8598021807935865210[25] = 0;
   out_8598021807935865210[26] = 0;
   out_8598021807935865210[27] = 0;
   out_8598021807935865210[28] = 0;
   out_8598021807935865210[29] = 0;
   out_8598021807935865210[30] = 1.0;
   out_8598021807935865210[31] = 0;
   out_8598021807935865210[32] = 0;
   out_8598021807935865210[33] = 0;
   out_8598021807935865210[34] = 0;
   out_8598021807935865210[35] = 0;
   out_8598021807935865210[36] = 0;
   out_8598021807935865210[37] = 0;
   out_8598021807935865210[38] = 0;
   out_8598021807935865210[39] = 0;
   out_8598021807935865210[40] = 1.0;
   out_8598021807935865210[41] = 0;
   out_8598021807935865210[42] = 0;
   out_8598021807935865210[43] = 0;
   out_8598021807935865210[44] = 0;
   out_8598021807935865210[45] = 0;
   out_8598021807935865210[46] = 0;
   out_8598021807935865210[47] = 0;
   out_8598021807935865210[48] = 0;
   out_8598021807935865210[49] = 0;
   out_8598021807935865210[50] = 1.0;
   out_8598021807935865210[51] = 0;
   out_8598021807935865210[52] = 0;
   out_8598021807935865210[53] = 0;
   out_8598021807935865210[54] = 0;
   out_8598021807935865210[55] = 0;
   out_8598021807935865210[56] = 0;
   out_8598021807935865210[57] = 0;
   out_8598021807935865210[58] = 0;
   out_8598021807935865210[59] = 0;
   out_8598021807935865210[60] = 1.0;
   out_8598021807935865210[61] = 0;
   out_8598021807935865210[62] = 0;
   out_8598021807935865210[63] = 0;
   out_8598021807935865210[64] = 0;
   out_8598021807935865210[65] = 0;
   out_8598021807935865210[66] = 0;
   out_8598021807935865210[67] = 0;
   out_8598021807935865210[68] = 0;
   out_8598021807935865210[69] = 0;
   out_8598021807935865210[70] = 1.0;
   out_8598021807935865210[71] = 0;
   out_8598021807935865210[72] = 0;
   out_8598021807935865210[73] = 0;
   out_8598021807935865210[74] = 0;
   out_8598021807935865210[75] = 0;
   out_8598021807935865210[76] = 0;
   out_8598021807935865210[77] = 0;
   out_8598021807935865210[78] = 0;
   out_8598021807935865210[79] = 0;
   out_8598021807935865210[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_8990824843719242692) {
   out_8990824843719242692[0] = state[0];
   out_8990824843719242692[1] = state[1];
   out_8990824843719242692[2] = state[2];
   out_8990824843719242692[3] = state[3];
   out_8990824843719242692[4] = state[4];
   out_8990824843719242692[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8000000000000007*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_8990824843719242692[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_8990824843719242692[7] = state[7];
   out_8990824843719242692[8] = state[8];
}
void F_fun(double *state, double dt, double *out_2803601295947585167) {
   out_2803601295947585167[0] = 1;
   out_2803601295947585167[1] = 0;
   out_2803601295947585167[2] = 0;
   out_2803601295947585167[3] = 0;
   out_2803601295947585167[4] = 0;
   out_2803601295947585167[5] = 0;
   out_2803601295947585167[6] = 0;
   out_2803601295947585167[7] = 0;
   out_2803601295947585167[8] = 0;
   out_2803601295947585167[9] = 0;
   out_2803601295947585167[10] = 1;
   out_2803601295947585167[11] = 0;
   out_2803601295947585167[12] = 0;
   out_2803601295947585167[13] = 0;
   out_2803601295947585167[14] = 0;
   out_2803601295947585167[15] = 0;
   out_2803601295947585167[16] = 0;
   out_2803601295947585167[17] = 0;
   out_2803601295947585167[18] = 0;
   out_2803601295947585167[19] = 0;
   out_2803601295947585167[20] = 1;
   out_2803601295947585167[21] = 0;
   out_2803601295947585167[22] = 0;
   out_2803601295947585167[23] = 0;
   out_2803601295947585167[24] = 0;
   out_2803601295947585167[25] = 0;
   out_2803601295947585167[26] = 0;
   out_2803601295947585167[27] = 0;
   out_2803601295947585167[28] = 0;
   out_2803601295947585167[29] = 0;
   out_2803601295947585167[30] = 1;
   out_2803601295947585167[31] = 0;
   out_2803601295947585167[32] = 0;
   out_2803601295947585167[33] = 0;
   out_2803601295947585167[34] = 0;
   out_2803601295947585167[35] = 0;
   out_2803601295947585167[36] = 0;
   out_2803601295947585167[37] = 0;
   out_2803601295947585167[38] = 0;
   out_2803601295947585167[39] = 0;
   out_2803601295947585167[40] = 1;
   out_2803601295947585167[41] = 0;
   out_2803601295947585167[42] = 0;
   out_2803601295947585167[43] = 0;
   out_2803601295947585167[44] = 0;
   out_2803601295947585167[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_2803601295947585167[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_2803601295947585167[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_2803601295947585167[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_2803601295947585167[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_2803601295947585167[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_2803601295947585167[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_2803601295947585167[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_2803601295947585167[53] = -9.8000000000000007*dt;
   out_2803601295947585167[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_2803601295947585167[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_2803601295947585167[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2803601295947585167[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2803601295947585167[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_2803601295947585167[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_2803601295947585167[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_2803601295947585167[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2803601295947585167[62] = 0;
   out_2803601295947585167[63] = 0;
   out_2803601295947585167[64] = 0;
   out_2803601295947585167[65] = 0;
   out_2803601295947585167[66] = 0;
   out_2803601295947585167[67] = 0;
   out_2803601295947585167[68] = 0;
   out_2803601295947585167[69] = 0;
   out_2803601295947585167[70] = 1;
   out_2803601295947585167[71] = 0;
   out_2803601295947585167[72] = 0;
   out_2803601295947585167[73] = 0;
   out_2803601295947585167[74] = 0;
   out_2803601295947585167[75] = 0;
   out_2803601295947585167[76] = 0;
   out_2803601295947585167[77] = 0;
   out_2803601295947585167[78] = 0;
   out_2803601295947585167[79] = 0;
   out_2803601295947585167[80] = 1;
}
void h_25(double *state, double *unused, double *out_1726729315642570782) {
   out_1726729315642570782[0] = state[6];
}
void H_25(double *state, double *unused, double *out_2811870095170146086) {
   out_2811870095170146086[0] = 0;
   out_2811870095170146086[1] = 0;
   out_2811870095170146086[2] = 0;
   out_2811870095170146086[3] = 0;
   out_2811870095170146086[4] = 0;
   out_2811870095170146086[5] = 0;
   out_2811870095170146086[6] = 1;
   out_2811870095170146086[7] = 0;
   out_2811870095170146086[8] = 0;
}
void h_24(double *state, double *unused, double *out_8285531725452842947) {
   out_8285531725452842947[0] = state[4];
   out_8285531725452842947[1] = state[5];
}
void H_24(double *state, double *unused, double *out_7632191599826134349) {
   out_7632191599826134349[0] = 0;
   out_7632191599826134349[1] = 0;
   out_7632191599826134349[2] = 0;
   out_7632191599826134349[3] = 0;
   out_7632191599826134349[4] = 1;
   out_7632191599826134349[5] = 0;
   out_7632191599826134349[6] = 0;
   out_7632191599826134349[7] = 0;
   out_7632191599826134349[8] = 0;
   out_7632191599826134349[9] = 0;
   out_7632191599826134349[10] = 0;
   out_7632191599826134349[11] = 0;
   out_7632191599826134349[12] = 0;
   out_7632191599826134349[13] = 0;
   out_7632191599826134349[14] = 1;
   out_7632191599826134349[15] = 0;
   out_7632191599826134349[16] = 0;
   out_7632191599826134349[17] = 0;
}
void h_30(double *state, double *unused, double *out_4713285606981469022) {
   out_4713285606981469022[0] = state[4];
}
void H_30(double *state, double *unused, double *out_2941209042313386156) {
   out_2941209042313386156[0] = 0;
   out_2941209042313386156[1] = 0;
   out_2941209042313386156[2] = 0;
   out_2941209042313386156[3] = 0;
   out_2941209042313386156[4] = 1;
   out_2941209042313386156[5] = 0;
   out_2941209042313386156[6] = 0;
   out_2941209042313386156[7] = 0;
   out_2941209042313386156[8] = 0;
}
void h_26(double *state, double *unused, double *out_3353658075972099194) {
   out_3353658075972099194[0] = state[7];
}
void H_26(double *state, double *unused, double *out_6553373414044202310) {
   out_6553373414044202310[0] = 0;
   out_6553373414044202310[1] = 0;
   out_6553373414044202310[2] = 0;
   out_6553373414044202310[3] = 0;
   out_6553373414044202310[4] = 0;
   out_6553373414044202310[5] = 0;
   out_6553373414044202310[6] = 0;
   out_6553373414044202310[7] = 1;
   out_6553373414044202310[8] = 0;
}
void h_27(double *state, double *unused, double *out_6678927504001655871) {
   out_6678927504001655871[0] = state[3];
}
void H_27(double *state, double *unused, double *out_5115972354113811067) {
   out_5115972354113811067[0] = 0;
   out_5115972354113811067[1] = 0;
   out_5115972354113811067[2] = 0;
   out_5115972354113811067[3] = 1;
   out_5115972354113811067[4] = 0;
   out_5115972354113811067[5] = 0;
   out_5115972354113811067[6] = 0;
   out_5115972354113811067[7] = 0;
   out_5115972354113811067[8] = 0;
}
void h_29(double *state, double *unused, double *out_8080891962152112839) {
   out_8080891962152112839[0] = state[1];
}
void H_29(double *state, double *unused, double *out_2430977697998993972) {
   out_2430977697998993972[0] = 0;
   out_2430977697998993972[1] = 1;
   out_2430977697998993972[2] = 0;
   out_2430977697998993972[3] = 0;
   out_2430977697998993972[4] = 0;
   out_2430977697998993972[5] = 0;
   out_2430977697998993972[6] = 0;
   out_2430977697998993972[7] = 0;
   out_2430977697998993972[8] = 0;
}
void h_28(double *state, double *unused, double *out_1677254236340810226) {
   out_1677254236340810226[0] = state[0];
}
void H_28(double *state, double *unused, double *out_7513376715068524546) {
   out_7513376715068524546[0] = 1;
   out_7513376715068524546[1] = 0;
   out_7513376715068524546[2] = 0;
   out_7513376715068524546[3] = 0;
   out_7513376715068524546[4] = 0;
   out_7513376715068524546[5] = 0;
   out_7513376715068524546[6] = 0;
   out_7513376715068524546[7] = 0;
   out_7513376715068524546[8] = 0;
}
void h_31(double *state, double *unused, double *out_7582297473383129652) {
   out_7582297473383129652[0] = state[8];
}
void H_31(double *state, double *unused, double *out_2781224133293185658) {
   out_2781224133293185658[0] = 0;
   out_2781224133293185658[1] = 0;
   out_2781224133293185658[2] = 0;
   out_2781224133293185658[3] = 0;
   out_2781224133293185658[4] = 0;
   out_2781224133293185658[5] = 0;
   out_2781224133293185658[6] = 0;
   out_2781224133293185658[7] = 0;
   out_2781224133293185658[8] = 1;
}
#include <eigen3/Eigen/Dense>
#include <iostream>

typedef Eigen::Matrix<double, DIM, DIM, Eigen::RowMajor> DDM;
typedef Eigen::Matrix<double, EDIM, EDIM, Eigen::RowMajor> EEM;
typedef Eigen::Matrix<double, DIM, EDIM, Eigen::RowMajor> DEM;

void predict(double *in_x, double *in_P, double *in_Q, double dt) {
  typedef Eigen::Matrix<double, MEDIM, MEDIM, Eigen::RowMajor> RRM;

  double nx[DIM] = {0};
  double in_F[EDIM*EDIM] = {0};

  // functions from sympy
  f_fun(in_x, dt, nx);
  F_fun(in_x, dt, in_F);


  EEM F(in_F);
  EEM P(in_P);
  EEM Q(in_Q);

  RRM F_main = F.topLeftCorner(MEDIM, MEDIM);
  P.topLeftCorner(MEDIM, MEDIM) = (F_main * P.topLeftCorner(MEDIM, MEDIM)) * F_main.transpose();
  P.topRightCorner(MEDIM, EDIM - MEDIM) = F_main * P.topRightCorner(MEDIM, EDIM - MEDIM);
  P.bottomLeftCorner(EDIM - MEDIM, MEDIM) = P.bottomLeftCorner(EDIM - MEDIM, MEDIM) * F_main.transpose();

  P = P + dt*Q;

  // copy out state
  memcpy(in_x, nx, DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
}

// note: extra_args dim only correct when null space projecting
// otherwise 1
template <int ZDIM, int EADIM, bool MAHA_TEST>
void update(double *in_x, double *in_P, Hfun h_fun, Hfun H_fun, Hfun Hea_fun, double *in_z, double *in_R, double *in_ea, double MAHA_THRESHOLD) {
  typedef Eigen::Matrix<double, ZDIM, ZDIM, Eigen::RowMajor> ZZM;
  typedef Eigen::Matrix<double, ZDIM, DIM, Eigen::RowMajor> ZDM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, EDIM, Eigen::RowMajor> XEM;
  //typedef Eigen::Matrix<double, EDIM, ZDIM, Eigen::RowMajor> EZM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, 1> X1M;
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> XXM;

  double in_hx[ZDIM] = {0};
  double in_H[ZDIM * DIM] = {0};
  double in_H_mod[EDIM * DIM] = {0};
  double delta_x[EDIM] = {0};
  double x_new[DIM] = {0};


  // state x, P
  Eigen::Matrix<double, ZDIM, 1> z(in_z);
  EEM P(in_P);
  ZZM pre_R(in_R);

  // functions from sympy
  h_fun(in_x, in_ea, in_hx);
  H_fun(in_x, in_ea, in_H);
  ZDM pre_H(in_H);

  // get y (y = z - hx)
  Eigen::Matrix<double, ZDIM, 1> pre_y(in_hx); pre_y = z - pre_y;
  X1M y; XXM H; XXM R;
  if (Hea_fun){
    typedef Eigen::Matrix<double, ZDIM, EADIM, Eigen::RowMajor> ZAM;
    double in_Hea[ZDIM * EADIM] = {0};
    Hea_fun(in_x, in_ea, in_Hea);
    ZAM Hea(in_Hea);
    XXM A = Hea.transpose().fullPivLu().kernel();


    y = A.transpose() * pre_y;
    H = A.transpose() * pre_H;
    R = A.transpose() * pre_R * A;
  } else {
    y = pre_y;
    H = pre_H;
    R = pre_R;
  }
  // get modified H
  H_mod_fun(in_x, in_H_mod);
  DEM H_mod(in_H_mod);
  XEM H_err = H * H_mod;

  // Do mahalobis distance test
  if (MAHA_TEST){
    XXM a = (H_err * P * H_err.transpose() + R).inverse();
    double maha_dist = y.transpose() * a * y;
    if (maha_dist > MAHA_THRESHOLD){
      R = 1.0e16 * R;
    }
  }

  // Outlier resilient weighting
  double weight = 1;//(1.5)/(1 + y.squaredNorm()/R.sum());

  // kalman gains and I_KH
  XXM S = ((H_err * P) * H_err.transpose()) + R/weight;
  XEM KT = S.fullPivLu().solve(H_err * P.transpose());
  //EZM K = KT.transpose(); TODO: WHY DOES THIS NOT COMPILE?
  //EZM K = S.fullPivLu().solve(H_err * P.transpose()).transpose();
  //std::cout << "Here is the matrix rot:\n" << K << std::endl;
  EEM I_KH = Eigen::Matrix<double, EDIM, EDIM>::Identity() - (KT.transpose() * H_err);

  // update state by injecting dx
  Eigen::Matrix<double, EDIM, 1> dx(delta_x);
  dx  = (KT.transpose() * y);
  memcpy(delta_x, dx.data(), EDIM * sizeof(double));
  err_fun(in_x, delta_x, x_new);
  Eigen::Matrix<double, DIM, 1> x(x_new);

  // update cov
  P = ((I_KH * P) * I_KH.transpose()) + ((KT.transpose() * R) * KT);

  // copy out state
  memcpy(in_x, x.data(), DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
  memcpy(in_z, y.data(), y.rows() * sizeof(double));
}




}
extern "C" {

void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_25, H_25, NULL, in_z, in_R, in_ea, MAHA_THRESH_25);
}
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<2, 3, 0>(in_x, in_P, h_24, H_24, NULL, in_z, in_R, in_ea, MAHA_THRESH_24);
}
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_30, H_30, NULL, in_z, in_R, in_ea, MAHA_THRESH_30);
}
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_26, H_26, NULL, in_z, in_R, in_ea, MAHA_THRESH_26);
}
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_27, H_27, NULL, in_z, in_R, in_ea, MAHA_THRESH_27);
}
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_29, H_29, NULL, in_z, in_R, in_ea, MAHA_THRESH_29);
}
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_28, H_28, NULL, in_z, in_R, in_ea, MAHA_THRESH_28);
}
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_31, H_31, NULL, in_z, in_R, in_ea, MAHA_THRESH_31);
}
void car_err_fun(double *nom_x, double *delta_x, double *out_6723881171663706926) {
  err_fun(nom_x, delta_x, out_6723881171663706926);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_2739404126537862093) {
  inv_err_fun(nom_x, true_x, out_2739404126537862093);
}
void car_H_mod_fun(double *state, double *out_8598021807935865210) {
  H_mod_fun(state, out_8598021807935865210);
}
void car_f_fun(double *state, double dt, double *out_8990824843719242692) {
  f_fun(state,  dt, out_8990824843719242692);
}
void car_F_fun(double *state, double dt, double *out_2803601295947585167) {
  F_fun(state,  dt, out_2803601295947585167);
}
void car_h_25(double *state, double *unused, double *out_1726729315642570782) {
  h_25(state, unused, out_1726729315642570782);
}
void car_H_25(double *state, double *unused, double *out_2811870095170146086) {
  H_25(state, unused, out_2811870095170146086);
}
void car_h_24(double *state, double *unused, double *out_8285531725452842947) {
  h_24(state, unused, out_8285531725452842947);
}
void car_H_24(double *state, double *unused, double *out_7632191599826134349) {
  H_24(state, unused, out_7632191599826134349);
}
void car_h_30(double *state, double *unused, double *out_4713285606981469022) {
  h_30(state, unused, out_4713285606981469022);
}
void car_H_30(double *state, double *unused, double *out_2941209042313386156) {
  H_30(state, unused, out_2941209042313386156);
}
void car_h_26(double *state, double *unused, double *out_3353658075972099194) {
  h_26(state, unused, out_3353658075972099194);
}
void car_H_26(double *state, double *unused, double *out_6553373414044202310) {
  H_26(state, unused, out_6553373414044202310);
}
void car_h_27(double *state, double *unused, double *out_6678927504001655871) {
  h_27(state, unused, out_6678927504001655871);
}
void car_H_27(double *state, double *unused, double *out_5115972354113811067) {
  H_27(state, unused, out_5115972354113811067);
}
void car_h_29(double *state, double *unused, double *out_8080891962152112839) {
  h_29(state, unused, out_8080891962152112839);
}
void car_H_29(double *state, double *unused, double *out_2430977697998993972) {
  H_29(state, unused, out_2430977697998993972);
}
void car_h_28(double *state, double *unused, double *out_1677254236340810226) {
  h_28(state, unused, out_1677254236340810226);
}
void car_H_28(double *state, double *unused, double *out_7513376715068524546) {
  H_28(state, unused, out_7513376715068524546);
}
void car_h_31(double *state, double *unused, double *out_7582297473383129652) {
  h_31(state, unused, out_7582297473383129652);
}
void car_H_31(double *state, double *unused, double *out_2781224133293185658) {
  H_31(state, unused, out_2781224133293185658);
}
void car_predict(double *in_x, double *in_P, double *in_Q, double dt) {
  predict(in_x, in_P, in_Q, dt);
}
void car_set_mass(double x) {
  set_mass(x);
}
void car_set_rotational_inertia(double x) {
  set_rotational_inertia(x);
}
void car_set_center_to_front(double x) {
  set_center_to_front(x);
}
void car_set_center_to_rear(double x) {
  set_center_to_rear(x);
}
void car_set_stiffness_front(double x) {
  set_stiffness_front(x);
}
void car_set_stiffness_rear(double x) {
  set_stiffness_rear(x);
}
}

const EKF car = {
  .name = "car",
  .kinds = { 25, 24, 30, 26, 27, 29, 28, 31 },
  .feature_kinds = {  },
  .f_fun = car_f_fun,
  .F_fun = car_F_fun,
  .err_fun = car_err_fun,
  .inv_err_fun = car_inv_err_fun,
  .H_mod_fun = car_H_mod_fun,
  .predict = car_predict,
  .hs = {
    { 25, car_h_25 },
    { 24, car_h_24 },
    { 30, car_h_30 },
    { 26, car_h_26 },
    { 27, car_h_27 },
    { 29, car_h_29 },
    { 28, car_h_28 },
    { 31, car_h_31 },
  },
  .Hs = {
    { 25, car_H_25 },
    { 24, car_H_24 },
    { 30, car_H_30 },
    { 26, car_H_26 },
    { 27, car_H_27 },
    { 29, car_H_29 },
    { 28, car_H_28 },
    { 31, car_H_31 },
  },
  .updates = {
    { 25, car_update_25 },
    { 24, car_update_24 },
    { 30, car_update_30 },
    { 26, car_update_26 },
    { 27, car_update_27 },
    { 29, car_update_29 },
    { 28, car_update_28 },
    { 31, car_update_31 },
  },
  .Hes = {
  },
  .sets = {
    { "mass", car_set_mass },
    { "rotational_inertia", car_set_rotational_inertia },
    { "center_to_front", car_set_center_to_front },
    { "center_to_rear", car_set_center_to_rear },
    { "stiffness_front", car_set_stiffness_front },
    { "stiffness_rear", car_set_stiffness_rear },
  },
  .extra_routines = {
  },
};

ekf_lib_init(car)
