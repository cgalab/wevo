#ifdef ENABLE_LOG

#include "log.h"

/*
void logging_function()
{
    src::severity_logger< severity_level > slg;

    BOOST_LOG_SEV(slg, normal) << "A regular message";
    BOOST_LOG_SEV(slg, warning) << "Something bad is going on but I can handle it";
    BOOST_LOG_SEV(slg, critical) << "Everything crumbles, shoot me now!";
}


void named_scope_logging()
{
    BOOST_LOG_NAMED_SCOPE("named_scope_logging");

    src::severity_logger< severity_level > slg;

    BOOST_LOG_SEV(slg, normal) << "Hello from the function named_scope_logging!";
}

void tagged_logging()
{
    src::severity_logger< severity_level > slg;
    slg.add_attribute("Tag", attrs::constant< std::string >("My tag value"));

    BOOST_LOG_SEV(slg, normal) << "Here goes the tagged record";
}

void timed_logging()
{
    BOOST_LOG_SCOPED_THREAD_ATTR("Timeline", attrs::timer());

    src::severity_logger< severity_level > slg;
    BOOST_LOG_SEV(slg, normal) << "Starting to time nested functions";

    logging_function();

    BOOST_LOG_SEV(slg, normal) << "Stopping to time nested functions";
} 
*/

std::ostream& operator<< (std::ostream& strm, severity_level level)
{
    static const char* strings[] =
    {
        "normal",
        "notification",
        "warning",
        "error",
        "critical"
    };

    if (static_cast<std::size_t>(level) < sizeof(strings) / sizeof(*strings)) {
        strm << strings[level];
    } else {
        strm << static_cast< int >(level);
    }

    return strm;
}

void Log::init() {
    typedef sinks::synchronous_sink<sinks::text_ostream_backend> text_sink;
    boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();

    sink->locked_backend()->add_stream(boost::make_shared<std::ofstream>("log/summary.log"));

    sink->set_formatter
            (
             expr::stream
             << std::hex << std::setw(8) << std::setfill('0') << line_id << std::dec << std::setfill(' ')
             << ": <" << severity << ">\t"
             << "(" << scope << ") "
             << expr::if_(expr::has_attr(tag_attr))
             [
             expr::stream << "[" << tag_attr << "] "
             ]
             << expr::if_(expr::has_attr(timeline))
             [
             expr::stream << "[" << timeline << "] "
             ]
             << expr::smessage
             );

    logging::core::get()->add_sink(sink);
    logging::add_common_attributes();
    logging::core::get()->add_global_attribute("Scope", attrs::named_scope());
}

void Log::disable() {
    boost::log::core::get()->set_logging_enabled(false);
}

#endif
