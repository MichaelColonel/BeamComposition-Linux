/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include <TH1.h>
#include <TH2.h>
#include <QSettings>
#include <QString>
#include <QDebug>

#include <numeric>
#include <iostream>

#include "ccmath_lib.h"

#include "channelschargefit.h"
#include "channelscountsfit.h"

#define SIZE ((CHANNELS) * 2 + 1)
//#define N 13
#define N 9

namespace {
/*
const struct {
    double x;
    double channel1[2]; // mean, sigma pair
    double channel2[2]; // mean, sigma pair
    double channel3[2]; // mean, sigma pair
    double channel4[2]; // mean, sigma pair
} data[N] = {
    {   0.0,   33.8,   4.2,   71.2,  4.4,   11.5,  4.7,   43.2,  4.6 },
    {   3.0,   87.2,   4.5,  135.8,  5.2,  113.9,  5.6,  122.0,  5.1 },
    {   5.0,  123.6,   4.0,  173.0,  4.9,  156.2,  5.0,  165.9,  4.9 },
    {  10.0,  203.0,   4.3,  254.5,  4.9,  249.3,  5.0,  263.6,  5.0 },
    {  20.0,  356.8,   5.1,  414.1,  5.8,  430.4,  5.7,  451.3,  5.9 },
    {  40.0,  669.1,   7.2,  736.0,  7.5,  793.4,  8.1,  830.1,  8.1 },
    {  50.0,  843.1,   8.6,  899.7,  8.7,  969.5,  9.3, 1014.0,  9.2 },
    {  80.0, 1381.0,  16.8, 1357.0, 15.0, 1428.0, 17.0, 1488.0, 14.3 },
    { 100.0, 1700.0,  18.3, 1620.0, 16.4, 1678.0, 18.0, 1747.0, 15.6 },
    { 150.0, 2384.0,  21.1, 2189.0, 21.6, 2205.0, 22.7, 2286.0, 20.3 },
    { 200.0, 2948.0,  25.0, 2656.0, 27.0, 2634.0, 26.7, 2709.0, 23.5 },
    { 220.0, 3115.0,  25.6, 2783.0, 29.3, 2735.0, 26.8, 2797.0, 22.7 },
    { 250.0, 3302.0, 25.75, 2914.0, 32.8, 2809.0, 26.8, 2852.0, 21.3 }
};
*/

struct ReferenceCountsSignals {
    double x;
    double channel1[2]; // mean, sigma pair
    double channel2[2]; // mean, sigma pair
    double channel3[2]; // mean, sigma pair
    double channel4[2]; // mean, sigma pair
};
/*
const ReferenceCountsSignals reference_signal_counts_data[N] = {
    {   0.0,   48.3,   2.9,  111.6,  3.0,   97.6,  3.1,   96.4,  2.5 },
    {  10.0,  149.1,   3.1,  209.2,  3.3,  200.9,  3.2,  203.3,  3.3 },
    {  15.0,  196.8,   3.2,  260.0,  3.4,  254.9,  3.3,  261.6,  3.5 },
    {  20.0,  243.6,   3.4,  309.6,  3.7,  307.2,  3.5,  314.2,  3.7 },
    {  50.0,  523.1,   4.9,  601.0,  5.4,  616.3,  5.1,  628.7,  5.7 },
    { 100.0, 1012.0,   8.9, 1090.0,  8.8, 1128.0,  8.7, 1160.0,  9.3 },
    { 150.0, 1516.0,  12.4, 1544.0, 11.7, 1592.0, 11.4, 1672.0, 12.4 },
    { 200.0, 1970.0,  15.4, 1939.7, 14.3, 1991.0, 13.7, 2123.0, 15.1 },
    { 250.0, 2371.0,  18.3, 2290.7, 16.7, 2346.0, 15.9, 2515.0, 17.5 },
    { 300.0, 2731.0,  21.2, 2608.7, 19.1, 2669.0, 18.1, 2879.0, 20.1 },
    { 350.0, 3052.0,  23.6, 2893.7, 20.7, 2955.0, 19.2, 3151.0, 21.3 },
    { 400.0, 3316.0,  24.8, 3107.0, 20.6, 3165.0, 18.8, 3451.0, 21.6 },
    { 450.0, 3517.0,  25.3, 3253.0, 20.4, 3312.0, 18.6, 3593.0, 20.8 }
};
*/

// 25 m trigger, 37 m signal, 24 delay (pi+ calib -- 24-25.11.2016) (detector output 1,2,4,3) N=13
/*
const ReferenceCountsSignals reference_signal_counts_data[N] = {
    {   0.0,   34.0,   2.8,   72.8,  2.4,   80.6,  2.9,   80.5,  3.0 },
    {  10.0,  123.2,   3.0,  143.4,  2.6,  167.5,  3.0,  172.1,  3.2 },
    {  15.0,  169.6,   3.0,  179.6,  2.6,  214.9,  3.0,  221.8,  3.3 },
    {  50.0,  477.2,   4.5,  418.5,  3.8,  529.2,  4.9,  547.6,  4.9 },
    { 100.0,  913.1,   7.5,  752.0,  5.9,  964.9,  7.4, 1003.0,  7.8 },
    { 150.0, 1325.0,  10.1, 1050.0,  7.7, 1347.0,  9.7, 1404.0, 10.1 },
    { 200.0, 1681.0,  12.5, 1305.0,  9.5, 1670.0, 11.8, 1743.0, 12.3 },
    { 220.0, 1803.0,  13.3, 1393.0, 10.1, 1781.0, 12.6, 1856.0, 13.0 },
    { 250.0, 1993.0,  14.5, 1531.0, 10.9, 1954.0, 13.6, 2039.0, 14.1 },
    { 300.0, 2259.0,  16.5, 1727.0, 12.3, 2199.0, 15.2, 2294.0, 15.7 },
    { 350.0, 2494.0,  17.6, 1902.0, 12.9, 2415.0, 15.9, 2520.0, 16.4 },
    { 400.0, 2673.0,  17.7, 2028.0, 12.7, 2569.0, 15.5, 2685.0, 16.2 },
    { 450.0, 2782.0,  17.4, 2099.0, 12.1, 2650.0, 15.0, 2775.0, 15.6 },
};
*/
/*
// 25 m trigger, 37 m signal, 24 delay (30.11.2016 detector output 2,1,3,4) N=15
const ReferenceCountsSignals reference_signal_counts_data[N] = {
    {   0.0,   80.9,  2.6,   28.8,   2.2,  79.0,   2.7,   82.2,  2.8 },
    {   5.0,  114.9,  3.2,   56.5,   3.5, 114.9,   3.4,  118.7,  3.7 },
    {   8.0,  148.9,  3.8,   93.7,   4.9, 145.5,   4.5,  153.9,  5.9 },
    {  10.0,  164.9,  2.8,  105.2,   2.3, 170.9,   2.8,  177.7,  3.0 },
    {  15.0,  208.1,  2.9,  142.2,   2.4, 217.8,   2.9,  225.4,  3.1 },
    {  50.0,  499.7,  4.5,  390.9,   3.8, 534.5,   4.4,  544.3,  4.7 },
    { 100.0,  909.5,  7.3,  738.0,   5.8, 976.9,   7.3,  989.6,  7.5 },
    { 150.0, 1288.0,  9.8, 1055.0,   7.8, 1381.0,  9.8, 1381.0,  9.8 },
    { 200.0, 1614.0, 11.9, 1324.0,   9.5, 1729.0, 11.7, 1713.0, 11.7 },
    { 220.0, 1735.0, 12.8, 1425.0,  10.2, 1864.0, 12.6, 1841.0, 12.4 },
    { 250.0, 1903.0, 14.0, 1558.0,  11.1, 2034.0, 14.0, 2002.0, 13.5 },
    { 300.0, 2161.0, 15.8, 1762.0,  12.6, 2304.0, 18.5, 2258.0, 15.0 },
    { 350.0, 2382.0, 16.5, 1929.0,  13.0, 2525.0, 24.0, 2471.0, 15.2 },
    { 400.0, 2538.0, 16.8, 2038.0,  13.1, 2661.0, 24.0, 2609.0, 14.9 },
    { 450.0, 2636.0, 16.0, 2098.0,  12.5, 2735.0, 15.3, 2685.0, 14.1 },
};
*/
/*
// 25 m trigger, 37 m signal, 24 delay, 100 ns GATE (01.12.2016) N=15
const ReferenceCountsSignals reference_signal_counts_data[N] = {
    {   0.0,   35.3,  3.2,   32.7,   2.8,  127.0,  3.0,   79.5,  3.2 },
//    {   5.0,   70.3,  3.7,   58.3,   4.0,  159.9,  4.4,  117.0,  4.1 },
//    {   8.0,  107.1,  4.0,   95.4,   5.2,  188.9,  4.5,  151.5,  6.2 },
    {  10.0,  125.0,  3.4,  107.5,   3.1,  211.8,  3.3,  175.1,  3.4 },
    {  15.0,  171.6,  3.5,  144.9,   3.1,  255.0,  3.3,  221.9,  3.5 },
    {  50.0,  486.6,  5.1,  396.7,   4.2,  545.9,  4.6,  533.5,  5.0 },
    { 100.0,  925.5,  8.1,  745.5,   6.3,  956.2,  7.5,  976.9,  8.0 },
    { 150.0, 1338.0, 10.5, 1053.0,   8.0, 1338.0, 10.0, 1389.0, 10.5 },
    { 200.0, 1696.0, 12.6, 1318.0,   9.5, 1677.0, 12.1, 1748.0, 12.8 },
    { 220.0, 1825.0, 13.3, 1413.0,  10.0, 1799.0, 13.0, 1875.0, 13.6 },
    { 250.0, 2007.0, 14.5, 1546.0,  10.8, 1970.0, 14.2, 2054.0, 14.8 },
    { 300.0, 2286.0, 16.3, 1749.0,  12.2, 2233.0, 16.1, 2320.0, 16.8 },
    { 350.0, 2521.0, 16.9, 1914.0,  12.1, 2456.0, 18.3, 2544.0, 17.6 },
    { 400.0, 2690.0, 16.7, 2022.0,  11.6, 2608.0, 24.4, 2695.0, 17.4 },
    { 450.0, 2804.0, 16.8, 2084.0,  11.0, 2689.0, 27.3, 2791.0, 16.9 }
};
*/

// New detector 100 ns GATE (20.11.2019) N=9
const ReferenceCountsSignals reference_signal_counts_data[N] = {
    {   0.0,   48.8,  7.4,  69.89,   7.0,   47.8,  7.0,   65.9,  8.0 },
    {   5.0,  139.0,  3.4,  160.2,   6.7,  134.6,  6.6,  160.6, 11.3 },
    {  10.0,  228.6,  8.0,  250.2,   6.7,  221.1,  7.1,  255.1,  8.8 },
    {  50.0,  954.3,  8.8,  979.5,   6.8,  920.1,  7.0, 1023.0, 11.2 },
    { 100.0, 1871.0,  8.1, 1896.0,   7.1, 1796.0,  6.4, 1993.0, 11.2 },
    { 150.0, 2786.0,  8.7, 2811.0,   7.1, 2669.0,  6.7, 2967.0, 11.9 },
    { 200.0, 3731.0,  8.5, 3752.0,   7.3, 3563.0,  7.4, 3971.0, 12.5 },
    { 210.0, 3923.0, 10.4, 3944.0,   6.8, 3745.0,  8.2, 4171.8, 13.6 },
    { 220.0, 4115.0, 11.4, 4136.0,   7.8, 3927.0,  8.2, 4372.6, 14.1 }
};

/*
const struct ChargeCountsSignals {
    int z;
    double mean;
    double sigma;
} charge_counts_data[CARBON_Z] = {
    {  1,   53.7,   30.0 },
    {  2,  180.0,   80.2 },
    {  3,  390.0,   100.4 },
    {  4,  800.0,   100.9 },
    {  5, 1300.0,   120.9 },
    {  6, 2000.0,   200.4 }
};
*/

const struct ChargeCountsSignals {
    int z;
    double mean;
    double sigma;
} charge_counts_data[CARBON_Z] = {
    {  1,   3.59,   0.27 },
    {  2,     0.,   0. },
    {  3,     0.,   0. },
    {  4,     0.,   0. },
    {  5,     0.,   0. },
    {  6,  216.4,  18.42 }
};

double* fit_data[SIZE] = {};

double
ccmath_splfit( double w, double *x, double *y, double *z, int m, double tn)
{
    return (tn != 0.) ? ccmath_tnsfit( w, x, y, z, m, tn) : ccmath_csfit( w, x, y, z, m);
}
/*
const double mc2e = 511000.; // eV
const double Imean = 173.; // eV

double
correction(double beta)
{
    double gamma2 = 1. / (1. - beta * beta);
    double k = (2. * mc2e * beta * beta * gamma2) / Imean;
    double res = (::log(k) - beta * beta) / (beta * beta);
    return res;
}
*/
/*
const double mc2e = 510998.91844E-09; // GeV
const double Imean = 173.0E-09; // GeV

const double projmass[CARBON_Z] = { 0.93827231, 3.727, 6.534, 8.393, 10.25, 11.17 }; // GeV

double tmax( double beta2, double gamma, double gamma2, double prom)
{
    double t = mc2e / prom;
    double part2 = 1. + (2. * gamma * t) + t * t;
    double part1 = 2. * mc2e * beta2 * gamma2;
    return (part1 / part2);
}

double
correction( double beta, double projm)
{
    double beta2 = beta * beta;
    double gamma2 = 1. / (1. - beta2);
    double gamma = sqrt(gamma2);
    double tm = tmax( beta2, gamma, gamma2, projm);
    double k = (2. * mc2e * beta2 * gamma2 * tm) / (Imean * Imean);
    double res = (0.5 * ::log(k) - beta2) / (beta2);
    return res;
}
*/

const double mc2e = 510998.91844E-09; // GeV
const double Imean = 173.0E-09; // GeV
const double hwp = 31.05E-09; // GeV

const double projmass[6] = { 0.93827231, 3.727, 6.534, 8.393, 10.25, 11.17 }; // GeV

const double cmean = 2. * log(173. / 31.05) + 1.;
const double k = 3.;
const double x0 = 3.;

double tmax( double beta2, double gamma_, double gamma2, double prom)
{
    double t = mc2e / prom;
    double part2 = 1. + (2. * gamma_ * t) + t * t;
    double part1 = 2. * mc2e * beta2 * gamma2;
    return (part1 / part2);
}

double
xparam( double beta, double gamma)
{
    return log10(beta * gamma);
}

double
x1()
{
    return (cmean < 5.215) ? 0.2 : (0.325 * cmean) - 1.5;
}

double
delta( double x_, double x0_, double x1_, double k_) {
    double a = (cmean - 2. * log(10.) * x0_) / pow( (x1_ - x0_), 3.);

    double res = 0.;
    if (x_ >= x1_) {
        res = (2. * log(10.) * x_ - cmean);
    }
    else if ((x_ >= x0_) && (x_ < x1_)) {
        res = (2. * log(10.) * x_ - cmean + a * pow((x1_ - x_), k_));
    }
    else if (x_ < x0_) {
        res = 0.0;
    }
    return res;
}

double
correction( double beta, double projm)
{
    double beta2 = beta * beta;
    double gamma2 = 1. / (1. - beta2);
    double gamma = sqrt(gamma2);

    double tm = tmax( beta2, gamma, gamma2, projm);
    double k = (2. * mc2e * beta2 * gamma2 * tm) / (Imean * Imean);
//    double res = (0.5 * ::log(k) - beta2) / (beta2);
    double x_ = xparam( beta, gamma);
    double x1_ = x1();
    double delta_ = delta( x_, x0, x1_, k);
    double p = (tm * tm) / (gamma * projm * gamma * projm);
    double res = (0.5 * ::log(k) - beta2 - delta_ / 2. + p / 8.) / (beta2);
    return res;
}

//const std::array< double, CHANNELS> channel_amp = { 1.0, 1.003715745, 0.955349248, 1.025628856 };
//const std::array< double, CHANNELS> channel_amp = { 216.0 / 3087.96, 1.5 * 216.0 / 2660.16, 1.5 * 216.0 / 3152.59, 1.5 * 216.0 / 3724 };

//const std::array< double, CHANNELS> channel_amp = { 1., 75.68 / 74.60, 75.68 / 78.40, 75.68 / 89.02 }; // self trigger
const std::array< double, CHANNELS> channel_amp = { 1., 169.238 / 100.54, 169.238 / 122.02, 169.238 / 108.47 }; // ext trigger
//const std::array< double, CHANNELS> channel_amp = { 1., 18.4 / 18.44, 18.4 / 17.57, 18.4 / 19.46 };

//size_t accepted[6] = {};
//size_t rejected[6] = {};

//const double aa = 6.240625;
//const double bb = -6.3625;

} // namespace


