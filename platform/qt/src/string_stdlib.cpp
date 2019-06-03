#include <mbgl/util/platform.hpp>

#include <QByteArray>
#include <QString>

#include <QtQml/QtQml>
#include <string>

namespace mbgl {
namespace platform {

std::string uppercase(const std::string& str) {
    auto upper = QString::fromUtf8(str.data(), str.length()).toUpper().toUtf8();

    return std::string(upper.constData(), upper.size());
}

std::string lowercase(const std::string& str) {
    auto lower = QString::fromUtf8(str.data(), str.length()).toLower().toUtf8();

    return std::string(lower.constData(), lower.size());
}

std::string formatNumber(double number,
                         const std::string& localeId,
                         const std::string& currency,
                         uint8_t minFractionDigits,
                         uint8_t maxFractionDigits)
{
    std::string formatted;
    QQmlEngine engine;
    QQmlComponent component(&engine, QUrl(QStringLiteral("qrc:/FormatNumber.qml")));
    QObject *object = component.create();

    QVariant returnedValue;
    QVariant qNumber = number;
    QVariant qLocale = QString::fromStdString(localeId);
    QVariant qCurrency = QString::fromStdString(currency);
    QVariant qMinFractionDigits = minFractionDigits;
    QVariant qMaxFractionDigits = maxFractionDigits;
    QMetaObject::invokeMethod(object, "formatNumber", Q_RETURN_ARG(QVariant, returnedValue),
                              Q_ARG(QVariant, qNumber), Q_ARG(QVariant, qLocale), Q_ARG(QVariant, qCurrency),
                              Q_ARG(QVariant, qMinFractionDigits), Q_ARG(QVariant, qMaxFractionDigits));

    formatted = returnedValue.toString().toStdString();
    delete object;
    return formatted;
}

} // namespace platform
} // namespace mbgl
