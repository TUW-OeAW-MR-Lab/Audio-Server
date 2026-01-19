/*
    Logger.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Frederik Siepe/sonible GmbH
*/

#pragma once

#include <juce_core/juce_core.h>
#include <util/SupportFileLocation.h>
#include <util/ListenerInterface.h>

namespace mrlab
{

/** Custom logging class with various log levels.

    Make sure to set it up once with the static setCurrentLogger (&myLogger).
    From now on, you can call the static functions MRLab::logInfo() etc.
    from anywhere in your code without needing access to the myLogger object.

    You can also add listeners to the myLogger instance.
 */
class Logger : public juce::FileLogger
{
public:
    //==============================================================================
    enum class LogLevel
    {
        Info,
        Debug,
        Warn,
        Error,
        Fatal
    };

    //==============================================================================
    struct Listener
    {
        /** Called when a new log message is received. */
        virtual void messageLogged (const juce::String& message) = 0;
    };

    //==============================================================================
    Logger() : juce::FileLogger (APP_SUPPORT_DIR.getChildFile ("MRLabLog.txt"), "\n\n==== MRLabLog Session Start ====", maxFileSizeBytes) {}

    //==============================================================================
    /** Convenience wrapper to write an info log message. */
    static void logInfo (const juce::String& message) { log (LogLevel::Info, message); }

    /** Convenience wrapper to write a debug log message. Debug messages are ignored for non-debug builds. */
    static void logDebug (const juce::String& message) { log (LogLevel::Debug, message); }

    /** Convenience wrapper to write a warn log message. */
    static void logWarn (const juce::String& message) { log (LogLevel::Warn, message); }

    /** Convenience wrapper to write an error log message. */
    static void logError (const juce::String& message) { log (LogLevel::Error, message); }

    /** Convenience wrapper to write a fatal log message. */
    static void logFatal (const juce::String& message) { log (LogLevel::Fatal, message); }

    //==============================================================================
    /** Write a log message of a desired LogLevel. */
    static void log (LogLevel level, const juce::String& message)
    {
#if ! JUCE_DEBUG
        if (level == LogLevel::Debug)
            return;
#endif

        // clang-format off
        switch (level) {
            case LogLevel::Info:  return writeToLog (juce::String ("Info:  ") + message);
            case LogLevel::Debug: return writeToLog (juce::String ("Debug: ") + message);
            case LogLevel::Warn:  return writeToLog (juce::String ("Warn:  ") + message);
            case LogLevel::Error: return writeToLog (juce::String ("Error: ") + message);
            case LogLevel::Fatal: return writeToLog (juce::String ("Fatal: ") + message);
        }
        // clang-format on
    }

    //==============================================================================
    /** Log message to file and also notify listeners. Do not call this directly, rather use the static logInfo() etc. functions above. */
    void logMessage (const juce::String& message) override
    {
        // log to std::cout
        std::cout << message << std::endl;

        // log to file
        juce::FileLogger::logMessage (message);

        // notify listeners
        listeners.call (&Listener::messageLogged, message);

        // trim file size if needed
        trimFileSize (getLogFile(), maxFileSizeBytes);
    }

private:
    static constexpr auto maxFileSizeBytes = 10 * 1024 * 1024; // 10 MB

    //==============================================================================
    /** Prohibit using the standard logger on this class by hiding it as private. */
    static void writeToLog (const juce::String& message) { juce::Logger::writeToLog (message); }

    //==============================================================================
    MRLAB_IMPLEMENT_LISTENER_INTERFACE
};
} // namespace mrlab