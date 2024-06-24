#ifndef SPINNER_OBJECT_HPP
#define SPINNER_OBJECT_HPP

#include "Callback.hpp"

namespace Spinner
{

    class Object
    {
    public:
        inline Object()
        {
            CallbackOwnerToken = CallbackOwnerToken::Create();
        }

        inline virtual ~Object()
        {
            CallbackOwnerToken.reset();
        }

    protected:
        Spinner::CallbackOwnerToken::Pointer CallbackOwnerToken;

        template<typename... Args>
        inline CallbackIdType RegisterCallback(Callback<Args...> &callback, Callback<Args...>::FunctionType function)
        {
            return callback.Register(function, CallbackOwnerToken);
        }

        template<typename... Args>
        inline void UnregisterCallback(Callback<Args...> &callback, CallbackIdType id)
        {
            callback.Unregister(id);
        }

        template<typename... Args>
        inline void SetSingleCallback(CallbackSingle<Args...> &callbackSingle, CallbackSingle<Args...>::FunctionType function)
        {
            callbackSingle.SetCallback(function, CallbackOwnerToken);
        }
    };

}

#endif //SPINNER_OBJECT_HPP
