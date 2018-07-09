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

class RunInfo {
public:
    typedef std::array< size_t, CARBON_Z > ChargeEventArray;
    RunInfo();
    RunInfo( size_t counted, size_t processed,
        const ChargeEventArray& composition);
    RunInfo(const RunInfo& src);
    virtual ~RunInfo() {}
    RunInfo& operator=(const RunInfo& src);
    RunInfo operator+(const RunInfo& src);
    RunInfo& operator+=(const RunInfo& src);
    bool operator==(const RunInfo& src) const;

    size_t counted() const { return triggers_counted; }
    size_t processed() const { return triggers_processed; }

    double averageComposition(int Z) const;
    void clear();

private:
    size_t triggers_counted;
    size_t triggers_processed;

    ChargeEventArray comp; // charge events counters
};

inline
RunInfo::RunInfo()
    :
    triggers_counted(0),
    triggers_processed(0)
{
    std::fill( comp.begin(), comp.end(), 0);
}

inline
RunInfo::RunInfo( size_t counted, size_t processed,
    const ChargeEventArray& composition)
    :
    triggers_counted(counted),
    triggers_processed(processed),
    comp(composition)
{
}

inline
RunInfo&
RunInfo::operator=(const RunInfo& src)
{
    this->triggers_counted = src.triggers_counted;
    this->triggers_processed = src.triggers_processed;
    this->comp = src.comp;

    return *this;
}

inline
RunInfo::RunInfo(const RunInfo &src)
    :
    triggers_counted(src.triggers_counted),
    triggers_processed(src.triggers_processed),
    comp(src.comp)
{
}

inline
RunInfo
RunInfo::operator+(const RunInfo& obj)
{
    RunInfo newinfo;
    newinfo.triggers_counted = this->triggers_counted + obj.triggers_counted;
    newinfo.triggers_processed = this->triggers_processed + obj.triggers_processed;

    std::transform( this->comp.begin(), this->comp.end(),
                    obj.comp.begin(), newinfo.comp.begin(),
                    std::plus<ChargeEventArray::value_type>());

    return newinfo;
}


inline
RunInfo&
RunInfo::operator+=(const RunInfo& obj)
{
    this->triggers_counted += obj.triggers_counted;
    this->triggers_processed += obj.triggers_processed;

    std::transform( this->comp.begin(), this->comp.end(),
                    obj.comp.begin(), this->comp.begin(),
                    std::plus<ChargeEventArray::value_type>());

    return *this;
}

inline
bool
RunInfo::operator==(const RunInfo& obj) const
{
    bool res = false;
    bool size1 = this->triggers_counted == obj.triggers_counted;
    bool size2 = this->triggers_processed == obj.triggers_processed;
    if (size1 && size2) {
         res = std::equal( this->comp.begin(), this->comp.end(), obj.comp.begin());
    }
    return res;
}

inline
void
RunInfo::clear()
{
    triggers_counted = 0;
    triggers_processed = 0;
    std::fill( comp.begin(), comp.end(), 0);
}

inline
double
RunInfo::averageComposition(int Z) const
{
    return (triggers_processed) ? double(comp[Z]) / triggers_processed : -1.0;
}
