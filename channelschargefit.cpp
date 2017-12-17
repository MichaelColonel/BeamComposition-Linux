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

#include <TF1.h>
#include <TH1.h>
#include <TMath.h>
#include <TLegend.h>

#include <sstream>

#include "channelschargefit.h"

namespace {

// charge names
//const char* cnames[7] = { "0", "H", "He", "Li", "Be", "B", "C" };

// charge colors
const Color_t ccolors[7] = { kBlack, kRed, kBlue, kCyan, kOrange, kMagenta + 10, kViolet };

// number of Gaus parameters
const int gparams = 3;

const char* fit_parameters_names[] = {
    "Const1", "Mean1", "Sigma1", // Z = 1
    "Const2", "Mean2", "Sigma2", // Z = 2
    "Const3", "Mean3", "Sigma3", // Z = 3
    "Const4", "Mean4", "Sigma4", // Z = 4
    "Const5", "Mean5", "Sigma5", // Z = 5
    "Const6", "Mean6", "Sigma6"  // Z = 6
};

} // namespace

namespace CalibrationFitting {

bool BeamCompositionFit::fit_it[CARBON_Z] = { true, true, true, true, true, true };

BeamCompositionFit::BeamCompositionFit( TH1* hist,
    unsigned int minz, unsigned int maxz, double dz)
{
    Double_t mina = hist->GetXaxis()->GetXmin();
    Double_t maxa = hist->GetXaxis()->GetXmax();

    for( unsigned int i = minz; i <= maxz; i++) {
        double z(i);
        hist->SetAxisRange( z - dz, z + dz);
        GausParametersPair pair = fit_gaus( hist, z, dz);
        hist->SetAxisRange( mina, maxa);
        params[i] = pair.first;
        errors[i] = pair.second;
    }
}

BeamCompositionFit::~BeamCompositionFit()
{

}

double*
BeamCompositionFit::fit_parameters() const
{
    size_t npeaks = params.size();
    double* par = new double[npeaks * gparams];
    int i = 0;
#if defined(_MSC_VER) && (_MSC_VER < 1900) && defined(Q_OS_WIN)
    for ( ChargeGausParametersMap::const_iterator it = params.begin(); it != params.end(); ++it) {
        const GausParameters& values = it->second;
        for ( GausParameters::const_iterator iter = values.begin(); iter != values.end(); ++iter) {
            const double& value = *iter;
#elif defined(Q_OS_LINUX)
    for( const auto& pair : params) {
        for ( double value : pair.second) {
#endif
            par[i++] = value;
        }
    }
    return par;
}

double*
BeamCompositionFit::fit_errors() const
{
    size_t npeaks = errors.size();
    double* par = new double[npeaks * gparams];
    int i = 0;
#if defined(_MSC_VER) && (_MSC_VER < 1900) && defined(Q_OS_WIN)
    for ( ChargeGausParametersMap::const_iterator it = params.begin(); it != params.end(); ++it) {
        const GausParameters& values = it->second;
        for ( GausParameters::const_iterator iter = values.begin(); iter != values.end(); ++iter) {
            const double& value = *iter;
#elif defined(Q_OS_LINUX)
    for( const auto& pair : errors) {
        for ( double value : pair.second) {
#endif
            par[i++] = value;
        }
    }
    return par;
}

double
BeamCompositionFit::operator()( double* x, double* par) const
{
    size_t npeaks = params.size();
    double result = 0.0;
    for ( size_t i = 0; i < npeaks; ++i)  {
        double norm = par[gparams * i + 0];
        double mean = par[gparams * i + 1];
        double sigma = par[gparams * i + 2];
        result += norm * TMath::Gaus( *x, mean, sigma, kFALSE);
    }
    return result;
}

GausParametersPair
BeamCompositionFit::fit_gaus( TH1* hist, double z, double dz) const
{
    TF1 *fun = new TF1( "fun", "gaus", z - dz, z + dz);
#if defined(_MSC_VER) && (_MSC_VER < 1900) && defined(Q_OS_WIN)
    GausParameters gpar;
    gpar[0] = 100.;
    gpar[1] = z;
    gpar[2] = dz;
#elif defined(Q_OS_LINUX)
    GausParameters gpar({ 100. , z,  dz}); // values (norm, mean, sigma)
#endif
    GausParameters gepar; // errors
    fun->SetParameters(gpar.data());
    hist->Fit(fun);

    fun->GetParameters(gpar.data());
    const double* epar = fun->GetParErrors();
    //---------------
    for( int i = 0; i < gparams; i++) {
        gepar[i] = epar[i];
    }
    //---------------

    delete fun;

    return std::make_pair( gpar, gepar);
}

TF1*
BeamCompositionFit::fit_charge( unsigned int Z, double dz) const
{
    char name[4] = "FZX";
    name[2] = Z + '0';
    double z(Z);
    TF1* fun = new TF1( name, "gaus", z - dz, z + dz);
    const GausParameters& par = params.at(Z);
    const GausParameters& epar = errors.at(Z);
    fun->SetParameters(par.data());
    fun->SetParErrors(epar.data());
    return fun;
}

bool*
BeamCompositionFit::charge_in_fit()
{
    return fit_it;
}

TF1*
BeamCompositionFit::fit( BeamCompositionFit& fbeam, TH1* hist,
    unsigned int zmin, unsigned int zmax, double dz)
{
    Double_t* par = fbeam.fit_parameters();
    Double_t* epar = fbeam.fit_errors();
    int peaks = fbeam.npeaks();

    TF1* fit = new TF1( "fit", fbeam,
        double(zmin) - dz, double(zmax) + dz, gparams * peaks);

    fit->SetLineWidth(2.5);
    fit->SetLineColor(kBlack);
    fit->SetLineStyle(1);
    for ( unsigned int i = zmin; i <= zmax; ++i) {
//        if (!(fit_it[i - 1]))
//            continue;
        size_t pos = gparams * (i - zmin);
        for ( int j = 0; j < gparams; ++j)
            fit->SetParName( pos + j, fit_parameters_names[gparams * (i - 1) + j]);
    }
//	TVirtualFitter* ft = TVirtualFitter::Fitter( hist, 3 * peaks);
//	ft->SetParameter( 17, "Z=6", 6.0, 0.2, 5.7, 6.2);
//	ft->FixParameter(17);
    fit->SetParameters(par);
    fit->SetParErrors(epar);
    fit->SetNpx(400);
//    hist->Fit( "fit", "+rob=0.75", "ep");
    hist->Fit( "fit", "RB0", "ep");

    delete [] par;
    delete [] epar;
    return fit;
}

/*
std::map< int, TF1* >
BeamCompositionFit::fit_legend( BeamCompositionFit& fbeam, TF1* fit, TLegend* legend)
{
    std::map< int, TF1* > charges;

    Double_t* params = fbeam.fit_parameters();
    fit->GetParameters(params);

    int zmax = 6;
    int zmin = 1;

    double totint = fit->Integral( 0.0, 7.5);
    for ( int i = zmin; i <= zmax; i++) {
        double p[3];
        p[0] = params[3 * (i - zmin) + 0];
        p[1] = params[3 * (i - zmin) + 1];
        p[2] = params[3 * (i - zmin) + 2];
        TF1* ft = fbeam.fit_charge( i, 0.7); // don't delete
        charges.insert(std::make_pair( i, ft));
        ft->SetParameters(p);
        ft->SetLineColor(ccolors[i]);
        ft->SetFillColor(ccolors[i]);
        ft->SetFillStyle(3001);
        ft->SetLineWidth(1);
        ft->SetNpx(600);
        ft->Draw("same");

        double parint = ft->Integral( double(i) - 1.0, double(i) + 1.0);
        double percent = floor(1000.0 * parint / totint) / 10.0;
        std::stringstream ss;
        ss << "Z = " << i <<  " : " << percent << " %";
        std::string str = ss.str();
        legend->AddEntry( ft, str.c_str(), "l");
    }

    delete [] params;
    return charges;
}
*/

} // CalibrationFitting
