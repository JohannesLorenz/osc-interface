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
	@file spa.h
	generic spa utils
*/

#ifndef SPA_SPA_H
#define SPA_SPA_H

// Note: Any shared data, i.e.
//       * member variables
//       * return types from non-inline functions
//       * thrown errors that reach plugin and host
//       must be in your own (version) control, i.e. no STL, boost, libXYZ...
#include <cstdarg> // only functions for varargs

// The same counts for our own libraries!
#include <ringbuffer/ringbuffer.h>

// fix "major" defined in GNU libraries
#ifdef major
#undef major
#endif

// fix "minor" defined in GNU libraries
#ifdef minor
#undef minor
#endif

#include "spa_fwd.h"

namespace spa {

//! api version to check whether plugin and host are compatible
//! @todo this may not work, needs some better idea
class api_version
{
public:
	//! major spa version, change means API break
	static unsigned major() { return 0; }
	//! minor spa version, change means API break
	static unsigned minor() { return 0; }
	//! patch spa version, change guarantees that API does not break
	static unsigned patch() { return 0; }
};

namespace detail {

inline unsigned m_strlen(const char* str)
{
	unsigned sz = 0;
	for(; *str; ++str, ++sz) ;
	return sz;
}

inline void m_memcpy(char* dest, const char* src, int sz)
{
	for(; sz; --sz, ++dest, ++src) *dest = *src;
}

inline bool m_streq(const char* s1, const char* s2)
{
	for(; (*s1 == *s2) && *s1; ++s1, ++s2) ;
	return *s1 == *s2;
}

}

//! base class for all exceptions that the API introduces
class error_base
{
	const char* _what; //!< simple explenation of the issue
public:
	const char* what() const noexcept { return _what; }
	error_base(const char* what) : _what(what) {}
};

//! host asks for a port using plugin::port , but no port with such a name
class port_not_found_error : public error_base
{
public:
	const char* portname;
	port_not_found_error(const char* portname = nullptr) :
		error_base("no port with that name"),
	portname(portname) {}
	// TODO: iostream operator<< for all classes
};

//! error thrown by spa containers if an element out of range is being
//! requested
class out_of_range_error : public error_base
{
public:
	int accessed;
	int size;
	out_of_range_error(int accessed, int size) :
		error_base("accessed an element out of range"),
		accessed(accessed),
		size(size) {}
};

//! name of the entry function that a host must resolve
constexpr const char* descriptor_name = "spa_descriptor";

//! Simple string on heap, without library dependencies
//! @todo untested
class simple_str
{
	char* _data = nullptr;
	unsigned len;
	char _empty = 0; // TODO: static

	void mdelete() { if(_data != &_empty) delete _data; }
public:
	//! Return an a pointer to the internal byte array, representing a
	//! C style string.
	const char* data() const noexcept { return _data; }
	//! Return the number of chars before the final 0 byte
	unsigned length() const noexcept { return len; }
	//! Return whether the string is empty
	bool empty() const noexcept { return !len; }
	//! Clear the string
	void clear() noexcept { mdelete(); _data = &_empty; len = 0; }
	//! Construct a string, copying @p initial
	simple_str(const char* initial) noexcept(false) { operator=(initial); }
	//! Construct an empty string. Guaranteed not to alloc
	simple_str() noexcept(false) : _data(&_empty), len(0) {}
	//! Assign this string to be a copy of @p newstr
	simple_str& operator=(const char* newstr) noexcept(false)
	{
		mdelete();
		len = detail::m_strlen(newstr);
		unsigned sz = len + 1;
		_data = new char[sz];
		detail::m_memcpy(_data, newstr, sz);
		return *this;
	}
	~simple_str() noexcept { mdelete(); }
	//! Append @p rhs.
	//! @note The internal pointer can change
	simple_str& operator+=(const char* rhs) noexcept(false) {
		unsigned rhslen = detail::m_strlen(rhs);
		char* newdata = new char[len + rhslen + 1];
		detail::m_memcpy(_data, newdata, len);
		detail::m_memcpy(_data + len, rhs, rhslen + 1);
		mdelete();
		_data = newdata;
		len = len + rhslen;
		return *this;
	}
	//! Return character at position @p idx
	char& operator[](int idx) noexcept { return _data[idx]; }
	//! Return character at position @p idx
	const char& operator[](int idx) const noexcept { return _data[idx]; }
	//! Return safely character at position @p idx
	char& at(unsigned idx) noexcept(false)
	{
		if(idx > len)
			throw out_of_range_error(idx, 1+len);
		else
			return _data[idx];
	}
	//! Return safely character at position @p idx
	const char& at(unsigned idx) const noexcept(false)
	{
		if(idx > len)
			throw out_of_range_error(idx, 1+len);
		else
			return _data[idx];
	}

};

class port_ref_base
{
protected:
	//! direction, as seen from the plugin
	enum direction_t {
	input = 1, //!< data from host to plugin
	output = 2 //!< data from plugin to host
	};
	// TODO:
	// - initial value?
	// - compulsory value?

