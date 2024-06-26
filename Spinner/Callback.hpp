#ifndef SPINNER_CALLBACK_HPP
#define SPINNER_CALLBACK_HPP

#include <functional>
#include <map>
#include <memory>
#include <iostream>
#include <atomic>

namespace Spinner
{

    using CallbackIdType = size_t;

    struct CallbackOwnerToken
    {
        using Pointer = std::shared_ptr<CallbackOwnerToken>;
        using WeakPointer = std::weak_ptr<CallbackOwnerToken>;

        inline static Pointer Create()
        {
            return std::make_shared<CallbackOwnerToken>();
        }
    };

    template<typename... Args>
    class Callback
    {
    public:
        using FunctionType = std::function<void(Args...)>;

        struct OwnedCallback
        {
            FunctionType Callback;
            CallbackOwnerToken::WeakPointer Token;
            bool Unowned;
        };

    protected:
        std::atomic<CallbackIdType> IdCounter = 0;
        std::map<CallbackIdType, OwnedCallback> Callbacks{};
        std::atomic<bool> CancelToken = false;

    protected:
        inline void Prune()
        {
            auto iter = Callbacks.begin();
            auto iterEnd = Callbacks.end();
            for (; iter != iterEnd;)
            {
                if (!iter->second.Unowned && iter->second.Token.expired())
                {
                    iter = Callbacks.erase(iter);
                }
                else
                {
                    ++iter;
                }
            }
        }

    public:
        inline CallbackIdType Register(FunctionType callback, const CallbackOwnerToken::WeakPointer &ownerToken)
        {
            if (callback == nullptr)
            {
                throw std::runtime_error("Cannot register a null callback");
            }

            CallbackIdType id = ++IdCounter;
            bool unowned = ownerToken.expired();
            Callbacks.emplace(id, OwnedCallback{callback, ownerToken, unowned});
            return id;
        }

        inline void Unregister(CallbackIdType id)
        {
            Callbacks.erase(id);
        }

        inline void ClearCallbacks()
        {
            Callbacks.clear();
        }

        inline void Run(Args... args)
        {
            CancelToken = false;

            Prune();

            for (auto &callback : Callbacks)
            {
                try
                {
                    if (CancelToken)
                    {
                        break;
                    }

                    callback.second.Callback(args...);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Callback failed: " << e.what() << '\n';
                }
            }
        }

        inline void Cancel()
        {
            CancelToken = true;
        }
    };

    template<typename... Args>
    class CallbackSingle
    {
    public:
        using FunctionType = std::function<void(Args...)>;
        using OwnedCallback = Callback<Args...>::OwnedCallback;

    protected:
        OwnedCallback Callback;

    public:
        inline void SetCallback(FunctionType callback, const CallbackOwnerToken::WeakPointer &ownerToken)
        {
            bool unowned = ownerToken.expired();
            Callback = OwnedCallback{callback, ownerToken, unowned};
        }

        inline void ClearCallback()
        {
            Callback.Callback = nullptr;
            Callback.Token = nullptr;
        }

        inline void Run(Args... args)
        {
            if (Callback.Callback == nullptr || (!Callback.Unowned && Callback.Token.expired()))
            {
                // if either are null then clear the token, just in case
                Callback.Token.reset();
                return;
            }

            try
            {
                Callback.Callback(args...);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Callback failed: " << e.what() << '\n';
            }
        }
    };

} // Spinner

#endif //SPINNER_CALLBACK_HPP
