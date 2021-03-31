#include <string>
#include <string.h>

#include "./testManager.h"


namespace Ballast {
  namespace Test {

    
    const std::string TestManager::getTestFailureMessage(const char* const fileName, const char* const lineNumber, const char* const reason)
    {
        std::string message;
        message.reserve(strlen(fileName) + strlen(lineNumber) + strlen(reason) + 4);

        message += fileName;
        message += "\r\n";
        message += lineNumber;
        message += "\r\n";
        message += reason;

        return message;
    }


    const std::string TestManager::getTestNotImplementedMessage(const char* const fileName, const char* const lineNumber, const char* const functionName)
    {
        const char* notImplementedMessage = "Test not implemented: ";

        std::string message;
        message.reserve(strlen(fileName) + strlen(lineNumber) + strlen(functionName) + strlen(notImplementedMessage) + 4);

        message += fileName;
        message += "\r\n";
        message += lineNumber;
        message += "\r\n";
        message += notImplementedMessage;
        message += functionName;

        return message;
    }


  }
}
