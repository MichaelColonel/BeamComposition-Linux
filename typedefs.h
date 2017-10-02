/*
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#pragma once

#include <QList>

#include <tuple>
#include <array>
#include <vector>
#include <queue>
#include <map>

#define CHANNELS 4
#define CARBON_Z 6

class TH1;
class TH2;

typedef std::tuple< TH1*, TH2* > DiagramTuple;
typedef std::tuple< TH1**, TH2** > DiagramsTuple;
typedef std::array< quint16, CHANNELS > CountsArray;

// Acquisition run type
enum RunType {
    RUN_BACKGROUND,
    RUN_FIXED_POSITION,
    RUN_SCANNING
};

enum DiagramType {
    NONE,
    HIST_CHANNELS,
    HIST_CHANNEL1,
    HIST_CHANNEL2,
    HIST_CHANNEL3,
    HIST_CHANNEL4,
    HIST_FITALL,
    HIST_FIT_CHANNEL1,
    HIST_FIT_CHANNEL2,
    HIST_FIT_CHANNEL3,
    HIST_FIT_CHANNEL4,
    HIST_FIT_MEAN,
    HIST_FIT_MEDIAN,
    HIST_RANK1,
    HIST_RANK2,
    HIST_RANK3,
    HIST_RANK4,
    HIST_Z,
    HIST_CHANNEL12,
    HIST_CHANNEL23,
    HIST_CHANNEL34,
    HIST_CHANNEL14,
    HIST_CHANNEL13,
    HIST_CHANNEL24,
    HIST_Z12,
    HIST_Z23,
    HIST_Z34,
    HIST_Z14,
    HIST_Z13,
    HIST_Z24
};

typedef QList< CountsArray > CountsList;
typedef QList<quint8> DataList;

typedef std::vector<quint8> DataVector;
typedef std::queue< DataVector > DataQueue;

// pair.first: mu -- mean, average value
// pair.second: sigma -- RMS, sqrt(variance)
typedef std::pair< double, double > SignalPair;
typedef std::array< SignalPair, CHANNELS > SignalArray;
typedef std::map< double, SignalArray > ReferenceSignalMap;
typedef std::map< int, SignalPair > ChargeSignalMap;

struct Diagrams {
    Diagrams()
        :
        fitall(nullptr),
        fit_mean(nullptr),
        fit_median(nullptr),
        z(nullptr),
        c12(nullptr),
        c23(nullptr),
        c34(nullptr),
        c13(nullptr),
        c14(nullptr),
        c24(nullptr),
        z12(nullptr),
        z23(nullptr),
        z34(nullptr),
        z13(nullptr),
        z14(nullptr),
        z24(nullptr)
    {
        std::fill( channels, channels + CHANNELS, nullptr);
        std::fill( rank, rank + CHANNELS, nullptr);
        std::fill( fit, fit + CHANNELS, nullptr);
    }

    Diagrams(const Diagrams& src)
        :
        fitall(src.fitall),
        fit_mean(src.fit_mean),
        fit_median(src.fit_median),
        z(src.z),
        c12(src.c12),
        c23(src.c23),
        c34(src.c34),
        c13(src.c13),
        c14(src.c14),
        c24(src.c24),
        z12(src.z12),
        z23(src.z23),
        z34(src.z34),
        z13(src.z13),
        z14(src.z14),
        z24(src.z24)
    {
        std::copy( src.channels, src.channels + CHANNELS, this->channels);
        std::copy( src.rank, src.rank + CHANNELS, this->rank);
        std::copy( src.fit, src.fit + CHANNELS, this->fit);
    }

    Diagrams& operator=(const Diagrams& src) {
        std::copy( src.channels, src.channels + CHANNELS, this->channels);
        std::copy( src.rank, src.rank + CHANNELS, this->rank);
        std::copy( src.fit, src.fit + CHANNELS, this->fit);
        this->fitall = src.fitall;
        this->fit_mean = src.fit_mean;
        this->fit_median = src.fit_median;
        this->z = src.z;
        this->c12 = src.c12;
        this->c23 = src.c23;
        this->c34 = src.c34;
        this->c13 = src.c13;
        this->c14 = src.c14;
        this->c24 = src.c24;
        this->z12 = src.z12;
        this->z23 = src.z23;
        this->z34 = src.z34;
        this->z13 = src.z13;
        this->z14 = src.z14;
        this->z24 = src.z24;
        return *this;
    }

    TH1* channels[CHANNELS];
    TH1* rank[CHANNELS];
    TH1* fit[CHANNELS];
    TH1* fitall;
    TH1* fit_mean;
    TH1* fit_median;
    TH1* z;
    TH2* c12;
    TH2* c23;
    TH2* c34;
    TH2* c13;
    TH2* c14;
    TH2* c24;
    TH2* z12;
    TH2* z23;
    TH2* z34;
    TH2* z13;
    TH2* z14;
    TH2* z24;
};
