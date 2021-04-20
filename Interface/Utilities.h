#ifndef RADIATIONINTERFACE_UTILITIES_H
#define RADIATIONINTERFACE_UTILITIES_H

#include <string>

void HelpTooltip(const std::string& text);
void HelpMarker(const std::string& text);
bool FontAwesomeButton(const char * text);
void FontAwesomeText(const char *text);
bool PopupModal(const std::string & name, const std::string & prompt);

#endif //RADIATIONINTERFACE_UTILITIES_H
