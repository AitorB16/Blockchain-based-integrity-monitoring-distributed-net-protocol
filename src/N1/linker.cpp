#include "linker.hpp"

int method_updateHash(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    const char *str;
    int r;
    network *net = net->getInstance();

    r = sd_bus_message_read(m, "s", &str);

    std::string newHash = str;

    //If network not trusted, kill thread
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

    /** **BORRAR** aqui ya tendrias el hash para hacer lo que necesites con él.
     *  Asignarlo a un objeto o lo que sea. desde aquí podrias llamar a otra función pasandole el hash
     * y que esa función actualice lo que sea.
     */
    std::cout << "New Hash: " << newHash.c_str() << "\n";
    /*Send the reply even if we expect the call to be set as "No-reply"
     *This fixes some possible issues with bad configured clients */
    /* **BORRAR** Yo enviaré el mensaje con no-reply, asi que esto de abajo no se envia.
    * En mi programa lo dejo por si acaso*/
    sd_bus_reply_method_return(m, "i", 1);
    return r;
}

int dbus_init()
{
    /** Bus configuration **/
    int r;
    r = sd_bus_open_user(&bus);
    if (r < 0)
    {
        if (EXEC_MODE == DEBUG_MODE || EXEC_MODE == INTERACTIVE_MODE)
            cout << "Lnk - Failed to connect to system bus: " << strerror(-r) << endl;
        Logger("Lnk - Failed to connect to system bus");
        // fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        return r;
    }
    //Add the object using the vtable defined in .h
    r = sd_bus_add_object_vtable(bus, &slot,
                                 "/net/n1/manager", /* object path */
                                 "net.n1.manager",  /* interface name */
                                 fscheck_vtable,
                                 NULL);
    if (r < 0)
    {
        if (EXEC_MODE == DEBUG_MODE || EXEC_MODE == INTERACTIVE_MODE)
            cout << "Lnk - Failed to issue method call: " << strerror(-r) << endl;
        Logger("Lnk - Failed to issue method call");
        // fprintf(stderr, "Failed to issue method call: %s\n", strerror(-r));
        return r;
    }

    //Request a well-known name so clients can find the service
    r = sd_bus_request_name(bus, "net.n1.manager", 0);
    if (r < 0)
    {
        if (EXEC_MODE == DEBUG_MODE || EXEC_MODE == INTERACTIVE_MODE)
            cout << "Lnk - Failed to acquire service name: " << strerror(-r) << endl;
        Logger("Lnk - Failed to acquire service name");
        // fprintf(stderr, "Failed to acquire service name: %s\n", strerror(-r));
        return r;
    }

    return 0;
}

void thread_process()
{
    int r = dbus_init();

    if (EXEC_MODE == DEBUG_MODE || EXEC_MODE == INTERACTIVE_MODE)
        cout << "Lnk - Linker UP" << endl;
    Logger("Lnk - Linker UP");

    while (1)
    {
        /* Process requests */
        r = sd_bus_process(bus, NULL);
        if (r < 0)
        {
            if (EXEC_MODE == DEBUG_MODE || EXEC_MODE == INTERACTIVE_MODE)
                cout << "Lnk - Failed to process bus: " << strerror(-r) << endl;
            Logger("Lnk - Failed to process bus");
            // fprintf(stderr, "Failed to process bus: %s\n", strerror(-r));
            //goto dbus_error;
        }
        if (r > 0) /* we processed a request, try to process another one, right-away */
            continue;
        /* Wait for the next request to process */
        r = sd_bus_wait(bus, (uint64_t)-1);
        if (r < 0)
        {
            if (EXEC_MODE == DEBUG_MODE || EXEC_MODE == INTERACTIVE_MODE)
                cout << "Lnk - Failed to wait bus: " << strerror(-r) << endl;
            Logger("Lnk - Failed to wait bus");
            // fprintf(stderr, "Failed to wait on bus: %s\n", strerror(-r));
            //goto dbus_error;
        }
    }
}