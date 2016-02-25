#ifndef __GVerbPatch_hpp__
#define __GVerbPatch_hpp__
/*

        Copyright (C) 1999 Juhana Sadeharju
                       kouhia at nic.funet.fi

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */

/**
 * Port of GVerb to the OWL by Martin Klang, Feb 2016
 * Ref: https://github.com/swh/lv2
 */

// make PATCH_OBJS='../MyPatches/gverb.o ../MyPatches/gverbdsp.o' PATCHNAME=GVerb PATCHSOURCE=../MyPatches/ patch

#include "StompBox.h"

namespace GVerb {
  extern "C"{

    inline int isnan(float x){
      return std::isnan(x);
    }

#include "gverb.h"

  }
}

class GVerbPatch : public Patch {
private:
  GVerb::ty_gverb* verb;
public:
  GVerbPatch(){
    registerParameter(PARAMETER_A, "Size");
    registerParameter(PARAMETER_B, "Time");
    registerParameter(PARAMETER_C, "Damp");
    registerParameter(PARAMETER_D, "Dry/Wet");
    registerParameter(PARAMETER_E, "Early");
    verb = GVerb::gverb_new(getSampleRate(), 300.0f, 50.0f, 7.0f, 0.5f, 15.0f, 0.5f, 0.5f, 0.5f);
  }      
  void processAudio(AudioBuffer &buffer) {
    float size = getParameterValue(PARAMETER_A)*300;
    float time = getParameterValue(PARAMETER_B)*10;
    float damp = getParameterValue(PARAMETER_C);
    float wet = getParameterValue(PARAMETER_D);
    // float early = getParameterValue(PARAMETER_E);
    GVerb::gverb_set_roomsize(verb, size);
    GVerb::gverb_set_revtime(verb, time);
    GVerb::gverb_set_damping(verb, damp);
    // GVerb::gverb_set_earlylevel(verb, early);
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);
    float l, r, dry;
    for(int i=0; i<buffer.getSize(); ++i){
      GVerb::gverb_do(verb, left[i], &l, &r);
      dry = left[i]*(1-wet);
      left[i] = l*wet + dry;
      right[i] = r*wet + dry;
    }
  }
};

#endif   // __GVerbPatch_hpp__
