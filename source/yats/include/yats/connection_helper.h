#pragma once

#include <map>
#include <memory>
#include <set>

#include <yats/class_name.h>
#include <yats/input_connector.h>
#include <yats/output_connector.h>
#include <yats/task_helper.h>

namespace yats
{

/**
 * Helper class to determine the ids of inputs and outputs of a task. This class could not be created directly.
 * Call abstract_connection_helper.get_io_id_helper() to receive an instance.
 */
class io_id_helper
{
public:
    /**
     * @param input_ids Vector with input ids. Index of id in vector must correspond to the position of the input
     * in the parameter list of the run function. 
     * @param output_ids Vector with output ids. Index of id in vector must correspond to the position of the output
     * in the return value of the run function. 
     */
    io_id_helper(std::vector<uint64_t>&& input_ids, std::vector<uint64_t>&& output_ids)
        : m_input_ids(std::move(input_ids))
        , m_output_ids(std::move(output_ids))
    {
    }

    /**
    * Returns the id of the input connector with {@code index}
    * of the run function.
    * @param index Index of input connector in run functions which id is to be returned.
    * @return Id;
    */
    uint64_t input_id(size_t index) const
    {
        return m_input_ids[index];
    }

    /**
    * Returns the id of the output connector with {@code index}
    * of the return value of the run function.
    * @param index Index of output connector in the return value of the run function.
    * @return Id;
    */
    uint64_t output_id(size_t index) const
    {
        return m_output_ids[index];
    }

protected:
    std::vector<uint64_t> m_input_ids;
    std::vector<uint64_t> m_output_ids;
};

class abstract_connection_helper
{
public:
    template <typename Type>
    using locations = std::map<const Type*, size_t>;

    /**
     * @param input Map <abstract_input_connector, index> where index is the index of the parameter in the run function
     * @param output Map <abstract_input_connector, index> where index is the index of the slot in the return value of the the run function
     */
    abstract_connection_helper(locations<abstract_input_connector> input, locations<abstract_output_connector> output)
        : m_in(input)
        , m_out(output)
    {
    }

    virtual ~abstract_connection_helper() = default;

    abstract_connection_helper(const abstract_connection_helper& other) = delete;
    abstract_connection_helper(abstract_connection_helper&& other) = delete;

    abstract_connection_helper& operator=(const abstract_connection_helper& other) = delete;
    abstract_connection_helper& operator=(abstract_connection_helper&& other) = delete;

    virtual void bind(const abstract_output_connector* connector, void* callback) = 0;
    virtual void* target(const abstract_input_connector* connector) = 0;

    /**
     * Returns the name of the task associated with the helper.
     */
    virtual std::string task_name() const = 0;

    const auto& inputs() const
    {
        return m_in;
    }

    const auto& outputs() const
    {
        return m_out;
    }

    /**
     * Returns the index of {@code output_connector} in the returned tuple
     * of the run function.
     */
    size_t output_index(const abstract_output_connector* output_connector) const
    {
        return m_out.at(output_connector);
    }

    void add_following(size_t following_node)
    {
        m_following.insert(following_node);
    }

    /**
     * Creates and returns a new instance of io_id_helper.
     */
    virtual io_id_helper create_io_id_helper() const = 0;

protected:
    template <typename LocationType, typename SequenceType, size_t... index>
    static locations<LocationType> map(const SequenceType& outputs, std::index_sequence<index...>)
    {
        // Prevent a warning about unused parameter when handling a run function with no parameters.
        (void) outputs;
        return { std::make_pair(&std::get<index>(outputs), index)... };
    }

    std::set<size_t> m_following;
    const locations<abstract_input_connector> m_in;
    const locations<abstract_output_connector> m_out;
};

template <typename Task>
class connection_helper : public abstract_connection_helper
{
public:
    using helper = decltype(make_helper(&Task::run));
    using input_callbacks = typename helper::input_callbacks;
    using input_connectors = typename helper::input_connectors;
    using input_queue = typename helper::input_queue;
    using input_queue_ptr = typename helper::input_queue_ptr;
    using output_connectors = typename helper::output_connectors;
    using output_callbacks = typename helper::output_callbacks;
    using output_tuple = typename helper::output_tuple;
    using input_tuple = typename helper::input_tuple;

    using input_sequence = std::make_index_sequence<std::tuple_size<input_connectors>::value>;
    using output_sequence = std::make_index_sequence<std::tuple_size<output_connectors>::value>;

    connection_helper(const input_connectors& inputs, const output_connectors& outputs, output_callbacks listeners)
        : abstract_connection_helper(map<abstract_input_connector>(inputs, input_sequence()), map<abstract_output_connector>(outputs, output_sequence()))
        , m_input(std::make_unique<input_queue>())
        , m_output(std::move(listeners))
        , m_callbacks(generate_callbacks(m_input, std::make_index_sequence<helper::input_count>()))
    {
    }

    void bind(const abstract_output_connector* connector, void* callback) override
    {
        auto location_id = m_out.at(connector);
        add(location_id, callback);
    }

