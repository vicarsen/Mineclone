#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include "mineclonelib/log.h"

DECLARE_LOG_CATEGORY(CVars);

namespace mc
{
template <typename tp> class cvar;

template <typename tp> class cvars {
    public:
	using value_type = tp;
	using cvar_type = cvar<tp>;
	using cvars_type = cvars<tp>;

	static cvars_type *get()
	{
		static cvars_type cvars;
		return &cvars;
	}

	void register_cvar(const char *name, cvar_type *cvar)
	{
#ifndef CVAR_NWARN_ON_OVERWRITE
		LOG_WARN_IF(CVars, m_cvars.find(name) != m_cvars.end(),
			    "CVar {} redeclared", name);
#endif

		m_cvars[name] = cvar;
	}

	cvar_type *find(const char *name) const
	{
		auto it = m_cvars.find(name);
		if (it == m_cvars.end()) {
			return nullptr;
		}

		return it->second;
	}

    private:
	~cvars() = default;

    private:
	std::unordered_map<std::string, cvar_type *> m_cvars;
};

template <typename tp> class cvar {
    public:
	using value_type = tp;

	cvar(value_type v, const char *name, const char *desc)
		: m_v(v)
		, m_name(name)
		, m_desc(desc)
	{
		cvars<value_type>::get()->register_cvar(name, this);
	}

	~cvar() = default;

	void add_callback(std::function<void(value_type, value_type)> callback)
	{
		m_callbacks.emplace_back(callback);
	}

	void set(value_type v)
	{
		for (std::size_t i = 0; i < m_callbacks.size(); i++) {
			m_callbacks[i](v, m_v);
		}

		m_v = v;
	}

	inline value_type get() const noexcept
	{
		return m_v;
	}

    private:
	value_type m_v;
	std::string m_name;
	std::string m_desc;

	std::vector<std::function<void(value_type, value_type)> > m_callbacks;
};
}
