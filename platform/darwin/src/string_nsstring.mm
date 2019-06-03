#import <Foundation/Foundation.h>

#include <mbgl/util/platform.hpp>

namespace mbgl {
namespace platform {

std::string uppercase(const std::string &string) {
    NSString *original = [[NSString alloc] initWithBytesNoCopy:const_cast<char *>(string.data())
                                                        length:string.size()
                                                      encoding:NSUTF8StringEncoding
                                                  freeWhenDone:NO];
    NSString *uppercase = [original uppercaseString];
    const std::string result{[uppercase cStringUsingEncoding : NSUTF8StringEncoding],
                             [uppercase lengthOfBytesUsingEncoding:NSUTF8StringEncoding]};
    return result;
}

std::string lowercase(const std::string &string) {
    NSString *original = [[NSString alloc] initWithBytesNoCopy:const_cast<char *>(string.data())
                                                        length:string.size()
                                                      encoding:NSUTF8StringEncoding
                                                  freeWhenDone:NO];
    NSString *lowercase = [original lowercaseString];
    const std::string result{[lowercase cStringUsingEncoding : NSUTF8StringEncoding],
                             [lowercase lengthOfBytesUsingEncoding:NSUTF8StringEncoding]};
    return result;
}

std::string formatNumber(double number,
                         const std::string& localeId,
                         const std::string& currency,
                         uint8_t minFractionDigits,
                         uint8_t maxFractionDigits)
{
    NSNumberFormatter *_numberFormatter;
    _numberFormatter = [[NSNumberFormatter alloc] init];
    _numberFormatter.locale = !localeId.empty() ? [NSLocale localeWithLocaleIdentifier:@(localeId.c_str())] : nil;
    _numberFormatter.currencyCode = !currency.empty() ? @(currency.c_str()) : nil;
    if (currency.empty()) {
        _numberFormatter.minimumFractionDigits = minFractionDigits;
        _numberFormatter.maximumFractionDigits = maxFractionDigits;
    }
    _numberFormatter.numberStyle = !currency.empty() ? NSNumberFormatterCurrencyStyle : NSNumberFormatterDecimalStyle;
    NSString *formatted = [_numberFormatter stringFromNumber:@(number)];
    std::string result = std::string([formatted UTF8String]);
    return result;
}

}
}
