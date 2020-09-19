#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QNetworkReply>

class ErrorHandler
{
public:
    enum ErrorType {
        NoError,
        ConnectionFailureError,
        NoConnectionError,
        AuthenticationError,
        IgnoredError,
    };

    static ErrorType toErrorType(QNetworkReply::NetworkError error);
};

#endif // ERRORHANDLER_H
