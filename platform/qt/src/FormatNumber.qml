import QtQuick 2.0

Item {
    function formatNumber(number, localeId, currency, minFractionDigits, maxFractionDigits) {
        if (currency !== "undefined") {
            return Number(number).toLocaleCurrencyString(Qt.locale(localeId))
        } else {
            return Number(number).toLocaleString(Qt.locale(localeId), 'f', maxFractionDigits)
        }
    }
}