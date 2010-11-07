#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <time.h>
#include "zbee.h"


void
print_general_message(void* m)
{
	general_message* msg;
	
	msg = (general_message*)m;
    printf("\n");
    switch(msg->message_id)
    {
        case MESSAGE_ID_RREQ:
            printf("*** RREQ message ***"); break;
        case MESSAGE_ID_RREP: 
            printf("*** RREP message ***"); break;
        case MESSAGE_ID_HELL:
            printf("*** HELLO message ***"); break;
        case MESSAGE_ID_DATA:
            printf("*** DATA message ***"); break;
        default: 
            printf("*** UNKNOWN MESSAGE ***");
	}
	printf("\n\tsource="); print_address(msg->source);
	printf("\n\tdestination="); print_address(msg->destination);
	printf("\n\taodv_source="); print_address(msg->aodv_source);
	printf("\n\taodv_destination="); print_address(msg->aodv_destination);
	printf("\n\thop_count=%3d\n\tsequence_number_destination=%3d", msg->hop_count, msg->sequence_number_destination); 
	printf("\n\tlifespan=%3d\n\tsequence_number_message(id)=%3d\n\n", msg->lifespan, msg->sequence_number_message);
}

void
send_general_message(HANDLE fd, 
					int message_id, 
					address source, 
					address destination, 
					int hop_count,
					address aodv_source, 
					address aodv_destination,
					int lifespan, 
					int sequence_number_message,
					int sequence_number_destination, 
					char* data)
{
	general_message gm;
	
	gm.message_id = message_id;
	copy_addresses(gm.source, source);
	copy_addresses(gm.destination, destination);
	gm.hop_count = hop_count;
	copy_addresses(gm.aodv_source, aodv_source);
	copy_addresses(gm.aodv_destination, aodv_destination);
	gm.lifespan = lifespan;
	gm.sequence_number_message = sequence_number_message;
	gm.sequence_number_destination = sequence_number_destination;
	if (data!=NULL && strlen(data)>=DATA_FIELD_LENGTH)
	{
		strncpy(gm.data, data, DATA_FIELD_LENGTH);
	}	
	else
	{
		strncpy(gm.data, "-----------------------", DATA_FIELD_LENGTH);
	}
	
	gm.tail = 0;
	
	printf("Sending message: ");
	print_general_message(&gm);
	printf("\n");
	
	send_data_to(fd, destination, (void*)&gm, sizeof(general_message));
}


void
read_one_message(HANDLE fd, char* buff)
{
	int size;
	
	buff[0] = MESSAGE_ID_INVALID;
    size = read_all(fd, buff);
	
    if (size>0)
    {
        print_general_message((char*)buff);
        switch(buff[0])
        {
            case MESSAGE_ID_RREP:        
            case MESSAGE_ID_RREQ:
            case MESSAGE_ID_HELL:
            case MESSAGE_ID_DATA:
                break;
            default:
		        printf("Message not valid.\n");	
        }
     }
}

void
use_HELLO_to_update_routing_table(routing_table_item table[], general_message* gm)
{

    printf("Updating routing table with HELLO... Using message: \n");
    print_general_message(gm); 

    routing_table_item item;
	/* 
	We receive a message that must be used to fill the table in this way:
		TABLE				Msg. RREP			
		destination	<- 		destination_aodv
		next_hop <-			source (physical source)
		number_of_hops <-	1
		initial_time <-		time(NULL)
		sequence_numb... <- sequence_number_destination
	*/

    copy_addresses(item.aodv_destination, gm->aodv_destination);
    copy_addresses(item.next_hop, gm->source);
    item.number_of_hops = 1;
    item.initial_time = time(NULL);
    item.sequence_number_destination = gm->sequence_number_destination;
  
    printf("Route added as neighbor...\n");
    put_item_at_routing_table(table, item);

}

void
use_RREP_to_update_routing_table(routing_table_item table[], general_message* gm)
{

    printf("Updating routing table with RREP... Using message: \n");
    print_general_message(gm); 

    routing_table_item item;
	/* 
	We receive a message that must be used to fill the table in this way:
		TABLE				Msg. RREP			
		destination	<- 		destination_aodv
		next_hop <-			source (physical source)
		number_of_hops <-	hop_count
		initial_time <-		time(NULL)
		sequence_numb... <- sequence_number_destination
	*/

    copy_addresses(item.aodv_destination, gm->aodv_destination);
    copy_addresses(item.next_hop, gm->source);
    item.number_of_hops = gm->hop_count;
    item.initial_time = time(NULL);
    item.sequence_number_destination = gm->sequence_number_destination;
  
    int index = get_index_item_routing_table(table, gm->aodv_destination);
    int route_changed = FALSE;

    if (index==-1)
    {
        printf("Route added (by the first time for this destination).\n");
        put_item_at_routing_table(table, item);
        route_changed = TRUE;
    }
    else if (item.sequence_number_destination > table[index].sequence_number_destination)
    {  
        printf("Checked the destination seq. number. Fresher data. OK.\n");
        put_item_at_routing_table(table, item);
        route_changed = TRUE;
    } 
    else if (item.sequence_number_destination == table[index].sequence_number_destination)
    {
        printf("Checked the destination seq. number. Same seq. number. Checking hops...\n");
        if (item.number_of_hops < table[index].number_of_hops)
        {
            printf("Less number of hops. OK.\n");
            put_item_at_routing_table(table, item);
            route_changed = TRUE;
        }
    }

    if (route_changed == FALSE)
    {
        printf("The routing table was NOT modified. ");
    }
}

