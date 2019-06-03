#include <mbgl/util/platform.hpp>
#include <mbgl/util/string.hpp>
#include <libnu/casemap.h>
#include <cstring>
#include <sstream>
#include <locale>
#include <iomanip>
#include <cmath>
#include <stdexcept>

namespace mbgl { namespace platform {

std::string uppercase(const std::string& str)
{
    std::stringstream output;
    char const *itr = str.c_str(), *nitr;
    char const *end = itr + str.length();
    char lo[5] = { 0 };

    for (; itr < end; itr = nitr)
    {
        uint32_t code_point = 0;
        char const* buf = nullptr;

        nitr = _nu_toupper(itr, end, nu_utf8_read, &code_point, &buf, nullptr);
        if (buf != nullptr)
        {
            do
            {
                buf = NU_CASEMAP_DECODING_FUNCTION(buf, &code_point);
                if (code_point == 0) break;
                output.write(lo, nu_utf8_write(code_point, lo) - lo);
            }
            while (code_point != 0);
        }
        else
        {
            output.write(itr, nitr - itr);
        }
    }

    return output.str();

}

std::string lowercase(const std::string& str)
{
    std::stringstream output;
    char const *itr = str.c_str(), *nitr;
    char const *end = itr + str.length();
    char lo[5] = { 0 };

    for (; itr < end; itr = nitr)
    {
        uint32_t code_point = 0;
        char const* buf = nullptr;

        nitr = _nu_tolower(itr, end, nu_utf8_read, &code_point, &buf, nullptr);
        if (buf != nullptr)
        {
            do
            {
                buf = NU_CASEMAP_DECODING_FUNCTION(buf, &code_point);
                if (code_point == 0) break;
                output.write(lo, nu_utf8_write(code_point, lo) - lo);
            }
            while (code_point != 0);
        }
        else
        {
            output.write(itr, nitr - itr);
        }
    }

    return output.str();
}

std::string formatNumber(double number,
                         const std::string& localeId,
                         const std::string& currency,
                         uint8_t minFractionDigits,
                         uint8_t maxFractionDigits)
{
    std::stringstream output;
    std::locale loc;
    bool localeSet;

    // Replace '-' with '_' in the locale string and append '.utf8' to it
    std::string localeStr = localeId;
    if (!localeStr.empty()) {
        localeStr.replace(localeStr.find_first_of("-"), 1, "_");
        localeStr.append(".utf8");
    }

    try {
        loc = std::locale(localeStr.c_str());
        localeSet = true;
    }
    catch (std::runtime_error& e) {
        loc = std::locale("");
        localeSet = false;
    }
    output.imbue(loc);

    // Print the value as currency
    if (!currency.empty()) {
        // If locale was successfully set print the input value formatted as money and
        // with currency symbol, otherwise print the input value with currency string.
        if (localeSet) {
            const auto& mp = std::use_facet<std::moneypunct<char>>(loc);
            int fracDigits = mp.frac_digits();
            // Multiply with 10^fracDigits to print the value correctly
            if (fracDigits) {
                number = number * pow(10, fracDigits);
            }
            output << std::showbase << std::put_money(number);
        } else {
            output << currency << " " << util::toString(number);
        }
    } else {
        unsigned int minPrecision = 0, maxPrecision = 3; //defaults
        auto integerPart = static_cast<long>(number);
        std::string fractPartStr;

        if (minFractionDigits > 0) {
            minPrecision = minFractionDigits;
        }
        if (maxFractionDigits > 0) {
            maxPrecision = maxFractionDigits;
        }

        if (maxPrecision != 0) {
            double fractPart = number - integerPart;
            fractPartStr = std::to_string(fractPart).substr(2, std::string::npos);
            // Erase possible trailing zeros in the decimal part
            fractPartStr.erase(fractPartStr.find_last_not_of('0') + 1, std::string::npos);
            std::size_t fractLen = fractPartStr.length();

            auto numDecimals = static_cast<unsigned int>(fractLen);
            if (numDecimals < minPrecision) {
                // Append extra zeroes to the decimal part in case it is too short
                fractPartStr.append(minPrecision - fractLen, '0');
            } else if (numDecimals > maxPrecision) {
                // Shorten the decimal part to the requested precision in case it is too long
                fractPart = round(fractPart * pow(10, maxPrecision));
                fractPart = fractPart / pow(10, maxPrecision);
                fractPartStr = std::to_string(fractPart).substr(2, std::string::npos);
                fractPartStr.resize(maxPrecision);

                // Increment the integer part if the fractional part rounds up to 1
                if (fractPart >= 1) {
                    integerPart++;
                }
            }
            char point = std::use_facet<std::numpunct<char>>(loc).decimal_point();
            std::string pointStr = std::string(&point, 1);
            fractPartStr.insert(0, pointStr);
        } else {
            // Round the value to the nearest integer in case no decimals are requested
            integerPart = round(number);
        }
        output << integerPart << fractPartStr;
    }

    if (output.good()) {
        return output.str();
    }
    return std::string("Could not evaluate number-format input to string");
}

} // namespace platform
} // namespace mbgl
