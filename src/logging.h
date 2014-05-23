#ifndef RAFT_LOGGING
#define RAFT_LOGGING

#define BOOST_LOG_DYN_LINK

#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/severity_feature.hpp>

namespace trivial = boost::log::trivial;

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(logger, boost::log::sources::severity_logger<trivial::severity_level>)

#define LOG_TRACE BOOST_LOG_SEV(logger::get(), trivial::trace)
#define LOG_INFO BOOST_LOG_SEV(logger::get(), trivial::info)
#define LOG_WARN BOOST_LOG_SEV(logger::get(), trivial::warning)
#define LOG_ERROR BOOST_LOG_SEV(logger::get(), trivial::error)
#define LOG_FATAL BOOST_LOG_SEV(logger::get(), trivial::fatal)

#endif
