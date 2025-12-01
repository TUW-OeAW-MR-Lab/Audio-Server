/*
    ListenerInterface.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

//==============================================================================
/** Macro to provide boilerplate code for a nested listener class.

    To use this, add this macro at the end of your class definition and declare
    a public Listener subclass containing the listener functions.

    The macro will define a private listeners member (of class juce::ListenerList)
    and public functions to add and remove listeners to it.

    @code
    class MyClass
    {
    public:
        void someCallback()
        {
            listeners.call (&Listener::myListenerFunction, 1.0f);
        }

        struct Listener
        {
            virtual void myListenerFunction (float someValue) = 0;
        }

        MRLAB_IMPLEMENT_LISTENER_INTERFACE
    };

    // example usage:
    class MyListener : public MyClass::Listener
    {
        MyListener()
        {
            MyClass m;
            m.addListener (this);
            m.removeListener (this);
        }

        void myListenerFunction (float someValue) override { ... }
    }
    @endcode
*/
#define MRLAB_IMPLEMENT_LISTENER_INTERFACE                \
                                                          \
private:                                                  \
    juce::ListenerList<Listener> listeners;               \
                                                          \
public:                                                   \
    void addListener (Listener* l) { listeners.add (l); } \
    void removeListener (Listener* l) { listeners.remove (l); }
