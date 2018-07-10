/*************************************************************************/
/* spa - simple plugin API                                               */
/* Copyright (C) 2018                                                    */
/* Johannes Lorenz (j.git$$$lorenz-ho.me, $$$=@)                         */
/*                                                                       */
/* This program is free software; you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation; either version 3 of the License, or (at */
/* your option) any later version.                                       */
/* This program is distributed in the hope that it will be useful, but   */
/* WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      */
/* General Public License for more details.                              */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program; if not, write to the Free Software           */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA  */
/*************************************************************************/

//! @file forward declarations for spa.h

#ifndef SPA_AUDIO_FWD_H
#define SPA_AUDIO_FWD_H

namespace spa {
namespace audio {

class invalid_args_error;

namespace stereo {
	class in;
	class out;
}

class in;
class out;
enum class scale_type_t;

template<class T> class control_in;
template<class T> class control_out;
class samplerate;
class buffersize;

class osc_ringbuffer;
class osc_ringbuffer_in;
class osc_ringbuffer_out;

class spa_audio_visitor; // TODO: rename

}
}

#endif // SPA_AUDIO_FWD_H
