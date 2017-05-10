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

#include "typedefs.h"

struct Hist1Parameters {
    void setBins( int bin, double low, double high) { bins = bin; min = low; max = high; }

    DiagramType type;
    const char* name;
    const char* title;
    int bins;
    double min;
    double max;
    static double energy_per_count;
};

struct Hist2Parameters {
    void setBinsX(const Hist1Parameters& h) { xmin = h.min; xmax = h.max; xbins = h.bins; }
    void setBinsY(const Hist1Parameters& h) { ymin = h.min; ymax = h.max; ybins = h.bins; }

    DiagramType type;
    const char* name;
    const char* title;
    int xbins;
    double xmin;
    double xmax;
    int ybins;
    double ymin;
    double ymax;
};
