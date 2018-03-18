#include <yats/constraint.h>
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
        thread_group thread_constraints()
#else
        static thread_group thread_constraints()
#endif
        {
            return thread_group();
        }
    };

    task_configurator<task> configurator;
    return 0;
}