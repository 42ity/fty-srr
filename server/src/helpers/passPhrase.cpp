#include "passPhrase.h"
#include <fty_common_macros.h>
#include <regex>

namespace fty {

static constexpr const char* PASS_PHRASE_FORMAT_REGEX = ".{8,}";
static constexpr const char* PASS_PHRASE_MESSAGE      = "Passphrase must have 8 characters";

bool checkPassphraseFormat(const std::string& passphrase)
{
    std::regex reg(PASS_PHRASE_FORMAT_REGEX);

    return !passphrase.empty() && std::regex_search(passphrase, reg);
}

std::string getPassphraseFormat()
{
    return std::string(PASS_PHRASE_FORMAT_REGEX);
}

std::string getPassphraseFormatMessage()
{
    return TRANSLATE_ME(PASS_PHRASE_MESSAGE);
}

} // namespace fty
