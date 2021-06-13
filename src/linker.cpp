#include "linker.hpp"

int method_updateHash(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    const char *str;
    int r;
    network *net = net->getInstance();

    r = sd_bus_message_read(m, "s", &str);

    std::string newHash = str;

    /* If network not trusted, kill thread */
    if (net->isNetworkComprometed())
    {
        if (EXEC_MODE == DEBUG_MODE)
            cout << "Lnk - STOPPING LINKER, NETWORK COMPROMETED" << endl;
        Logger("Lnk - STOPPING LINKER, NETWORK COMPROMETED");
        pthread_exit(NULL);
    }

    if (net->getSelfNode()->getLastHash() != newHash)
    {
        net->getSelfNode()->updateHashList(newHash);
    }

    std::cout << "New Hash: " << newHash.c_str() << "\n";

    sd_bus_reply_method_return(m, "i", 1);
    return r;
}

int dbus_init()
{
    string p1 = "/net/linker/manager";
    string p2 = "net.linker.manager";
    
    if (ID != -1)
    {
        p1 = "/net/n" + to_string(ID) + "/manager";
        p2 = "net.n" + to_string(ID) + ".manager";
    }

    /** Bus configuration **/
    int r;
    r = sd_bus_open_user(&bus);
    if (r < 0)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cout << "Lnk - Failed to connect to system bus: " << strerror(-r) << endl;
        Logger("Lnk - Failed to connect to system bus");
        return r;
    }
    /* Add the object using the vtable defined in .h */
    r = sd_bus_add_object_vtable(bus, &slot,
                                 p1.c_str(), /* object path */
                                 p2.c_str(), /* interface name */
                                 linker_vtable,
                                 NULL);
    if (r < 0)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cout << "Lnk - Failed to issue method call: " << strerror(-r) << endl;
        Logger("Lnk - Failed to issue method call");
        return r;
    }

    /* Request a well-known name so clients can find the service */
    r = sd_bus_request_name(bus, p2.c_str(), 0);
    if (r < 0)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cout << "Lnk - Failed to acquire service name: " << strerror(-r) << endl;
        Logger("Lnk - Failed to acquire service name");
        return r;
    }

    return 0;
}

void linker_process()
{
    int r = dbus_init();
    cout << "Lnk - Linker UP" << endl;
    Logger("Lnk - Linker UP");

    while (1)
    {
        /* Process requests */
        r = sd_bus_process(bus, NULL);
        if (r < 0)
        {
            if (EXEC_MODE == DEBUG_MODE)
                cout << "Lnk - Failed to process bus: " << strerror(-r) << endl;
            Logger("Lnk - Failed to process bus");
        }
        if (r > 0) /* A request has been processed, try to process another one, right-away */
            continue;
        /* Wait for the next request to process */
        r = sd_bus_wait(bus, (uint64_t)-1);
        if (r < 0)
        {
            if (EXEC_MODE == DEBUG_MODE)
                cout << "Lnk - Failed to wait bus: " << strerror(-r) << endl;
            Logger("Lnk - Failed to wait bus");

        }
    }
}