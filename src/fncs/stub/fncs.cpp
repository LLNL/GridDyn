#include "fncs.hpp"

#include <map>

namespace fncs {

static std::map<std::string, std::string> container;

/** Connect to broker and parse config file. */
void initialize() {}

/** Connect to broker and parse inline configuration. */
void initialize(const string& /*configuration*/) {}

/** Check whether simulator is configured and connected to broker. */
bool is_initialized()
{
    return true;
}

/** Request the next time step to process. */
time time_request(time next)
{
    return next;
}

/** Publish value using the given key. */
void publish(const string& key, const string& value)
{
    container[key] = value;
}

/** Publish value using the given key, adding from:to into the key. */
void route(const string& /*from*/,
           const string& /*to*/,
           const string& /*key*/,
           const string& /*value*/)
{
}

/** Tell broker of a fatal client error. */
void die() {}

/** Close the connection to the broker. */
void finalize() {}

/** Update minimum time delta after connection to broker is made.
     * Assumes time unit is not changing. */
void update_time_delta(time /*delta*/) {}

/** Get the keys for all values that were updated during the last
     * time_request. */
vector<string> get_events()
{
    return std::vector<std::string>();
}

/** Get a value from the cache with the given key.
     * Will hard fault if key is not found. */
string get_value(const string& key)
{
    auto fnd = container.find(key);
    if (fnd != container.end()) {
        return fnd->second;
    } else {
        return "";
    }
}

/** Get a vector of values from the cache with the given key.
     * Will return a vector of size 1 if only a single value exists. */
vector<string> get_values(const string& key)
{
    auto fnd = container.find(key);
    if (fnd != container.end()) {
        return std::vector<std::string>{fnd->second};
    }
    return std::vector<std::string>();
}

/** Get a vector of configured keys. */
vector<string> get_keys()
{
    std::vector<std::string> keys;
    for (auto& key : container) {
        keys.push_back(key.first);
    }
    return keys;
}

/** Return the name of the simulator. */
string get_name()
{
    return "sim";
}

/** Return the minimum time delta of the simulator. */
time get_time_delta()
{
    return 1;
}

/** Return a unique numeric ID for the simulator. */
int get_id()
{
    return 1;
}

/** Return the number of simulators connected to the broker. */
int get_simulator_count()
{
    return 1;
}
/*  Run-time API version detection. */
void get_version(int* major, int* minor, int* patch)
{
    *major = 2;
    *minor = 1;
    *patch = 0;
}

void clear()
{
    container.clear();
}

}  // namespace fncs
