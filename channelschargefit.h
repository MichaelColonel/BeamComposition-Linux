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

#pragma once

#include "typedefs.h"
#include <map>

class TF1;
class TLegend;

namespace CalibrationFitting {

typedef std::array< double, 3 > GausParameters;
typedef std::pair< GausParameters, GausParameters > GausParametersPair;
typedef std::map< unsigned int, GausParameters > ChargeGausParametersMap;

class BeamCompositionFit {
public:
    // use constructor to customize your function object
    BeamCompositionFit( TH1*, unsigned int zmin = 0, unsigned int zmax = 6, double dz = 0.5);
    virtual ~BeamCompositionFit();
    // function implementation using class data members
    double operator() (double* x, double* p) const;
    int npeaks() const { return params.size(); }
    TF1* fit_charge( unsigned int Z, double dz = 0.5) const;
    double* fit_parameters() const;
    double* fit_errors() const;
    static bool* charge_in_fit();
    static TF1* fit( BeamCompositionFit& fbeam, TH1* hist,
        unsigned int zmin = 1, unsigned int zmax = 6, double dz = 0.5);
//    static std::map< int, TF1* > fit_legend( BeamCompositionFit& fbeam, TF1* fit, TLegend* legend);

private:
    GausParametersPair fit_gaus( TH1* his, double z, double dz) const;
    ChargeGausParametersMap params; // fit parameters
    ChargeGausParametersMap errors; // fit parameters errors
    static bool fit_it[CARBON_Z];
};

} // namespace CalibrationFitting
