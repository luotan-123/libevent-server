
#pragma once 

#ifdef BOOST_LOG_DYN_LINK

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/trivial.hpp>                   
#include <boost/log/expressions.hpp>               
#include <boost/log/sources/severity_logger.hpp>   
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>     
#include <boost/log/utility/setup/common_attributes.hpp> 
#include <boost/log/support/date_time.hpp>
#include <boost/log/core/record.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;               
namespace expr = boost::log::expressions;          
namespace keywords = boost::log::keywords;         
namespace sinks = boost::log::sinks;
namespace attributes = boost::log::attributes;

#define FILE_T (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define BLT BOOST_LOG_TRIVIAL
#define LOG_DEBUG BLT(debug)<<" ("<<FILE_T<<" +"<<__LINE__<<") "<<__FUNCTION__<<" "
#define LOG_INFO BLT(info)<<" ("<<FILE_T<<" +"<<__LINE__<<") "<<__FUNCTION__<<" "
#define LOG_WARN BLT(warning)<<" ("<<FILE_T<<" +"<<__LINE__<<") "<<__FUNCTION__<<" "
#define LOG_ERROR BLT(error)<<" ("<<FILE_T<<" +"<<__LINE__<<") "<<__FUNCTION__<<" "
#define LOG_FATAL BLT(fatal)<<" ("<<FILE_T<<" +"<<__LINE__<<") "<<__FUNCTION__<<" "

static void logging_init(const std::string& logfile, bool debug) {

    logging::add_file_log(
            keywords::file_name = logfile + "-%Y_%m_%d.log",
            keywords::open_mode = (std::ios::out | std::ios::app),
            keywords::time_based_rotation=sinks::file::rotation_at_time_point(0,0,0),
            keywords::format =(expr::stream
                       << "["<<expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")<<"] "
                       << "<" << logging::trivial::severity<< ">"<<expr::smessage
            ),  
            keywords::auto_flush = true
        );  

    if(debug) {
        //logging::add_console_log(std::clog, keywords::format = "%TimeStamp%: %_%");
        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::trace);
    } else {
        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
    }

    logging::add_common_attributes();
}

#else

#include <iostream>

#include <stdlib.h>
#include <time.h>

enum log_rank_t {  
   DEBUG,
   INFO,  
   WARNING,  
   ERROR,  
   FATAL  
};
static const char* LOGNAME[] = {
    "DEBUG", 
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

class Logger {
public:  
    Logger(log_rank_t log_rank) : m_log_rank(log_rank) {};  
    ~Logger() {}
    std::ostream& start(log_rank_t log_rank,
                                const std::string& file,
                                const int32_t line,
                                const std::string& function) {
        time_t tm = time(NULL);
        char time_str[128] = {0};
        //char* pts = ctime_r(&tm, time_str);
        struct tm t40;
        struct tm* t4 = localtime_r(&tm, &t40);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t4);
        return stream(log_rank) <<std::endl
                                << "["<<time_str<<"] "
                                << LOGNAME[log_rank]
                                << " " <<file 
                                << " +" << line
                                << " (" <<function << ") "
                                << std::flush;
    }
private:  
    static std::ostream& stream(log_rank_t log_rank) {
        return std::cerr;
    }     
    log_rank_t m_log_rank;
};  

#define LOG(log_rank)   \
    Logger(log_rank).start(log_rank, __FILE__, __LINE__, __FUNCTION__)  

#define LOG_DEBUG   LOG(DEBUG)
#define LOG_INFO    LOG(INFO)
#define LOG_WARN    LOG(WARN)
#define LOG_WARNING LOG(WARNING)
#define LOG_ERROR   LOG(ERROR)
#define LOG_FATAL   LOG(FATAL)

static void logging_init(const std::string& logfile, bool debug) {
}


#endif
