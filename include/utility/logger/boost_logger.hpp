#ifndef utility_include_utility_logger_boost_logger_hpp
#define utility_include_utility_logger_boost_logger_hpp

#include <fstream>
#include <mutex>
#include "boost/filesystem.hpp"
#include "boost/log/core.hpp"
#include "boost/log/trivial.hpp"
#include "boost/log/expressions.hpp"
#include "boost/log/sinks.hpp"
#include "boost/log/utility/setup.hpp"
#include "boost/log/sources/severity_logger.hpp"
#include "boost/log/sources/record_ostream.hpp"

namespace diy
{
namespace utility
{
/**
 * logger defined
 */
#define UtilityBoostLogger(lvl) BOOST_LOG_TRIVIAL(lvl)

/**
 * boost log
 */
namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

class BoostLogger
{
public:
    static bool InitLoggerEnv(std::string conf_file)
    {
        using namespace logging::trivial;

        logging::add_common_attributes();
        logging::register_simple_formatter_factory<severity_level, char>("Severity");
        logging::register_simple_formatter_factory<severity_level, char>("Channel");
        logging::register_simple_formatter_factory<severity_level, char>("Tag");
        logging::register_simple_formatter_factory<severity_level, char>("LineID");
        logging::register_simple_formatter_factory<severity_level, char>("ThreadID");
        //logging::register_simple_formatter_factory<severity_level, char>("Scope");
        //logging::register_simple_formatter_factory<severity_level, char>("TimeLine");
        logging::register_simple_filter_factory<severity_level, char>("Severity");
        logging::register_simple_filter_factory<severity_level, char>("Channel");
        logging::register_simple_filter_factory<severity_level, char>("Tag");
        logging::register_simple_filter_factory<severity_level, char>("LineID");
        logging::register_simple_filter_factory<severity_level, char>("ThreadID");
        //logging::register_simple_filter_factory<severity_level, char>("Scope");
        //logging::register_simple_filter_factory<severity_level, char>("TimeLine");

        std::ifstream conf_ifs(conf_file);
        if (conf_ifs.is_open())
        {
            try
            {
                logging::init_from_stream(conf_ifs);
            }
            catch (const std::exception &ep)
            {
                std::cout << "InitLoggerEnv failed err:" << ep.what() << std::endl;
                return false;
            }
            conf_ifs.close();
        }
        else
        {
            return true;
        }
        return true;
    }

private:
};
} //namespace utility
} //namespace diy

#endif //!utility_include_utility_logger_boost_logger_hpp