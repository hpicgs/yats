#pragma once

#include <algorithm>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <yats/pipeline.h>
#include <yats/task_configurator.h>

namespace yats
{
/*implements round robin at first*/
class scheduler_adv
{
public:
    explicit scheduler_adv(const pipeline& pipeline)
        : m_tasks(pipeline.build())
    {
    }

    scheduler_adv(const scheduler_adv& other) = delete;
    scheduler_adv(scheduler_adv&& other) = delete;

    ~scheduler_adv() = default;

    scheduler_adv& operator=(const scheduler_adv& other) = delete;
    scheduler_adv& operator=(scheduler_adv&& other) = delete;

    void run()
    {
        // Erstmal möchte ich alle ausführbaren Tasks
        // Das sind alle Tasks, bei denen can_run = true ist,
        // bzw. alle Tasks, die keine Eingabe erwarten.
        // Topologische Sortierung.
        // Der Einfachheit halber soll can_run erstmal im Task selber prüfen,
        // ob er ausgeführt werden kann.
        // Diese Aufgabe wird als nächstes an der Scheduler übergeben. Dieser prüft dann,
        // ob alle Eingaben für den Task vorliegen.
        // Ebenfalls hätte ich im Task gerne die Mitteilung, dass er alles abgearbeitet hat,
        // also fertig ist.
        std::vector<abstract_task_container*> to_run;
        for (auto& elem : m_tasks)
        {
            to_run.push_back(elem.get());
        }

        std::function<void()> check_runnable;

        check_runnable = [&to_run, &check_runnable, this]() {
            std::lock_guard<std::mutex> lock(m_mutex);

            while (true)
            {
                // Prüfen, ob es mindestens einen ausführbaren Task gibt.
                auto runnable = std::find_if(to_run.begin(), to_run.end(), [](abstract_task_container* task) {
                    return task->can_run();
                });

                if (runnable == to_run.end())
                {
                    return;
                }
                auto task = *runnable;
                to_run.erase(runnable);

                m_threads.emplace_back([&to_run, &check_runnable, task]() {
                    task->run();
                    check_runnable();
                });
            }
        };

        check_runnable();
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (to_run.empty())
                {
                    break;
                }
            }
        }

        for (auto& thread : m_threads)
        {
            thread.join();
        }
    }

protected:
    // Stores all task_containers with their position as an implicit id
    std::vector<std::unique_ptr<abstract_task_container>> m_tasks;
    std::vector<std::thread> m_threads;
    std::mutex m_mutex;
};
}
