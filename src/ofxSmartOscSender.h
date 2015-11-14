//
//  ofxSmartOscSender.h
//  ofxSmartOscSender
//
//  Created by ISHII 2bit on 2015/11/12.
//
//

#ifndef ofxSmartOscSender_h
#define ofxSmartOscSender_h

#include "ofxOsc.h"
#include "ofxOscMessageStreamOperator.h"
#include <type_traits>

namespace bbb {
    template <typename Arg>
    void createMessageImpl(ofxOscMessage &m, const Arg &arg) {
        m << arg;
    }
    
    template <typename Arg, typename ... Args>
    void createMessageImpl(ofxOscMessage &m, const Arg &arg, const Args & ... args) {
        createMessageImpl(m << arg, args ...);
    }
    
    template <typename ... Args>
    ofxOscMessage createMessage(const std::string &address, const Args & ... args) {
        static_assert(sizeof...(args) != 0, "args is null at createMessage");
        
        ofxOscMessage m;
        m.setAddress(address);
        createMessageImpl(m, args ...);
        return m;
    }
    
    template <typename T>
    using get_type = typename T::type;

    template <typename T>
    using simplize_cast = get_type<std::conditional<
        std::is_same<get_type<std::remove_const<T>>, char *>::value
        || (std::is_array<T>::value && std::is_same<get_type<std::remove_extent<T>>, char>::value),
        char *,
        get_type<std::conditional<
            std::is_integral<T>::value,
            int,
            get_type<std::conditional<
                std::is_floating_point<T>::value,
                float,
                T
            >>
        >>
    >>;

    class ofxSmartOscSender : public ofxOscSender {
        bool bStrict;
        bool wrapInBundle;
    public:
        ofxSmartOscSender(bool bStrict = false, bool wrapInBundle = true)
        : bStrict(bStrict)
        , wrapInBundle(wrapInBundle) {};
        
        ofxSmartOscSender(std::string host, int port, bool bStrict = false, bool wrapInBundle = true)
        : ofxSmartOscSender(bStrict, wrapInBundle) { setup(host, port); }
        
        void setUsingStrictFormat(bool bStrict) {
            this->bStrict = bStrict;
        }
        
        bool getUsingStrictFormat() const {
            return bStrict;
        }
        
        void setSendingWithWrapInBundle(bool wrapInBundle) {
            this->wrapInBundle = wrapInBundle;
        }
        
        bool getSendingWithWrapInBundle() const {
            return wrapInBundle;
        }

        void sendMessage(ofxOscMessage &m) {
            ofxOscSender::sendMessage(m, wrapInBundle);
        }
        
        template <typename ... Args>
        inline void send(const std::string &address, const Args & ... args) {
            static_assert(sizeof...(args) != 0, "args is null at send");
            
            ofxOscMessage m = createMessage(address, args ...);
            if(bStrict) sendAsStrictFormat(address, args ...);
            else        sendAsSimpleFormat(address, args ...);
        }
        
        template <typename ... Args>
        void sendAsStrictFormat(const std::string &address, const Args & ... args) {
            static_assert(sizeof...(args) != 0, "args is null at send as Smart format");
            
            ofxOscMessage m = createMessage(address, args ...);
            ofxOscSender::sendMessage(m, wrapInBundle);
        }

        template <typename ... Args>
        void sendAsSimpleFormat(const std::string &address, const Args & ... args) {
            static_assert(sizeof...(args) != 0, "args is null at send as Smart format");
            
            ofxOscMessage m = createMessage(address, simplize_cast<Args>(args) ...);
            ofxOscSender::sendMessage(m, wrapInBundle);
        }
    };
};

using ofxSmartOscSender = bbb::ofxSmartOscSender;

#endif /* ofxSmartOscSender_h */