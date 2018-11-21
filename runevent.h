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

#include <TObject.h>

#include "typedefs.h"

class RunEvent : public TObject
{
public:
    RunEvent( const CountsArray& adc, const FittedCountsArray& fitted, int charge);
    RunEvent(const RunEvent& src);
    RunEvent& operator=(const RunEvent& src);
    virtual ~RunEvent();
    static void setBackground(const SignalArray& pedestals);
    ClassDef( RunEvent, 1);
private:
    CountsArray adc_data;
    FittedCountsArray adc_fitted_data;
    int charge;
    static SignalArray background_data;
};
