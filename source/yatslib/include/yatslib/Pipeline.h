#pragma once

#include <yatslib/Nodeconfigurator.h>

#include <map>
#include <memory>
#include <string>
#include <vector>


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

	void run()
	{
		std::vector<std::unique_ptr<AbstractNodecontainer>> nodes;
		for (auto &elem : m_nodes)
		{
			nodes.emplace_back(elem.second->make());
		}
	}

private:

	std::map<std::string, std::unique_ptr<AbstractNodeconfigurator>> m_nodes;
};