	simple_str label;
public:
	//! combination of direction_t (TODO: use bitmask?)
	virtual int directions() const = 0;

	//! accept function conforming to the visitor pattern
	//! equivalent to writing "SPA_OBJECT"
	virtual void accept(class visitor& v);

	virtual ~port_ref_base() {}
};

//! use this in every class that you want to make visitable
#define SPA_OBJECT void accept(class spa::visitor& v) override;

//! simple class for small types where copying is cheap
template<class T>
class port_ref : public virtual port_ref_base
{
	T* ref;
public:
	SPA_OBJECT

	// TODO: some of those functions may be useless

	operator T() const { return *ref; }
	operator const T*() const { return ref; }
	operator T*() { return ref; }

	const port_ref<T>& operator=(const T& value) { return set(value); }
	const port_ref<T>& set(const T& value) { *ref = value; return *this; }

	//! array operator - only to use if the target is an array
	T& operator[](int i) { return ref[i]; }
	//! array operator - only to use if the target is an array
	const T& operator[](int i) const { return ref[i]; }

	void set_ref(T* pointer) { ref = pointer; }
//	void set_ref(const T* pointer) { ref = pointer; }
};

class counted : public virtual port_ref_base {
public:
	int channel;
};

// not sure if the input output classes make any sense, and whether they should
// have an accept function...

class input : public virtual port_ref_base {
	int directions() const override { return direction_t::input; }
	void accept(class visitor& ) override {}
};

class output : public virtual port_ref_base {
	int directions() const override { return direction_t::output; }
	void accept(class visitor& ) override {}
};

/*
 * ringbuffers on the host side
 */
//! base class for different templates of ringbuffer - don't use directly
template<class T>
class ringbuffer_base : public ringbuffer_t<T>
{
public:
	ringbuffer_base(std::size_t size) : ringbuffer_t<T>(size) {}
};

//! generic ringbuffer class
template<class T>
class ringbuffer : public ringbuffer_base<T>
{
public:
	ringbuffer(std::size_t size) : ringbuffer_t<T>(size) {}
};

//! char ringbuffer specialization, which supports a special write function
template<>
class ringbuffer<char> : public ringbuffer_base<char>
{
	using base = ringbuffer_base<char>;
public:
	void write_with_length(const char* data, std::size_t len)
	{
		if(write_space() >= len + 4)
		{
			uint32_t len32 = len;
			//printf("vmessage length: %u\n", len32);

			char lenc[4] = { (char)((len32 >> 24) & 0xFF),
				(char)((len32 >> 16) & 0xFF),
				(char)((len32 >> 8) & 0xFF),
				(char)((len32) & 0xFF)};
			//printf("length array: %x %x %x %x\n",
			//	+lenc[0], +lenc[1], +lenc[2], +lenc[3]);
			base::write(lenc, 4);
			base::write(data, len);
		}
	}

	ringbuffer(std::size_t size) : base(size) {}
};

/*
 * ringbuffers refs on the host side
 */
//! base class, don't use
template<class T>
class ringbuffer_in_base : public ringbuffer_reader_t<T>, public virtual input
{
public:
	ringbuffer_in_base(std::size_t s) : ringbuffer_reader_t<T>(s) {}
};

//! ringbuffer in port for plugins to reference a host ringbuffer
template<class T>
class ringbuffer_in : public ringbuffer_in_base<T>
{
public:
	SPA_OBJECT
	using ringbuffer_in_base<T>::ringbuffer_in_base;
};

//! ringbuffer in port for plugins to reference a host ringbuffer
template<>
class ringbuffer_in<char> : public ringbuffer_in_base<char>
{
public:
	SPA_OBJECT
	using base = ringbuffer_in_base<char>;
	using base::ringbuffer_in_base;

