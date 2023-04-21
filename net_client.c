                        fscanf(fp," %d %s %d %s %d ", &node0, &address0, &port0, &address1, &port1);
                        g_net_link[i].type = SOCKET;
                        g_net_link[i].pipe_node0 = node0;
                        g_net_link[i].socket_node1.link0 = address0;
                        g_net_link[i].socket_node1.port0 = port0;
                        g_net_link[i].socket_node1.link1 = address1;

                        // Verify everything is being read in correctly
                        //printf("socket link: %d %s %d %s %d \n", g_net_link[i].pipe_node0, g_net_link[i].socket_node1.link0, g_net_link[i].socket_node1.port0, g_net_link[i].socket_node1.link1, g_net_link[i].socket_node1.port1);
                }
                else {
                        printf("   net.c: Unidentified link type\n");
                }

        }
}

/* Display the nodes and links of the network */
printf("Nodes:\n");
for (i=0; i<g_net_node_num; i++) {
        if (g_net_node[i].type == HOST) {
                printf("   Node %d HOST\n", g_net_node[i].id);
        }
        else if (g_net_node[i].type == SWITCH) {
                printf("  Node %d SWITCH\n", g_net_node[i].id);
        }
        else {
                printf(" Unknown Type\n");
        }
}
printf("Links:\n");
for (i=0; i<g_net_link_num; i++) {
        if (g_net_link[i].type == PIPE) {
                printf("   Link (%d, %d) PIPE\n",
                                g_net_link[i].pipe_node0,
                                g_net_link[i].pipe_node1);
        }
        else if (g_net_link[i].type == SOCKET) {
                printf("   Link (%d, Switch) SOCKET\n",
                                g_net_link[i].pipe_node0);
        }
}

fclose(fp);
return(1);
}




                                                                                                                                                           620,0-1       Bot
