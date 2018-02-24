/*************************************************************************/
/* OSC interface - a minimal OSC interface                               */
/* Copyright (C) 2018                                                    */
/* Johannes Lorenz (jlsf2013 @ sourceforge)                              */
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

#ifndef OSCINTERFACE_H
#define OSCINTERFACE_H

//! Base class for the OSC consumer
class OscConsumer
{
public:
	//! Must provide @p sample_count new samples in
	//! audio buffers @p outl and @p outr
	virtual void runSynth(float* outl, float* outr,
		unsigned long sample_count) = 0;
	//! Must send OSC messages to the consumer
	virtual void sendOsc(const char* port, const char* args, ...) = 0;
	//! Must return the consumer's used buffersize
	virtual unsigned long buffersize() const = 0;
	//! Destructor, must clean up any allocated memory
	virtual ~OscConsumer() {}
};

//! Base class to let the producer provide information without
//! it requiring to be started
class OscDescriptor
{
public:
	//! License possibilities
	enum class license_type
	{
		gpl_3_0,  //!< GPL 3.0
		gpl_2_0,  //!< GPL 2.0
		lgpl_3_0, //!< LGPL 3.0
		lgpl_2_1  //!< LGPL 2.1
	};

	//! Plugin descriptor which will not change over time
	//! (e.g. "joe-smith-sweep-3")
	virtual const char* label() const = 0;
	
	//! Plugin name
	//! (e.g. "Joe Smith's Sweep III - Resonant filter swept
	//!  by a Lorenz fractal")
	virtual const char* name() const = 0;

	//! Author or organisation name
	virtual const char* maker() const = 0;

	//! License that the consumer is coded in
	virtual license_type license() const = 0;

//	virtual int id() const = 0;

	//! Function that must return an allocated OscConsumer
	virtual OscConsumer* instantiate(unsigned long srate) const = 0;

	//! Desctructor, must clean up any allocated memory
	virtual ~OscDescriptor() {}
};

//! Function that must return an OscDescriptor.
//! The argument must currently be 0.
//! Entry point for any consumer.
typedef OscDescriptor* (*osc_descriptor_loader_t) (unsigned long);

#endif // OSCINTERFACE_H