	//! read the next message into temporary buffer
	//! @return true iff there was a next message;
	bool read_msg(char* read_buffer, std::size_t max)
	{
		if(read_space() > 0)
		{
			uint32_t length;
			{
				auto rd = read(4);
				length =  (+rd[3])
					+ (+rd[2] << 8)
					+ (+rd[1] << 16)
					+ (+rd[0] << 24);
				//printf("read length array: %x%x%x%x\n",
				//+rd[0], +rd[1], +rd[2], +rd[3]);
			}

			//printf("len: %d\n", +length);
			if(length && read_space() < length)
				throw std::runtime_error("char ringbuffer "
					"contains corrupted data");
			if(max < length)
				throw std::runtime_error("read buffer too small"
					"for message");
			else
			{
				auto rd = read(length);
				rd.copy(read_buffer, length);
			}
			return true;
		}
		else
			return false;
	}
};

//! ringbuffer out port for plugins to reference a host ringbuffer
template<class T>
class ringbuffer_out : public virtual output
{
public:
	SPA_OBJECT
	ringbuffer<T>* ref;
};

//! make a visitor function for type @p type, which will default to
//! visiting a base class @p base of it
#define SPA_MK_VISIT(type, base) \
	virtual void visit(type& p) { visit(static_cast<base&>(p)); }

class visitor
{
public:
	virtual void visit(port_ref_base& ) {}

#define SPA_MK_VISIT_PR(type) \
	SPA_MK_VISIT(port_ref<type>, port_ref_base) \
	SPA_MK_VISIT(port_ref<const type>, port_ref_base) \
	SPA_MK_VISIT(ringbuffer_in<type>, port_ref_base)

#define SPA_MK_VISIT_PR2(type) SPA_MK_VISIT_PR(type) \
	SPA_MK_VISIT_PR(unsigned type)

	SPA_MK_VISIT_PR2(char)
	SPA_MK_VISIT_PR2(short)
	SPA_MK_VISIT_PR2(int)
	SPA_MK_VISIT_PR2(long)
	SPA_MK_VISIT_PR2(long long)

	SPA_MK_VISIT_PR(float)
	SPA_MK_VISIT_PR(double)

#undef SPA_MK_VISIT_PR2
#undef SPA_MK_VISIT_PR
};

//! define an accept function for a (non-template) class
#define ACCEPT(classname, visitor_type)\
	inline void classname::accept(class spa::visitor& v) {\
		dynamic_cast<visitor_type&>(v).visit(*this);\
	}

//! define an accept function for a template class
#define ACCEPT_T(classname, visitor_type)\
	template<class T>\
	void classname<T>::accept(class spa::visitor& v) {\
		dynamic_cast<visitor_type&>(v).visit(*this);\
	}

ACCEPT_T(port_ref, spa::visitor)
ACCEPT(port_ref_base, spa::visitor)
ACCEPT_T(ringbuffer_in, spa::visitor)
ACCEPT(ringbuffer_in<char>, spa::visitor)

//! Base class for the spa plugin
class plugin
{
public:
	//! Must do one computation, depending on however this is defined
	//! E.g. if it's an audio plugin and has a sample count port, this
	//! should compute as many new floats for the out buffers
	virtual void run() = 0;

	//! The plugin must initiate all heavy variables
	virtual void init() {}
	//! Fast function to activate a plugin (RT)
	virtual void activate() {}
	virtual void deactivate() {}

#if 0
	/*
	* The following function is a notifier of events from the plugin.
	* They all return true(ish) values iff the specified event occurred.
	* In this case, they set the reference parameters to the event's
	* values and remove the event (such that it won't be queried again)
	*/
	virtual bool recent_savefile_operation(
	bool& load,
	const const char*& savefile,
	time_t& timestamp, bool& ok) = 0;
#endif

	//! TODO - currently still done via OSC (which requires osc ringbuffers)
	virtual int save(const char* savefile) { (void)savefile; return 0;
		/*TODO*/ }

	//! Comma seperated list of file formats we can load, e.g. "xiz,xmz"
	virtual const char* savefile_formats() const { return ""; }

