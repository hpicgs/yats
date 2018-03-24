#include <yats/options.h>
#include <yats/task_configurator.h>

using namespace yats;

int main()
{
    struct task
    {
        void run()
        {
        }

#ifdef SHOULD_FAIL
        options_map<task> options()
#else
        static options_map<task> options()
#endif
        {
            return {};
        }
    };

    task_configurator<task> configurator;
    return 0;
}
