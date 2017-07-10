/* cbusassisant.c - http server that receives events from IFTTT webhooks 
 * Copyright (C) 2017, Andrew Tarabaras.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, see
 * <http://www.gnu.org/licenses/>.
 *
 * This file was adapted from simplest_web_server.c from the mongoose examples
 */

#include "mongoose.h"
#include "cbusgroups.h"

static struct mg_serve_http_opts s_http_server_opts;

static void ev_handler(struct mg_connection *nc, int ev, void *p) {
    struct http_message *hm = (struct http_message *) p;
    char *sptr, *eptr;
    char *group = NULL;
    int level;
    if (ev == MG_EV_HTTP_REQUEST) {
        //mg_printf(nc, "%s",  standard_reply);
        mg_send_response_line(nc, 200,
                              "Content-Type: text/html\r\n"
                              "Connection: close");

        if(!mg_vcmp(&hm->uri,"/cbusonoff.json")){
            if((sptr = strstr(hm->body.p, "action")) == NULL)
                return;
            sptr += 9;
            eptr = strchr(sptr, ',');
            if((group = malloc(eptr-sptr+1)) == NULL)
                return;
            strncpy(group, sptr, eptr-sptr);
            group[eptr-sptr] = '\0';
            cbusSetGroup(group);
            free(group);
        }else

        if(!mg_vcmp(&hm->uri,"/cbuslevel.json")){
            if((sptr = strstr(hm->body.p, "group")) == NULL)
                return;
            sptr += 8;
            eptr = strchr(sptr, ',');
            if((group = malloc(eptr-sptr+1)) == NULL)
                return;
            strncpy(group, sptr, eptr-sptr);
            group[eptr-sptr] = '\0';
            sptr = strstr(hm->body.p, "level");
            sptr += 8;
            level = strtol(sptr, &eptr, 10);
            cbusSetLevel(group,level);
            free(group); 
       }else{
            mg_printf(nc,
                      "\r\n<h1>C-Bus Assisant is running</h1>\r\n");
        }
        nc->flags |= MG_F_SEND_AND_CLOSE;
    }
}

int main(int argc, char *argv[])
{
    struct mg_mgr mgr;
    struct mg_connection *nc;

    cgateconnect(argc, argv);
    mg_mgr_init(&mgr, NULL);
    printf("Starting web server on port %s\n", argv[3]);
    nc = mg_bind(&mgr, argv[3], ev_handler);
    if (nc == NULL) {
        printf("Failed to create listener\n");
        return 1;
    }

    // Set up HTTP server parameters
    mg_set_protocol_http_websocket(nc);
    //s_http_server_opts.document_root = ".";
    s_http_server_opts.enable_directory_listing = "no";

    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);
    return 0;
}