    void* target(const abstract_input_connector* connector) override
    {
        auto location_id = m_in.at(connector);
        return get(location_id);
    }

    typename helper::input_queue_ptr queue()
    {
        return std::move(m_input);
    }

    typename helper::output_callbacks callbacks()
    {
        return std::move(m_output);
    }

    std::set<size_t> following_nodes()
    {
        return std::move(m_following);
    }

    /**
    * Returns the name of the task associated with the helper.
    * @return Task name
    */
    std::string task_name() const override
    {
        return class_name::get<Task>();
    }

    /**
    * Creates and returns a new instance of io_id_helper.
    */
    io_id_helper create_io_id_helper() const override
    {
        return io_id_helper(input_ids(), output_ids());
    };

protected:
    template <size_t... Index>
    static input_callbacks generate_callbacks(input_queue_ptr& queue, std::index_sequence<Index...>)
    {
        // Prevent a warning about unused parameter when handling a run function with no parameters.
        (void) queue;
        return std::make_tuple(generate_callback<Index>(queue)...);
    }

    template <size_t Index>
    static typename std::tuple_element_t<Index, input_callbacks> generate_callback(input_queue_ptr& queue)
    {
        using parameter_type = typename std::tuple_element_t<Index, input_queue>::value_type;
        return [&current = std::get<Index>(*queue)](parameter_type input) mutable {
            current.push(std::move(input));
        };
    }

    template <size_t Index = 0>
    std::enable_if_t<(Index < helper::output_count)> add(size_t location_id, void* raw_callback)
    {
        if (Index == location_id)
        {
            using parameter = std::tuple_element_t<Index, output_tuple>;
            auto callback = static_cast<std::function<void(parameter)>*>(raw_callback);
            std::get<Index>(m_output).push_back(*callback);
        }
        else
        {
            add<Index + 1>(location_id, raw_callback);
        }
    }

    template <size_t Index = 0>
    std::enable_if_t<Index == helper::output_count> add(size_t, void*)
    {
        throw std::runtime_error("Output Parameter locationId not found. It is not allowed to connect task from different pipelines.");
    }

    template <size_t Index = 0>
    std::enable_if_t<(Index < helper::input_count), void*> get(size_t location_id)
    {
        if (Index == location_id)
        {
            return &std::get<Index>(m_callbacks);
        }
        else
        {
            return get<Index + 1>(location_id);
        }
    }

    template <size_t Index = 0>
    std::enable_if_t<Index == helper::input_count, void*> get(size_t)
    {
        throw std::runtime_error("Input parameter locationId not found. This implies an implementation error in yats.");
    }

    /**
    * Iterates through all outputs of the run function and stores the id of each output
    * in the returned vector. The index of the id corresponds to the position of the output
    * in the return value of the run function.
    * @return Vector with output ids. Index of id in vector corresponds to the position of the output
    * in the return value of the run function. 
    */
    std::vector<uint64_t> output_ids() const
    {
        std::vector<uint64_t> output_ids;
        parse_output_ids(output_ids);
        return output_ids;
    }

    /**
    * Iterates through all inputs of the run function and stores the id of each input
    * in the returned vector. The index of the id corresponds to the position of the input
    * parameter of the run function.
    * @return Vector with input ids. Index of id in vector corresponds to the position of the input
    * parameter of the run function.
    */
    std::vector<uint64_t> input_ids() const
    {
        std::vector<uint64_t> input_ids;
        parse_input_ids(input_ids);
        return input_ids;
    }

    /**
    * Iterates through all inputs of the run function and stores the id of each input
    * in {@code ids}. The index of the id corresponds to the position of the input
    * in the parameters of the run function.
    * @param ids Vector to which the input ids will be saved to.
    */
    template <size_t Index = 0>
    std::enable_if_t<Index < helper::input_count> parse_input_ids(std::vector<uint64_t>& ids) const
    {
        ids.push_back(std::tuple_element_t<Index, input_tuple>::id);
        parse_input_ids<Index + 1>(ids);
    }

    /**
     * End of recursion to determine the ids of the input paramaters
     */
    template <size_t Index = 0>
    std::enable_if_t<Index == helper::input_count> parse_input_ids(std::vector<uint64_t>&) const
    {
    }
    
    /**
    * Iterates through all outputs of the run function and stores the id of each output
    * in {@code ids}. The index of the id corresponds to position of the output
    * in the return value of the run function.
    *  @param ids Vector to which the output ids will be saved to.
    */
    template <size_t Index = 0>
    std::enable_if_t<Index < helper::output_count> parse_output_ids(std::vector<uint64_t>& ids) const
    {
        ids.push_back(std::tuple_element_t<Index, output_tuple>::id);
        parse_output_ids<Index + 1>(ids);
    }

    /**
    * End of recursion to determine the ids of the output paramaters
    */
    template <size_t Index = 0>
    std::enable_if_t<Index == helper::output_count> parse_output_ids(std::vector<uint64_t>&) const
    {
    }

    input_queue_ptr m_input;
    output_callbacks m_output;
    input_callbacks m_callbacks;
};
}
