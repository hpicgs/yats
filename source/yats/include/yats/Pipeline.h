#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <yats/TaskConfigurator.h>

namespace yats
{

class Pipeline
{
public:

	Pipeline() = default;

	template <typename Node>
	TaskConfigurator<Node>* add(const std::string &name)
	{
		m_nodes[name] = std::make_unique<TaskConfigurator<Node>>();
		return static_cast<TaskConfigurator<Node>*>(m_nodes[name].get());
	}

	void run()
	{
		std::vector<std::unique_ptr<AbstractTaskContainer>> nodes;
		for (auto &elem : m_nodes)
		{
			nodes.emplace_back(elem.second->make());
		}
	}

private:

	std::map<std::string, std::unique_ptr<AbstractTaskConfigurator>> m_nodes;
};

}  // namespace yats
