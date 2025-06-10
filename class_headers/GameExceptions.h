#ifndef GAME_EXCEPTIONS_H
#define GAME_EXCEPTIONS_H

#include <stdexcept>
#include <string>

// game exceptions
class GameError : public std::runtime_error {
public:
    explicit GameError(const std::string& message)
        : std::runtime_error("Game Error: " + message) {}
};

class ResourceLoadError : public GameError {
public:
    explicit ResourceLoadError(const std::string& resourceType, const std::string& resourcePath, const std::string& details = "")
        : GameError("Failed to load " + resourceType + " from path: '" + resourcePath + "'" + (details.empty() ? "" : " - Details: " + details)) {}
};

class ConfigurationError : public GameError {
public:
    explicit ConfigurationError(const std::string& configDetails)
        : GameError("Configuration Error: " + configDetails) {}
};

class GameLogicError : public GameError {
public:
    explicit GameLogicError(const std::string& logicDetails)
        : GameError("Game Logic Error: " + logicDetails) {}
};

class InvalidStateError : public GameLogicError {
public:
    explicit InvalidStateError(const std::string& stateDetails)
        : GameLogicError("Invalid State: " + stateDetails) {}
};

#endif // GAME_EXCEPTIONS_H