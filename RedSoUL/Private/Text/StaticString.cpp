/// Library headers
#include "Assert/RuntimeAssert.hpp" /// RUNTIME_ASSERT
#include "Text/StaticStringDepot.hpp"
/// Self header
#include "Text/StaticString.hpp"


StaticStringIdT
StaticString::get_empty_string_id ()
{
    return StaticStringDepot::ref().get_empty_string_id();
}


StaticString::StaticString ()
:
    m_string_id(get_empty_string_id())
#if (BUILD_MODE == DEBUG_BUILD_MODE)
   ,m_cached_string(data())
#endif
{

}


StaticString::StaticString (
    const StaticStringIdT string_id)
:
    m_string_id(string_id)
#if (BUILD_MODE == DEBUG_BUILD_MODE)
   ,m_cached_string(data())
#endif
{
    RUNTIME_ASSERT(string_id.is_valid(), "Invalid string Id!!");
    RUNTIME_ASSERT(StaticStringDepot::ref().is_cached_string_id(string_id),
                   "No an Id of any cached string. "
                   "please cache the string at first!!");
}


StaticString::StaticString (
    const char * const ascii_text)
:
    m_string_id(StaticStringDepot::ref().cache_string(ascii_text))
#if (BUILD_MODE == DEBUG_BUILD_MODE)
   ,m_cached_string(data())
#endif
{

}


StaticString::StaticString (
    const uint8_t * const utf8_text)
:
    m_string_id(StaticStringDepot::ref().cache_string(utf8_text))
#if (BUILD_MODE == DEBUG_BUILD_MODE)
   ,m_cached_string(data())
#endif
{

}


StaticStringIdT
StaticString::id () const
{
    return m_string_id;
}


uint16_t
StaticString::length () const
{
    return StaticStringDepot::ref().length(m_string_id);
}


const uint8_t *
StaticString::data() const
{
    return StaticStringDepot::ref().data(m_string_id);
}


bool
StaticString::operator== (
    const StaticString other) const
{
    return m_string_id == other.m_string_id;
}


bool StaticString::operator!= (
    const StaticString other) const
{
    return m_string_id != other.m_string_id;
}
