#ifndef _FNCS_INTERNAL_H_
#define _FNCS_INTERNAL_H_
 
#include <cctype>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "czmq.h"

#include "yaml-cpp/yaml.h"
#include "fncs.hpp"
#include "log.h"

using ::std::endl;
using ::std::ostream;
using ::std::ostringstream;
using ::std::string;
using ::std::toupper;
using ::std::vector;


FNCS_EXPORT ostream& operator << (ostream& os, zframe_t *self);


namespace fncs {

    class FNCS_EXPORT Subscription {
        public:
            Subscription()
                : key("")
                , topic("")
                , def("")
                , type("")
                , list("")
            {}

            string key;
            string topic;
            string def;
            string type;
            string list;

            bool is_list() const {
                return toupper(list[0]) == 'T' || toupper(list[0]) == 'Y';
            }

            string to_string() {
                const string indent("  ");
                ostringstream os;
                os << indent << key << ":" << endl;
                os << indent << indent << "topic: " << topic << endl;
                if (!def.empty()) {
                    os << indent << indent << "default: " << def << endl;
                }
                if (!type.empty()) {
                    os << indent << indent << "type: " << type << endl;
                }
                if (!list.empty()) {
                    os << indent << indent << "list: " << list << endl;
                }
                return os.str();
            }
    };

    class FNCS_EXPORT Config {
        public:
            Config()
                : broker("")
                , name("")
                , time_delta("")
                , fatal("")
                , values()
            {}

            string broker;
            string name;
            string time_delta;
            string fatal;
            vector<Subscription> values;

            string to_string() {
                ostringstream os;
                if (!name.empty()) {
                    os << "name: " << name << endl;
                }
                if (!broker.empty()) {
                    os << "broker: " << broker << endl;
                }
                if (!time_delta.empty()) {
                    os << "time_delta: " << time_delta << endl;
                }
                if (!fatal.empty()) {
                    os << "fatal: " << fatal << endl;
                }
                if (values.size()) {
                    os << "values:" << endl;
                    for (size_t i=0; i<values.size(); ++i) {
                        os << values[i].to_string();
                    }
                }
                return os.str();
            }
    };

    const char * const HELLO = "hello";
    const char * const ACK = "ack";
    const char * const TIME_REQUEST = "time_request";
    const char * const PUBLISH = "publish";
    const char * const DIE = "die";
    const char * const BYE = "bye";
    const char * const TIME_DELTA = "time_delta";

    /** Connects to broker and parses the given config object. */
    FNCS_EXPORT void initialize(Config config);

    /** Starts the FNCS logger. */
    FNCS_EXPORT void start_logging();

    /** Retrieve the internal logging streams. */
    FNCS_EXPORT void replicate_logging(TLogLevel &level, FILE *& one, FILE *& two);

    /** Converts given time string, e.g., '1ms', into a fncs time value.
     * Ignores the value; only converts the unit into a multiplier. */
    FNCS_EXPORT fncs::time time_unit_to_multiplier(const string &value);

    /** Converts given time string, e.g., 1s, into a fncs time value. */
    FNCS_EXPORT fncs::time parse_time(const string &value);

    /** Converts given time value, assumed in ns, to sim's unit. */
    FNCS_EXPORT fncs::time convert_broker_to_sim_time(fncs::time value);

    /** Parses the given configuration string. */
    FNCS_EXPORT Config parse_config(const string &configuration);

    /** Parses the given YAML::Node object representing the document. */
    FNCS_EXPORT Config parse_config(const YAML::Node &doc);

    /** Converts given 'value' YAML::Node into a fncs Subscription value. */
    FNCS_EXPORT fncs::Subscription parse_value(const YAML::Node &node);

    /** Converts all 'values' YAML::Node items into fncs Subscription values. */
    FNCS_EXPORT vector<fncs::Subscription> parse_values(const YAML::Node &node);

    /** Parses the given zconfig object. */
    FNCS_EXPORT Config parse_config(zconfig_t *zconfig);

    /** Converts given 'value' zconfig into a fncs Subscription value. */
    FNCS_EXPORT fncs::Subscription parse_value(zconfig_t *config);

    /** Converts all 'values' zconfig items into fncs Subscription values. */
    FNCS_EXPORT vector<fncs::Subscription> parse_values(zconfig_t *config);

    /** Converts given czmq frame into a string. */
    FNCS_EXPORT string to_string(zframe_t *frame);

    /** Publish value anonymously using the given key. */
    FNCS_EXPORT void publish_anon(const string &key, const string &value);

    /** Current time in seconds with nanosecond precision. */
    FNCS_EXPORT double timer();

    /** Current time as a fncs::time in nanoseconds. */
    FNCS_EXPORT fncs::time timer_ft();
}

#endif /* _FNCS_INTERNAL_H_ */
