#ifndef RAFT_LOGGING
#define RAFT_LOGGING

#define BOOST_LOG_DYN_LINK

#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>

namespace trivial = boost::log::trivial;

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(logger, boost::log::sources::severity_logger<trivial::severity_level>)

#define LOG_TRACE BOOST_LOG_SEV(logger::get(), trivial::trace)
#define LOG_INFO BOOST_LOG_SEV(logger::get(), trivial::info) << "\t"
#define LOG_WARN BOOST_LOG_SEV(logger::get(), trivial::warning)
#define LOG_ERROR BOOST_LOG_SEV(logger::get(), trivial::error)
#define LOG_FATAL BOOST_LOG_SEV(logger::get(), trivial::fatal)

inline void init_log() {
  using namespace boost::log;
  namespace expr = boost::log::expressions;

  add_common_attributes();
  boost::log::add_console_log(std::clog, boost::log::keywords::format =
     expressions::stream << "" 
     << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%H:%M:%S.%f") << " " 
     << expr::attr<attributes::current_thread_id::value_type>("ThreadID") << " " 
     << boost::log::trivial::severity << "\t" 
     << expr::smessage);
}

#endif
