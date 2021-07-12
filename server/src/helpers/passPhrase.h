#pragma once
#include <string>

namespace fty {

/**
 * Check pass phraseFormat
 * @param phassphrase
 * @return True if the passphrase is Ok, otherwise false.
 */
bool checkPassphraseFormat(const std::string& passphrase);

/**
 * Get passphrase format
 * @param phassphrase
 * @return The passphrase format
 */
std::string getPassphraseFormat();

/**
 * Get passphrase format message
 * @return The passphrase format message
 */
std::string getPassphraseFormatMessage();

} // namespace fty
