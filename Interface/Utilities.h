#ifndef RADIATIONINTERFACE_UTILITIES_H
#define RADIATIONINTERFACE_UTILITIES_H

#include <string>
#include "cereal/cereal.hpp"
#include <cereal/archives/json.hpp>
#include <boost/algorithm/string.hpp>

void HelpTooltip(const std::string& text);
void HelpMarker(const std::string& text);
bool FontAwesomeButton(const char * text);
void FontAwesomeText(const char *text);
bool PopupModal(const std::string & name, const std::string & prompt);

template<typename Class>
std::string ArchiveDump(const Class & archivable, const std::string & name = "value") {
    std::ostringstream ss;
    cereal::JSONOutputArchive archive(ss, cereal::JSONOutputArchive::Options::NoIndent());
    archive(cereal::make_nvp(name, archivable));

    return boost::replace_all_copy(ss.str(), "\n", " ");
}

#endif //RADIATIONINTERFACE_UTILITIES_H
