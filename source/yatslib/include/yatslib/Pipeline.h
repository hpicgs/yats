#pragma once

#include <yatslib/yatslib_api.h>
#include <yatslib/Nodeconfigurator.h>

#include <map>
#include <memory>
#include <string>

class Pipeline
{
public:

	Pipeline() = default;

	template <typename Node>
	Nodeconfigurator<Node>* add(const std::string &name)
	{
		m_nodes[name] = std::make_unique<Nodeconfigurator<Node>>();
		return static_cast<Nodeconfigurator<Node>*>(m_nodes[name].get());
	}

private:

	std::map<std::string, std::unique_ptr<AbstractNodeconfigurator>> m_nodes;
};
