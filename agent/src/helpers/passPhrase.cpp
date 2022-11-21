#include "passPhrase.h"
#include <fty_common_macros.h>
#include <regex>
#include <string>

namespace srr {

static constexpr auto PASS_PHRASE_FORMAT_REGEX = ".{8,}";

bool checkPassphraseFormat(const std::string& passphrase)
{
    std::regex reg(PASS_PHRASE_FORMAT_REGEX);

    return !passphrase.empty() && std::regex_search(passphrase, reg);
}

std::string getPassphraseFormat()
{
    return PASS_PHRASE_FORMAT_REGEX;
}

std::string getPassphraseFormatMessage()
{
    return TRANSLATE_ME("Passphrase must have 8 characters min.");
}

} // namespace fty
