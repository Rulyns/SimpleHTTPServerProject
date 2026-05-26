#include <microhttpd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t running = 1;

static void handle_signal(int signum) {
    (void)signum;
    running = 0;
}

static enum MHD_Result
answer_to_connection(void *cls, struct MHD_Connection *connection,
                     const char *url, const char *method,
                     const char *version, const char *upload_data,
                     size_t *upload_data_size, void **con_cls) {
    (void)cls;
    (void)version;
    (void)upload_data;
    (void)upload_data_size;

    static int marker;
    const char *body = "Hello, World";
    struct MHD_Response *response;
    enum MHD_Result ret;

    if (strcmp(method, "GET") != 0) {
        return MHD_NO;
    }

    if (strcmp(url, "/") != 0) {
        return MHD_NO;
    }

    if (*con_cls == NULL) {
        *con_cls = &marker;
        return MHD_YES;
    }

    response = MHD_create_response_from_buffer(
        strlen(body), (void *)(uintptr_t)body, MHD_RESPMEM_PERSISTENT);
    if (response == NULL) {
        return MHD_NO;
    }

    MHD_add_response_header(response, "Content-Type", "text/plain; charset=utf-8");

    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    *con_cls = NULL;

    return ret;
}

int main(void) {
    struct MHD_Daemon *daemon;
    const uint16_t port = 8080;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    daemon = MHD_start_daemon(
        MHD_USE_INTERNAL_POLLING_THREAD,
        port,
        NULL,
        NULL,
        &answer_to_connection,
        NULL,
        MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int)0,
        MHD_OPTION_END);

    if (daemon == NULL) {
        return 1;
    }

    while (running) {
        pause();
    }

    MHD_stop_daemon(daemon);
    return 0;
}
