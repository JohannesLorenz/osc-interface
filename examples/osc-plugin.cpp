/*************************************************************************/
/* osc-plugin.cpp - a minimal OSC example plugin                         */
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

/**
	@file osc-plugin.cpp
	a simple example gain plugin
	less than 170 LOC, including license and metadata
	@todo remove the port enum, it's useless
*/

#include <cstring>
#include <iostream>

#include <spa/audio.h>

class example_plugin : public spa::plugin
{
	//! ports can be extended, this is being hidden from the sourcee
	struct buffersize_port : public spa::audio::buffersize
	{
		//! last value of buffersize before it changed
		//! usually, there is utils::port_rack to check which ports
		//! changed,
		int last;
		buffersize_port() : last(0) {}
	};

public:
	void run() override
	{
		while(osc_in.read_msg())
		{
			if(!strcmp(osc_in.path(), "/gain"))
			{
				spa::audio::assert_types_are("/gain",
					"f", osc_in.types());
				gain = osc_in.arg(0).f;
			} else {
				std::cerr << "warning: unsupported "
					"OSC string \"" << osc_in.path()
					<< "\", ignoring...";
			}
		}

		for(int i = 0; i < buffersize; ++i)
		{
			out.left[i] = gain * in.left[i];
			out.right[i] = gain * in.right[i];
//			printf("generating %f, %f\n", gain*l_in, gain*r_in);
		}
	}

	struct {
		std::string operation;
		std::string file;
		uint64_t stamp;
		bool status;
	} recent;


public:	// FEATURE: make these private?
	virtual ~example_plugin() {}
	example_plugin() : osc_in(1024) {}


	enum port_id
	{
		p_in,
		p_out,
		p_buffersize,
		p_osc
	};

	bool ui_ext() const override { return false; }

private:

	void activate() override {}
	void deactivate() override {}

	float gain = 0.0f; // received via OSC

	spa::audio::stereo::in in;
	spa::audio::stereo::out out;
	buffersize_port buffersize;
	spa::audio::osc_ringbuffer_in osc_in;

	spa::port_ref_base& port(const char* path) override
	{
		using p = spa::port_ref_base;
		switch(path[0])
		{
			case 'i': return in;
			case 'o': return path[1] == 's' ? (p&)osc_in : (p&)out;
			case 'b': return buffersize;
			default: throw spa::port_not_found_error(path);
		}
	}
};

class example_descriptor : public spa::descriptor
{
public:
	example_descriptor() { properties.hard_rt_capable = 1; }

	hoster_t hoster() const override { return hoster_t::github; }
	const char* organization_url() const override {
		return "JohannesLorenz"; /* TODO: create spa organisation? */ }
	const char* project_url() const override { return "spa"; }
	const char* label() const override { return "example-plugin"; }

	const char* project() const override { return "spa"; }
	const char* name() const override { return "Example Plugin"; }
	const char* authors() const override { return "Johannes Lorenz"; }

	const char* description_full() const override {
		return description_line(); }
	const char* description_line() const override {
		return "example audio gain plugin for tests"; }

	license_type license() const override { return license_type::gpl_3_0; }

	// usually, a std::map/std::vector is better, but let's keep it simple
	const char* get_port_name(int state) const override
	{
		using p = example_plugin;
		switch(state)
		{
			case p::p_in: return "in";
			case p::p_out: return "out";
			case p::p_buffersize: return "buffersize";
			case p::p_osc: return "osc";
			default: return nullptr;
		}
	}
	example_plugin* instantiate() const override {
		return new example_plugin; }
};

extern "C" {
//! the main entry point
const spa::descriptor* spa_descriptor(unsigned long )
{
	// we have only one plugin, ignore the number
	return new example_descriptor;
}
}

