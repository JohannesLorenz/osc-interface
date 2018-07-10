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

/*
	!WARNING!

	This plugin library is currently under development. It may change
	a lot, even without announcement.

	At this unstable state, port contents may still change, and as a result,
	different versions of the library may not be compatible.
*/

/**
	@file audio.h
	spa audio utils
*/

#ifndef SPA_AUDIO_H
#define SPA_AUDIO_H

#include <rtosc/pseudo-rtosc.h>

#include "spa.h"

namespace spa {
namespace audio {

/*
	exceptions
*/
//! an error the plugin *can* throw if the args do not match the port
class invalid_args_error : public error_base
{
public:
	const char* portname;
	const char* args_found;
	invalid_args_error(const char* portname, const char* args_found) :
		error_base("invalid OSC args"),
		portname(portname),
		args_found(args_found) {}
};

// assertions that throw errors

//! let the plugin assert that the types supplied in a message for a port
//! match that port's types
inline void assert_types_are(const char* port, const char* exp_types,
				const char* types) noexcept(false) {
	if(!detail::m_streq(exp_types, types))
	throw invalid_args_error(port, types);
}

/*
	port types
*/

namespace stereo {

	//! enum for the case of two separate ports per stereo signal
	enum { left, right };

	//! audio signal input
	class in : public port_ref_base
	{
	public:
		SPA_OBJECT

		const float* left;
		const float* right;

		int directions() const override { return direction_t::input; }
	};

	//! audio signal output
	class out : public port_ref_base
	{
	public:
		SPA_OBJECT

		float* left;
		float* right;

		int directions() const override { return direction_t::output; }
	};
} // namespace stereo

//! audio signal input
class in : public virtual port_ref<const float>, public virtual counted,
	public virtual input
{
	SPA_OBJECT
};

//! audio signal input
class out : public port_ref<float>, public virtual counted,
	public virtual output
{
	SPA_OBJECT
};

enum class scale_type_t
{
	linear,
	logartihmic
};

template<class T>
class control_in : public virtual port_ref<const T>, public virtual input
{
	SPA_OBJECT
	scale_type_t scale_type;
	T min;
	T max;
	T def;
};

template<class T>
class control_out : public virtual port_ref<T>, public virtual input
{
	SPA_OBJECT
	scale_type_t scale_type;
	T min;
	T max;
};

class samplerate : public virtual control_in<int> {
	SPA_OBJECT
};

class buffersize : public virtual control_in<int> {
	SPA_OBJECT
};

//! ringbuffer instance for the host
class osc_ringbuffer : public ringbuffer<char>
{
	using base = ringbuffer<char>;
public:
	void write(const char *dest, const char *args, ...)
	{
		va_list va;
		va_start(va,args);
		write(dest, args, va);
		va_end(va);
	}
	void write(const char *dest, const char *args, va_list va)
	{
		// TODO: => move to cpp file
		// TODO: check iwyu?
		const size_t len =
		pseudo_rtosc::rtosc_vmessage(write_buffer, max_msg,
			dest, args, va);

		write_with_length(write_buffer, len);
	}

	osc_ringbuffer(std::size_t size, int max_msg = 1024) :
		base(size),
		max_msg(max_msg), write_buffer(new char[max_msg]) {}
	~osc_ringbuffer() { delete[] write_buffer; }
	private:
	std::size_t max_msg;
	char* write_buffer;
};

//! ringbuffer in port for plugins to reference a host ringbuffer
class osc_ringbuffer_in : public ringbuffer_in<char>
{
public:
	SPA_OBJECT
	using base = ringbuffer_in<char>;
	osc_ringbuffer_in(std::size_t s, int max_msg = 1024) :
		base(s),
		max_msg(max_msg), read_buffer(new char[max_msg]) {}
	~osc_ringbuffer_in() { delete[] read_buffer; }

	//! read the next message into temporary buffer
	//! @return true iff there was a next message;
	bool read_msg() { return base::read_msg(read_buffer, max_msg); }

	const char* path() const { return read_buffer; }
	const char* types() const { return pseudo_rtosc::rtosc_argument_string(
		read_buffer); }
	pseudo_rtosc::rtosc_arg_t arg(unsigned i) const { return
		pseudo_rtosc::rtosc_argument(read_buffer, i); }

	// TODO: private?!
	std::size_t max_msg;
	char* read_buffer; // TODO: smash?
};

//! ringbuffer out port for plugins to reference a host ringbuffer
class osc_ringbuffer_out : public ringbuffer_out<char>
{
	// TODO: untested
	using base = ringbuffer_out<char>;
public:
	SPA_OBJECT
	void set_ref(osc_ringbuffer* pointer) {
		base::ref = static_cast<ringbuffer<char>*>(pointer); }
	osc_ringbuffer& ref() {
		return static_cast<osc_ringbuffer&>(*base::ref); }
	const osc_ringbuffer& ref() const {
		return static_cast<const osc_ringbuffer&>(*base::ref); }
};

/*
	visitor
*/

class spa_audio_visitor : public virtual spa_visitor
{
public:
	using spa_visitor::visit;

#define SPA_MK_VISIT_AUDIO(type) \
	SPA_MK_VISIT(control_in<type>, port_ref<const type>) \
	SPA_MK_VISIT(control_out<type>, port_ref<type>)

#define SPA_MK_VISIT_AUDIO2(type) \
	SPA_MK_VISIT_AUDIO(type) \
	SPA_MK_VISIT_AUDIO(unsigned type)

	SPA_MK_VISIT_AUDIO2(char)
	SPA_MK_VISIT_AUDIO2(short)
	SPA_MK_VISIT_AUDIO2(int)
	SPA_MK_VISIT_AUDIO2(long)
	SPA_MK_VISIT_AUDIO2(long long)
	SPA_MK_VISIT_AUDIO(float)
	SPA_MK_VISIT_AUDIO(double)

#undef SPA_MK_VISIT_AUDIO2
#undef SPA_MK_VISIT_AUDIO

	SPA_MK_VISIT(audio::stereo::in, port_ref_base)
	SPA_MK_VISIT(audio::stereo::out, port_ref_base)

	SPA_MK_VISIT(osc_ringbuffer_in, ringbuffer_in<char>)
	SPA_MK_VISIT(osc_ringbuffer_out, ringbuffer_out<char>)

	SPA_MK_VISIT(in, port_ref<const float>)
	SPA_MK_VISIT(out, port_ref<float>)
	SPA_MK_VISIT(samplerate, control_in<int>)
	SPA_MK_VISIT(buffersize, control_in<int>)
};

/*
	accept definitions
*/

#define ACCEPT_SPA_AUDIO(classname) ACCEPT(classname, spa_audio_visitor)
#define ACCEPT_SPA_AUDIO_T(classname) ACCEPT_T(classname, spa_audio_visitor)

namespace stereo {
	ACCEPT_SPA_AUDIO(in)
	ACCEPT_SPA_AUDIO(out)
}

ACCEPT_SPA_AUDIO(in)
ACCEPT_SPA_AUDIO(out)
ACCEPT_SPA_AUDIO_T(control_in)
ACCEPT_SPA_AUDIO_T(control_out)
ACCEPT_SPA_AUDIO(samplerate)
ACCEPT_SPA_AUDIO(buffersize)

ACCEPT_SPA_AUDIO(osc_ringbuffer_in)

#undef ACCEPT_SPA_AUDIO_T
#undef ACCEPT_SPA_AUDIO

} // namespace audio
} // namespace spa

#endif // SPA_AUDIO_H
