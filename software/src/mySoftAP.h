#if Wiring_WiFi
    #include "Particle.h"
    #include "softap_http.h"

    #ifndef mysoftap_h
    #define mysoftap_h

    struct Page
    {
        const char* url;
        const char* mime_type;
        const char* data;
    };


    void myPage(const char* url, ResponseCallback* cb, void* cbArg, Reader* body, Writer* result, void* reserved);

    #endif
#endif