namespace CalibrationFitting {

SharedParameters Parameters::instance_;

Parameters::Parameters(QSettings* settings)
    :
    k(0.5),
    K(1.0),
    reference_charge(1),
    projectile_charge(CARBON_Z),
    flag_process_data(true)
{
    std::fill( charge_radius, charge_radius + CARBON_Z, 0.7);
    std::fill( charge_beta, charge_beta + CARBON_Z, 0.739);
    charge_beta[0] = 0.9999;
    charge_beta[CARBON_Z - 1] = 0.72;

    initiate(settings);
}

Parameters::~Parameters()
{
    for ( size_t i = 0; i < SIZE; ++i) {
        if (fit_data[i]) delete [] fit_data[i];
    }
}

SharedParameters
Parameters::instance(QSettings* settings)
{
    if (!instance_) {
        instance_ = SharedParameters(new Parameters(settings));
    }
    return instance_;
}

void
Parameters::initiate(QSettings *set)
{
    // reference signal to counts map
    if (set) {
        set->beginGroup("SignalReferenceMap");
        int size = set->value( "size", 0).toInt();
        if (size) {
            reference_counts_signals.clear();
            for ( int i = 0; i < size; ++i) {
                set->beginGroup(QString("key%1").arg(i));
                double key = set->value( "key", 0.0).toDouble();
                SignalArray array;
                set->beginReadArray("mu-sigma-values");
                for ( size_t j = 0; j < array.size(); ++j) {
                    set->setArrayIndex(j);
                    double mu = set->value( "mu", 0.0).toDouble();
                    double sigma = set->value( "sigma", 0.0).toDouble();
                    array[j] = SignalPair( mu, sigma);
                }
                set->endArray();
                set->endGroup();
                reference_counts_signals.insert(std::make_pair( key, array));
            }
        }
        else {
            restore_reference_signals();
        }
        set->endGroup();
    }
    else {
        restore_reference_signals();
    }

    // particle charge to counts map
    if (set) {
        charge_counts_signals.clear();
        set->beginGroup("ChargeSignalMap");
        for ( int i = 1; i <= CARBON_Z; ++i) {
            const int& z = charge_counts_data[i - 1].z;
            const double& m = charge_counts_data[i - 1].mean;
            const double& s = charge_counts_data[i - 1].sigma;

            set->beginGroup(QString("charge%1").arg(i));
            int key = set->value( "charge", z).toInt();
            double mu = set->value( "mu", m).toDouble();
            double sigma = set->value( "sigma", s).toDouble();
            set->endGroup();
            charge_counts_signals.insert(std::make_pair( key, SignalPair( mu, sigma)));
        }
        set->endGroup();
    }
    else {
        restore_charge_signals();
    }

    // background
    if (set) {
        set->beginGroup("Background");
        set->beginReadArray("mu-sigma-values");
        for ( size_t i = 0; i < CHANNELS; ++i) {
            set->setArrayIndex(i);
            double mu = set->value( "mu", 0.0).toDouble();
            double sigma = set->value( "sigma", 0.0).toDouble();
            background_signals[i] = SignalPair( mu, sigma);
        }
        set->endArray();
        set->endGroup();
    }

    // linear fit parameters
/*
    int ch = 0, pos1 = -1, pos2 = -1;
    if (set) {
        set->beginGroup("LinearFit");
        ch = set->value( "reference-channel", 0).toInt();
        pos1 = set->value( "reference-pos1", -1).toInt();
        pos2 = set->value( "reference-pos2", -1).toInt();
        linear_fit[0] = set->value( "parameter-a", 0.0).toDouble();
        linear_fit[1] = set->value( "parameter-b", 0.0).toDouble();
        set->endGroup();
    }
*/
    // charge-radius
    if (set) {
        set->beginGroup("ChargeRadius");
        set->beginReadArray("values");
        for ( int i = 0; i < CARBON_Z; ++i) {
            set->setArrayIndex(i + 1);
            double value = set->value( "value", 1.0).toDouble();
            charge_radius[i] = value;
        }
        set->endArray();
        set->endGroup();
    }

    // charge-beta
    if (set) {
        set->beginGroup("ChargeBeta");
        set->beginReadArray("values");
        for ( int i = 0; i < CARBON_Z; ++i) {
            set->setArrayIndex(i + 1);
            double value = set->value( "value", 0.7).toDouble();
            charge_beta[i] = value;
        }
        set->endArray();
        set->endGroup();
    }

    // charge-in-fit
    if (set) {
        set->beginGroup("ChargeFit");
        set->beginReadArray("values");
        bool* fit_charge = CalibrationFitting::BeamCompositionFit::charge_in_fit();
        for ( int i = 0; i < CARBON_Z; ++i) {
            set->setArrayIndex(i + 1);
            bool value = set->value( "value", true).toBool();
            fit_charge[i] = value;
        }
        set->endArray();
        set->endGroup();
    }

/*
    if (ch > 0 && pos1 != -1 && pos2 != -1) {
        recalculate( ch, pos1, pos2);
    }
    else if (ch == 0 || pos1 == -1 || pos2 == -1) {
        recalculate( 4, 0, 3);
    }
    else if (linear_fit[0] == 0.0 && linear_fit[1] == 0.0) {
        recalculate();
    }
    else
        recalculate( 4, 0, 3);
*/
    // tension
    tension_parameter = set->value( "tension-parameter", 0.0).toDouble();

    // reference charge
    reference_charge = set->value( "reference-charge", 1).toInt();
    projectile_charge = set->value( "projectile-charge", CARBON_Z).toInt();


    recalculate();
    recalculate_charge_fit(projectile_charge);
}

/*
void
Parameters::recalculate(int ref_channel, int fit_pos_start, int fit_pos_stop)
{
    reference_channel = ref_channel;
    fit_points[0] = fit_pos_start;
    fit_points[1] = fit_pos_stop;
    recalculate();
}
*/
void
Parameters::restore_reference_signals()
{
    reference_counts_signals.clear();

    const ReferenceCountsSignals* data = reference_signal_counts_data;

    for ( int i = 0; i < N; ++i) {
        SignalPair ch1( data[i].channel1[0], data[i].channel1[1]);
        SignalPair ch2( data[i].channel2[0], data[i].channel2[1]);
        SignalPair ch3( data[i].channel3[0], data[i].channel3[1]);
        SignalPair ch4( data[i].channel4[0], data[i].channel4[1]);
#if defined(_MSC_VER) && (_MSC_VER < 1900)
        SignalArray array;
        array[0] = ch1;
        array[1] = ch2;
        array[2] = ch3;
        array[3] = ch4;
#elif defined(__GNUG__) && (__cplusplus >= 201103L)
        SignalArray array({ ch1, ch2, ch3, ch4 });
#endif
        reference_counts_signals.insert(std::make_pair( data[i].x, array));
    }
}

void
Parameters::restore_charge_signals()
{
    charge_counts_signals.clear();

    const ChargeCountsSignals* data = charge_counts_data;

    for ( int i = 0; i < CARBON_Z; ++i) {
        SignalPair signal( data[i].mean, data[i].sigma);
        charge_counts_signals.insert(std::make_pair( data[i].z, signal));
    }
}

void
Parameters::recalculate()
{
    size_t n = reference_counts_signals.size();

    for ( size_t i = 0; i < SIZE; ++i) {
        if (fit_data[i]) {
            delete [] fit_data[i];
            fit_data[i] = nullptr;
        }
        fit_data[i] = new double[n];
    }

    double* x = fit_data[0];
    double* y1 = fit_data[1];
    double* p1 = fit_data[2];
    double* y2 = fit_data[3];
    double* p2 = fit_data[4];
    double* y3 = fit_data[5];
    double* p3 = fit_data[6];
    double* y4 = fit_data[7];
    double* p4 = fit_data[8];

    ReferenceSignalMap::const_iterator iter = reference_counts_signals.begin();
    const SignalArray& ref_back = iter->second;

    size_t i = 0;
#if defined(_MSC_VER) && (_MSC_VER < 1900)
    for ( ReferenceSignalMap::const_iterator it = reference_counts_signals.begin();
          it != reference_counts_signals.end(); ++it) {
        x[i] = it->first;
        const SignalArray& array = it->second;
#elif defined(__GNUG__) && (__cplusplus >= 201103L)
    for ( const auto& elem : reference_counts_signals) {
        x[i] = elem.first;
        const SignalArray& array = elem.second;
#endif
        y1[i] = array[0].first - ref_back[0].first;
        y2[i] = array[1].first - ref_back[1].first;
        y3[i] = array[2].first - ref_back[2].first;
        y4[i] = array[3].first - ref_back[3].first;
        ++i;
    }

    size_t fitn = n - 1;

    ccmath_cspl( y1, x, p1, fitn, tension_parameter);
    ccmath_cspl( y2, x, p2, fitn, tension_parameter);
    ccmath_cspl( y3, x, p3, fitn, tension_parameter);
    ccmath_cspl( y4, x, p4, fitn, tension_parameter);
/*
    double* ylin = y4;
    switch (reference_channel) {
    case 1:
        ylin = y1;
        break;
    case 2:
        ylin = y2;
        break;
    case 3:
        ylin = y3;
        break;
    case 4:
    default:
        ylin = y4;
        break;
    }

    int& pos1 = fit_points[0];
    int& pos2 = fit_points[1];
    double a = (ylin[pos2] - ylin[pos1]) / (x[pos2] - x[pos1]);
    double b = ylin[pos2] - a * x[pos2];
    linear_fit[0] = a;
    linear_fit[1] = b;
*/
}

RunInfo
Parameters::fit( const CountsList& list, Diagrams& d, bool flag_background)
{
    size_t n = reference_counts_signals.size();
    int fitn = n - 1;
    double* x = fit_data[0];
    double* y1 = fit_data[1];
    double* p1 = fit_data[2];
    double* y2 = fit_data[3];
    double* p2 = fit_data[4];
    double* y3 = fit_data[5];
    double* p3 = fit_data[6];
    double* y4 = fit_data[7];
    double* p4 = fit_data[8];

    size_t events_counted = list.size();
    size_t events_processed = 0;

    RunInfo::ChargeEventArray charge_events;
    std::fill( charge_events.begin(), charge_events.end(), 0);

    const SignalArray& back = background_signals;

    double* pp[CHANNELS] = { p1, p2, p3, p4 };
    double* yy[CHANNELS] = { y1, y2, y3, y4 };

    const SignalPair& ref_charge = charge_counts_signals[reference_charge];

//    ChargeSignalMap::const_iterator iter = charge_counts_signals.begin();
//    const SignalPair& mip_charge = iter->second;

    double signal = ref_charge.first;
    double beta = charge_beta[reference_charge - 1];
    double kpower = 1 / k;

    if (flag_process_data) {
#if defined(_MSC_VER) && (_MSC_VER < 1900)
        for ( CountsList::const_iterator it = list.begin(); it != list.end(); ++it) {
            const CountsArray& array = *it;
#elif defined(__GNUG__) && (__cplusplus >= 201103L)
        for ( const CountsArray& array : list) {
#endif
            ChannelsArray values;

            std::copy( array.begin(), array.end(), values.begin());

            d.channels[0]->Fill(array[0]);
            d.channels[1]->Fill(array[1]);
            d.channels[2]->Fill(array[2]);
            d.channels[3]->Fill(array[3]);

            bool skip = false;
            // if background flag, then clear all diagrams except channels data
/*
            if (flag_background) {
                for ( int i = 0; i < CHANNELS; ++i) {
                    d.fit[i]->Reset();
                    d.rank[i]->Reset();
                }
                d.fitall->Reset();
                d.fit_median->Reset();
                d.fit_mean->Reset();

                d.z12->Reset();
                d.z23->Reset();
                d.z34->Reset();
                d.z14->Reset();
                d.z13->Reset();
                d.z24->Reset();

                d.c12->Reset();
                d.c23->Reset();
                d.c34->Reset();
                d.c14->Reset();
                d.c13->Reset();
                d.c24->Reset();

                d.z->Reset();
                d.z2->Reset();
            }
*/
            if (!flag_background) {
                // add correct reference pedestals offset
                for ( int i = 0; i < CHANNELS; ++i) {
                    if (values[i] == 0 || values[i] >= 4095)
                        skip = true;

                    values[i] -= back[i].first;
                    if (values[i] <= 0.)
                        skip = true;
                }
                if (skip)
                    continue;

                skip = false;
                for ( int i = 0; i < CHANNELS; ++i) {
//                    values[i] = channel_amp[i] * values[i];
                    values[i] = channel_amp[i] * ccmath_splfit( values[i], yy[i], x, pp[i], fitn, tension_parameter);
//                    values[i] = splfit( values[i], yy[i], x, pp[i], fitn, tension_parameter);
                    if (values[i] <= 0.)
                        skip = true;
                }
                if (skip)
                    continue;
            }
            else {
                d.c12->Fill( values[0], values[1]);
                d.c23->Fill( values[1], values[2]);
                d.c34->Fill( values[2], values[3]);
                d.c14->Fill( values[0], values[3]);
                d.c13->Fill( values[0], values[2]);
                d.c24->Fill( values[1], values[3]);
            }
            // if it's not a background measurement and signals in channels
            // are higher than background then calculate the charge
            if (!flag_background) {

                ChannelsArray charge;
                int z = counts_to_charge( values, charge, signal, beta, kpower);

                for ( int i = 0; i < CHANNELS; ++i) {
                    d.fit[i]->Fill(values[i]);
                    d.fitall->Fill(values[i]);
                }

                ChannelsArray rank(values);
                std::sort( rank.begin(), rank.end());
                double mean = std::accumulate( values.begin(), values.end(), 0.) / CHANNELS;
                double median = (rank[1] + rank[2]) / 2.;

                for ( int i = 0; i < CHANNELS; ++i) {
                    d.rank[i]->Fill(values[i]);
                }

                d.fit_median->Fill(median);
                d.fit_mean->Fill(mean);

                d.c12->Fill( values[0], values[1]);
                d.c23->Fill( values[1], values[2]);
                d.c34->Fill( values[2], values[3]);
                d.c14->Fill( values[0], values[3]);
                d.c13->Fill( values[0], values[2]);
                d.c24->Fill( values[1], values[3]);

                if (z > 0) {
                    size_t& charge_event = charge_events[z - 1];
                    d.z12->Fill( charge[0], charge[1]);
                    d.z23->Fill( charge[1], charge[2]);
                    d.z34->Fill( charge[2], charge[3]);
                    d.z14->Fill( charge[0], charge[3]);
                    d.z13->Fill( charge[0], charge[2]);
                    d.z24->Fill( charge[1], charge[3]);

                    charge_event++; // increase a number of proccessed events for particular charge
                    events_processed++; // increase a number of all proccessed events

                    ChannelsArray tmp(charge);
                    std::sort( tmp.begin(), tmp.end());

                    double charge_rank2 = (tmp[1] + tmp[2]) / 2.;
                    d.z->Fill(charge_rank2); // rank 2
                    d.z2->Fill(charge_rank2 * charge_rank2);

//                   double mean_change = (charge[1] + charge[2]) / 2.;
//                   d.z->Fill(mean_change); // mean change
//                   d.z2->Fill(mean_change * mean_change);

//                   double mean_change = std::accumulate( charge.begin(), charge.end(), 0.0) / double(CHANNELS);
//                   d.z->Fill(mean_change); // mean change
//                   d.z2->Fill(mean_change * mean_change);

                }
            }
        }
    }
    return RunInfo( events_counted, events_processed, charge_events);
}

void
Parameters::save(QSettings *set)
{
    // reference signal to counts map
    set->beginGroup("SignalReferenceMap");

    set->setValue( "size", int(reference_counts_signals.size()));

    int row = 0;
#if defined(_MSC_VER) && (_MSC_VER < 1900)
    for ( ReferenceSignalMap::const_iterator it = reference_counts_signals.begin();
          it != reference_counts_signals.end(); ++it) {
        double key = it->first;
#elif defined(__GNUG__) && (__cplusplus >= 201103L)
    for ( const auto& elem : reference_counts_signals) {
        double key = elem.first;
#endif
        set->beginGroup(QString("key%1").arg(row));
        set->setValue( "key", key);
#if defined(_MSC_VER) && (_MSC_VER < 1900)
        const SignalArray& array = it->second;
#elif defined(__GNUG__) && (__cplusplus >= 201103L)
        const SignalArray& array = elem.second;
#endif
        set->beginWriteArray("mu-sigma-values");
        int index = 0;
#if defined(_MSC_VER) && (_MSC_VER < 1900)
        for ( SignalArray::const_iterator iter = array.begin(); iter != array.end(); ++iter) {
            const SignalPair& p = *iter;
#elif defined(__GNUG__) && (__cplusplus >= 201103L)
        for ( const SignalPair& p : array) {
#endif
            set->setArrayIndex(index);
            set->setValue( "mu", p.first);
            set->setValue( "sigma", p.second);
            index++;
        }
        set->endArray();
        set->endGroup();
        row++;
    }
    set->endGroup();

    // particle charge to counts map
    set->beginGroup("ChargeSignalMap");
#if defined(_MSC_VER) && (_MSC_VER < 1900)
    for ( ChargeSignalMap::const_iterator it = charge_counts_signals.begin();
          it != charge_counts_signals.end(); ++it) {
        int charge = it->first;
#elif defined(__GNUG__) && (__cplusplus >= 201103L)
    for ( const auto& elem : charge_counts_signals) {
        int charge = elem.first;
#endif
        set->beginGroup(QString("charge%1").arg(charge));
        set->setValue( "charge", charge);
#if defined(_MSC_VER) && (_MSC_VER < 1900)
        const SignalPair& p = it->second;
#elif defined(__GNUG__) && (__cplusplus >= 201103L)
        const SignalPair& p = elem.second;
#endif
        set->setValue( "mu", p.first);
        set->setValue( "sigma", p.second);
        set->endGroup();
    }
    set->endGroup();

    // background
    set->beginGroup("Background");
    set->beginWriteArray("mu-sigma-values");
    int index = 0;
#if defined(_MSC_VER) && (_MSC_VER < 1900)
    for ( SignalArray::const_iterator it = background_signals.begin();
          it != background_signals.end(); ++it) {
        const SignalPair& p = *it;
#elif defined(__GNUG__) && (__cplusplus >= 201103L)
    for ( const SignalPair& p : background_signals) {
#endif
        set->setArrayIndex(index);
        set->setValue( "mu", p.first);
        set->setValue( "sigma", p.second);
        index++;
    }
    set->endArray();
    set->endGroup();

    // charge radius
    set->beginGroup("ChargeRadius");
    set->beginWriteArray("values");
    for ( int i = 0; i < CARBON_Z; ++i) {
        set->setArrayIndex(i + 1);
        set->setValue( "value", charge_radius[i]);
    }
    set->endArray();
    set->endGroup();

    // charge beta
    set->beginGroup("ChargeBeta");
    set->beginWriteArray("values");
    for ( int i = 0; i < CARBON_Z; ++i) {
        set->setArrayIndex(i + 1);
        set->setValue( "value", charge_beta[i]);
    }
    set->endArray();
    set->endGroup();

    const bool* fit_charge = CalibrationFitting::BeamCompositionFit::charge_in_fit();
    // charge in fit
    set->beginGroup("ChargeFit");
    set->beginWriteArray("values");
    for ( int i = 0; i < CARBON_Z; ++i) {
        set->setArrayIndex(i + 1);
        set->setValue( "value", fit_charge[i]);
    }
    set->endArray();
    set->endGroup();
/*
    // linear fit parameters
    set->beginGroup("LinearFit");
    set->setValue( "reference-channel", reference_channel);
    set->setValue( "reference-pos1", fit_points[0]);
    set->setValue( "reference-pos2", fit_points[1]);
    set->setValue( "parameter-a", linear_fit[0]);
    set->setValue( "parameter-b", linear_fit[1]);
    set->endGroup();
*/
    // tension and power
    set->setValue( "reference-charge", reference_charge);
    set->setValue( "projectile-charge", projectile_charge);
    set->setValue( "tension-parameter", tension_parameter);
}

void
Parameters::recalculate_charge_fit(int charge)
{
    const SignalPair& mip = charge_counts_signals[1];
    const SignalPair& charge_signal = charge_counts_signals[charge];
    double projm_mip = 0.13957018;//projmass[0];
    double projm_charge = projmass[charge - 1];
    double beta_mip = charge_beta[0];
    double beta_charge = charge_beta[charge - 1];

    K = correction( beta_charge, projm_charge) / correction( beta_mip, projm_mip);
#if defined(_MSC_VER) && (_MSC_VER < 1900)
    k = log(charge_signal.first / (mip.first * beta_mip * beta_mip * K)) / log(double(charge));
#elif defined(__GNUG__) && (__cplusplus >= 201103L)
    k = log(charge_signal.first / (mip.first * beta_mip * beta_mip * K)) / log(charge);
#endif

//    qDebug() << "K: " << K << " k: " << k;
}

int
Parameters::counts_to_charge( const ChannelsArray& values, ChannelsArray& charges,
    double signal, double beta, double power) const
{
    int charge_detect = 0;

//    double beta_charge = charge_beta[5];

    for ( int i = 0; i < CHANNELS; ++i) {
//        double vv = (values[i] - bb) / aa;
//        if (vv > 0.0) {
        if (values[i] > 0.) {
//            charges[i] = sqrt((values[i] - bb) / aa);
            charges[i] = pow( values[i] / (signal * beta * beta * K), power);
//            charges[i] = sqrt((values[i] * beta_charge * beta_charge) / (signal * beta * beta * K));
//            charges[i] = 1.1771428 * sqrt((values[i] * beta_charge * beta_charge) / (signal * beta * beta * K));
//            charges[i] = pow( values[i] / (charge1.first * 0.5637), 1.0 / 2.33745);
//            charges[i] = sqrt(values[i] / signal);
            charge_detect++;
        }
        else
            charges[i] = -1.0;
    }

    int res = (charge_detect >= CHANNELS - 1) ? majority_scheme(charges) : -1;

    return res;
}

int
Parameters::majority_scheme(const ChannelsArray& z/*, double radius */) const
{
    int pos_z = -1;
/*
    for ( int i = 1; i <= CARBON_Z; ++i) {
        double rms = 0.;
        for ( int j = 0; j < CHANNELS; ++j) {
            rms += (z[j] - double(i)) * (z[j] - double(i));
        }
        rms /= CHANNELS;
        rms = sqrt(rms);
        if (fabs(rms) < 0.2) {
            pos_z = i;
            break;
        }
        else {
            ChannelsArray tmp = z;
            int position_max = std::max_element( tmp.begin(), tmp.end()) - tmp.begin();

            rms = 0.;
            for ( int j = 0; j < CHANNELS; ++j) {
                if (j != position_max)
                    rms += (z[j] - double(i)) * (z[j] - double(i));
            }
            rms /= CHANNELS - 1;
            rms = sqrt(rms);

            if (fabs(rms) < 0.2) {
                pos_z = i;
                break;
            }
        }
    }
*/
    for ( int i = 1; i <= CARBON_Z; ++i) {
        double r = 0.0;
        ChannelsArray delta;
//        bool big = true;
        for ( int j = 0; j < CHANNELS; ++j) {
//            if (j == 0 || j == 1 || j == 3) {
                double diff = z[j] - double(i);
                delta[j] = diff;
                r += diff * diff;
//            if (delta[j] < 0.)
//                big = false;
//            }
        }

//        bool border = fabs(delta[0]) + fabs(delta[CHANNELS - 1]) <= 1.0;
/*
        bool b1_4 = fabs(delta[0]) <= 0.5 && fabs(delta[1]) <= 0.5 && fabs(delta[2]) <= 0.5 && fabs(delta[3]) <= 0.5;
        bool b12 = fabs(delta[0]) <= 0.5 && fabs(delta[1]) <= 0.5;
        bool b23 = fabs(delta[1]) <= 0.5 && fabs(delta[2]) <= 0.5;
        bool b34 = fabs(delta[2]) <= 0.5 && fabs(delta[3]) <= 0.5;
        bool b1_3 = fabs(delta[0]) <= 0.5 && fabs(delta[1]) <= 0.5 && fabs(delta[2]) <= 0.5;
        bool b2_4 = fabs(delta[1]) <= 0.5 && fabs(delta[2]) <= 0.5 && fabs(delta[3]) <= 0.5;

        if (i == 6 && b1_4)
            accepted[0]++;
        else
            rejected[0]++;

        if (i == 6 && (b12 || b23 || b34) && !b1_4 && !b1_3 && !b2_4)
            accepted[1]++;
        else
            rejected[1]++;

        if (i == 6 && (b1_3 || b2_4) && !b1_4)
            accepted[2]++;
        else
            rejected[2]++;
*/
        r = sqrt(r / double(CHANNELS));

        if (/*border && */r <= charge_radius[i - 1]) {
            pos_z = i;
            break;
        }
//        else if (i == CARBON_Z && big) {
//            pos_z = i;
//            break;
//        }
    }

    return pos_z;
}
/*
TH1*
Parameters::rebin( int channel, double min, double max, int bins, Diagrams &diagrams)
{
    TH1* ch = diagrams.channels[channel];

    double* x1 = new double[bins];
    double* y1 = new double[bins];
    double* x2 = new double[bins];
    double* bbins = new double[bins];

    double* pp[CHANNELS] = { p1, p2, p3, p4 };

    double c = (max - min) / bins;
    double* p = pp[channel];

    for ( int i = 1; i < bins; ++i) {
        x1[i] = linear_fit[0] * i * c;
        y1[i] = splfit( x1[i], y, x, p, fitn, tension_parameter);
        x2[i] = y1[i] * linear_fit[0];
        bbins[i] = x2[i];
    }
    TH1* rebin_channel = ch->Rebin( bins - 1, "rebin", bbins);

    delete [] x1;
    delete [] y1;
    delete [] x2;
    delete [] bbins;

    return rebin_channel;
}
*/
} // namespace CalibrationFitting