int 
main()
{
    char buff[1024*8];
   
    routing_table_item table[ROUTING_TABLE_MAX_ITEMS];
    
    initialize_routing_table(table);
	
	int my_sequence_number = 0;

    /* print_routing_table(table);
    item.destination[0] = '5';
    put_item_at_routing_table(table, item); */
    
   
    /*
    
    Requestor
    I want to send a message to node X.
    Check whether we have him in our routing table or not.
      If he's not in our routing table, just send RREQ.
    If I don't receive a route in certain amount of time, I will 
    resend the message (longer lifespane and other ID). 

    Requested
    I receive a RREQ. 
        Do I know a route to the destination?
        Yes
            Send a RREP. 
        No
            Re-broadcast RREQ (until lifespan is dead). Be sure not to
            rebroadcast a message with the same sequence number. 
            
    If I receive a RREP... I have to update routes that go the destination of
    the route reply.     

    Each node has a SN that will be updated each time it sends a message. It 
    represents how fresh is the information about itself. 

    Case for 3 nodes maximun.
        
    */    

    HANDLE fd;
    char data[] = "                                  ";
    
    address my_address, destination, broadcast;
	init_address(broadcast, "FFFF");
    printf("Write local address (4 characters 0000-FFFF, for example '0001'): ");
    gets(buff);
    init_address(my_address, buff);
    init_address(destination, "FFFF");
	
	printf("Write serial port number (COM1-COM8): ");
    gets(buff);
    initialize_zigbee_module(&fd, buff, my_address);

    if (fd!=(HANDLE)-1)
    {
        printf("Connected to ZigBee module.\n");

        int exit = FALSE;

        while(exit==FALSE){
            int index;
            printf("\n\n*************\nInstructions: "
			" q (quit) p (proceed) r (read) c (change) s (send) h (show r. table)\n");
			
            char a = getch();           
            fflush(stdin);  

            switch(a)
            {
                case 'H':
                case 'h': // Show routing table.
                    print_routing_table(table);
                    break; 

                case 'R':
                case 'r': // Read. 
				{
                    read_one_message(fd, buff);
					general_message* gm = (general_message*)buff; 
                    switch(buff[0])
                    {
                        /**************************/
                        /* Message RREQ received. */
                        /**************************/
                        case MESSAGE_ID_RREQ: 
                        {
                            int message_answered = FALSE;
							printf("Message RREQ received: \n");
							print_general_message(buff);

                            /* If the destination is me, reply RREP. */
							if (addresses_are_equal(gm->aodv_destination, my_address))
                            {
								message_answered = TRUE;
								
								printf("Warning: answering a RREQ with an invalid RREP.\n");
								send_general_message(fd, MESSAGE_ID_RREP, my_address, gm->source, 
									INITIAL_HOP_COUNT, gm->aodv_source, gm->aodv_destination, DEFAULT_TTL, 
                                    gm->sequence_number_message, ++my_sequence_number, NULL);
							}
							else if (index = get_index_item_routing_table(table, gm->destination)!=-1)
                            { /* If the destination is in my link state table, reply RREP. */
                                if (table[index].number_of_hops==1) /* It's a neighbor. */
								{
									message_answered = TRUE;
									send_general_message(fd, MESSAGE_ID_RREP, my_address, gm->source, 
										INITIAL_HOP_COUNT, gm->aodv_source, gm->aodv_destination, DEFAULT_TTL, gm->sequence_number_message, 
                                        table[index].sequence_number_destination, NULL);
								}
							}
							
							if (message_answered == FALSE) /* No information about the requested node. */
							{
							/* Forward RREQ. */
	
								if (gm->lifespan > gm->hop_count) /* If it's still valid... */
								{
									send_general_message(fd, MESSAGE_ID_RREQ, my_address, broadcast, 
										gm->hop_count+1, gm->aodv_source, gm->aodv_destination, gm->lifespan, 
                                        gm->sequence_number_message, gm->sequence_number_destination, NULL);
                                    printf("RREQ forwarded.\n");
								}
								else
								{
									printf("RREQ not forwarded (hopcount=%d>=lifespan=%d).\n", gm->hop_count, gm->lifespan);
								}
							}
                            break;
                        }	

                        /**************************/
                        /* Message RREP received. */
                        /**************************/
                        case MESSAGE_ID_RREP:
						{
                            int index; 
                            use_RREP_to_update_routing_table(table, gm);
                            if (!addresses_are_equal(gm->aodv_destination, my_address))
                            {
                                if ((index=get_index_item_routing_table(table, gm->aodv_destination))!=-1)
                                {
                                    printf("RREP forwarded...\n");
                                    send_general_message(fd, MESSAGE_ID_RREP, my_address, table[index].next_hop, 
										gm->hop_count+1, gm->aodv_source, gm->aodv_destination, gm->lifespan, 
                                        gm->sequence_number_message, gm->sequence_number_destination, NULL);
                                }
                                else
                                {
                                    printf("RREQ cannot be forwarded. Error in routing.\n");
                                }
                            }
                            else
                            {
                                printf("RREP message used to update routing table. Not forwarded.\n");
                            }
                            
							break;
						}

                        /**************************/
                        /* Message HELLO received. */
                        /**************************/
                        case MESSAGE_ID_HELL:
						{
                            use_HELLO_to_update_routing_table(table, gm);
							break;
						}
    
                        /**************************/
                        /* Message DATA received. */
                        /**************************/
                        case MESSAGE_ID_DATA:
						{
                            printf("Data message received!!!\n"); 
							break;
						}

                        default:
                            break;
                    }
                    break;
				}
                case 'c': // Change data/destination.
                    printf("Please, write the new destination address (4 characters, 0000-FFFF): "); 
                    gets(buff);
                    init_address(destination, buff);
                    printf("Please, write the new data (less than 20 characters): "); 
                    gets(data);
                    break;
            
                case 's': // Send.
					printf("Sending a message...\nPlease, write the new destination address (4 characters, 0000-FFFF): ");
                    gets(buff);
                    init_address(destination, buff);
                    printf("Please, write the new data (less than 20 characters): "); gets(data);                    
					
					int route_found = FALSE;
					
                    if ((index = get_index_item_routing_table(table, destination))!=-1)
                    {
                        /* Send Message (using the next hop). */
                        printf("Destination is already in the routing table. Checking validity...\n");
						
						time_t current_time = time (NULL);
						printf("Checking time of the route: current=[%ld] initial_time+timeout=[%ld]\n");
						if (table[index].initial_time + ROUTE_TIMEOUT > current_time)
						{
							/* It's okay. We can use this route now. */
							
							route_found = TRUE;
							printf("Route found!\n");
							
						}
                    }
					
                    if (route_found == FALSE)
                    {
                        general_message* mes = (general_message*)buff;
                        while(TRUE)
                        {
                            /* Send RREQ to discover a new route. */
                            printf("Destination is NOT in the table. Sending RREQ...\n");
				            int message_sequence_number = rand();
							send_general_message(fd, MESSAGE_ID_RREQ, my_address, broadcast, 
												INITIAL_HOP_COUNT, my_address,  /* Looking for ... */ destination , 
												DEFAULT_TTL, message_sequence_number, 0,NULL);
                            /* Wait for the first RREP. The other RREP messages will 
                            be processed in other stage. */
                            printf("Waiting for the first RREP...\n"); 
                            Sleep(7000); 

                            read_one_message(fd, buff);
                            if (buff[0] == MESSAGE_ID_RREP) /* The expected message was received. */
                                break;
                        }

                        use_RREP_to_update_routing_table(table, mes);

						printf("Route available. Now sending message...\n");
						int message_sequence_number = rand();
						send_general_message(fd, MESSAGE_ID_DATA, my_address, table[index].next_hop, 
								INITIAL_HOP_COUNT, my_address, destination, DEFAULT_TTL, message_sequence_number, 0, data);
						
						/*
						packet p = new packet(destination, data);
                        send_data_to(fd, destination_next_hop, p);
						*/
                    }
                    break;

                case 'P':
                case 'p': // Proceed.
					/* 
					Leer todo.
					Si es RREQ responder con un RREP si tengo en mi tabla.
					Si es mensaje con destino a mi, mostrar.
					Si es 
					*/
                    //read_all(fd, buff);
                    //send_to(fd, destination, "12345678");
                    break;

                case 'Q':
                case 'q': // Quit. 
                    exit = TRUE;
                    break;

                default: 
                    printf("Non valid option.\n");
            }
        }
        finalize_serial(fd);
    }
    else
    {
        printf("ERROR: Cannot connect to ZigBee module.\n");
    }
    
    return 0;
}