	//! Destructor, must clean up any allocated memory
	virtual ~plugin() {}

	//! Return the port with name @p path (or throw port_not_found_error)
	virtual port_ref_base& port(const char* path) = 0;

	//! return whether the plugin has an external UI
	virtual bool ui_ext() const = 0;
	//! show or hide the external UI
	virtual void ui_ext_show(bool show) { (void)show; }

	//! Should return an XPM array for a preview logo, or nullptr
	virtual const char** xpm_load() const { return nullptr; }
};

//! Base class to let the host provide information without
//! it requiring to be started
class descriptor
{
public:
	//! License possibilities
	enum class license_type
	{
		gpl_3_0,  //!< GPL 3.0 or any later
		gpl_2_0,  //!< GPL 2.0 or any later
		lgpl_3_0, //!< LGPL 3.0 or any later
		lgpl_2_1  //!< LGPL 2.1 or any later
	};

	enum class hoster_t
	{
		gitlab,
		github,
		sourceforge,
		other
	};

	/*
	 * IDENTIFICATION OF THIS PLUGIN
	 * the comination of the following functions identify your plugin
	 * uniquely in the world. plugins should use them as identification
	 * for e.g. savefiles
	 */
	//! main hoster of your source (no mirrored one)
	virtual hoster_t hoster() const = 0;

	//! full url of you hoster, e.g. "https://github.com"
	//! @note only if hoster did not match any of your hosters
	virtual const char* hoster_other() const { return nullptr; }

	//! organisation or user shortcut for hosters, if any
	//! (e.g. github organisation or user)
	virtual const char* organization_url() const = 0;

	//! project for this plugin
	//! @note if multiple plugins share this plugin, they should have
	//!   the same project (and maybe the same descriptor subclass as
	//!   base class)
	virtual const char* project_url() const = 0;

	//! Plugin descriptor which will not change over time
	//! Should be unique inside your project
	//! (e.g. "sweep-filter-3")
	virtual const char* label() const = 0;

	// TODO: branch name?
	/*
	 * END OF IDENTIFICATION FOR THIS PLUGIN
	 */

	//! Project name, not abbreviated
	virtual const char* project() const = 0;

	//! Full name, not abbreviated (e.g. "Resonant sweep filter")
	virtual const char* name() const = 0;

	//! Author(s), comma separated, e.g.
	//! "firstname1 lastname1, firstname2 lastname2 <mail>"
	virtual const char* authors() const { return nullptr; }

	//! Organization(s), comma separated
	virtual const char* organizations() const { return nullptr; }

	//! License that the plugin is coded in
	virtual license_type license() const = 0;

	//! Describe in one line (<= 80 chars) what the plugin does
	virtual const char* description_line() const { return nullptr; }

	//! Describe in detail what the plugin does
	virtual const char* description_full() const { return nullptr; }

	//! Function that must return an allocated plugin
	virtual plugin* instantiate() const = 0;

	//! Desctructor, must clean up any allocated memory
	virtual ~descriptor() {}

	//! Return a port name. If all are returned, return nullptr.
	//! If state is 0, the plugin must be careful creating/removing
	//! new ports until it has returned nullptr
	//! @todo this generalises a bit bad, use 'port_after(const char*)'
	//!   or 'const char** port_array'
	//! @note The plugin must not show all its ports. A good start would be:
	//!   * cumpolsory ports (e.g. buffer sizes)
	//!   * ports that have a special meaning to the host and
	virtual const char* get_port_name(int state) const = 0;

	//! csv-list of files that can be loaded, e.g. "xmz, xiz"
	virtual const char* save_filetypes() const { return ""; }

	virtual int version_major() const { return 0; }
	virtual int version_minor() const { return 0; }
	virtual int version_patch() const { return 0; }

	struct properties
	{
		//! plugin has realtime dependency (e.g. hardware device),
		//! so its output may not be cached or subject to significant
		//! latency
		unsigned realtime_dependency:1;
		//! plugin makes no syscalls and uses no "slow algorithms"
		unsigned hard_rt_capable:1;
	} properties;
};

//! Function that must return a spa descriptor.
//! The argument must currently be 0 (TODO).
//! Entry point for any plugin.
typedef descriptor* (*descriptor_loader_t) (unsigned long);

} // namespace spa

#endif // SPA_SPA_H

