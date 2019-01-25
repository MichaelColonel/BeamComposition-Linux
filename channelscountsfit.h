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

#include <QtGlobal>

#include <memory>

#include "runinfo.h"
#include "typedefs.h"

#if defined(Q_OS_WIN) && !defined(BOOST_CORE_NONCOPYABLE_HPP)
namespace boost {

//  Private copy constructor and copy assignment ensure classes derived from
//  class noncopyable cannot be copied.

//  Contributed by Dave Abrahams

namespace noncopyable_  // protection from unintended ADL
{
  class noncopyable
  {
   protected:
    noncopyable() {}
      ~noncopyable() {}
    private:  // emphasize the following members are private
      noncopyable( const noncopyable& );
      noncopyable& operator=( const noncopyable& );
  };
}

typedef noncopyable_::noncopyable noncopyable;

} // namespace boost
#elif defined(Q_OS_LINUX)
#include <boost/core/noncopyable.hpp>
#endif

class QSettings;

namespace CalibrationFitting {

class Parameters;
typedef std::shared_ptr<Parameters> SharedParameters;

class Parameters : private boost::noncopyable {

public:
    virtual ~Parameters();

    ReferenceSignalMap& reference_signals() { return reference_counts_signals; }
    ChargeSignalMap& charge_signals() { return charge_counts_signals; }
    SignalArray& background() { return background_signals; }
    double& tension() { return tension_parameter; }
    RunInfo fit( const CountsList&, Diagrams& diagrams, bool background_flag = false);
    void save(QSettings* set);
    void restore_reference_signals();
    void restore_charge_signals();
//    void recalculate( int ref_channel, int fit_pos_start, int fit_pos_stop);
    void recalculate();
    void recalculate_charge_fit(int charge = CARBON_Z);
//    TH1* rebin( int channel, double min, double max, int bins, Diagrams& diagrams);
//    double* refit_channel( int i, int bins, double min, double max) const;
//    const double* fit_parameters() const { return linear_fit; }
    int& reference_charge_parameter() { return reference_charge; }
    int& projectile_charge_parameter() { return projectile_charge; }
    double* charge_radius_parameter() { return charge_radius; }
    double* charge_beta_parameter() { return charge_beta; }
//    void set_linear_fit_parameters( double a, double b) {
//        linear_fit[0] = a;
 //       linear_fit[1] = b;
//    }

//    const int* reference_channel_parameters(int& ref_channel) const {
//        ref_channel = reference_channel;
//        return fit_points;
//    }

    static SharedParameters instance(QSettings* settings = nullptr);

protected:
    double tension_parameter;
    SignalArray background_signals;
    ReferenceSignalMap reference_counts_signals;
    ChargeSignalMap charge_counts_signals;
//    double linear_fit[10];
//    int reference_channel;
//    int fit_points[2];
    double k;
    double K;
    int reference_charge;
    int projectile_charge;
    double charge_radius[CARBON_Z];
    double charge_beta[CARBON_Z];
    bool flag_process_data;

private:
    typedef std::array< double, CHANNELS> ChannelsArray;
    void initiate(QSettings* set = nullptr);
//    double count_to_charge(double count);
    int counts_to_charge( const ChannelsArray& counts, ChannelsArray& charges,
                          double mip_signal, double beta, double power) const;
    int majority_scheme(const ChannelsArray& counts/*, double charge_radius*/) const;

    Parameters(QSettings* settings = nullptr);
    static SharedParameters instance_;
};

} // namespace CalibrationFitting

typedef CalibrationFitting::Parameters FitParameters;
typedef CalibrationFitting::SharedParameters SharedFitParameters;
