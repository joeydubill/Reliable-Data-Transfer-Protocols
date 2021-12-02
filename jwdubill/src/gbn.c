#include "../include/simulator.h"

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

float RTT = 25.0;
int sender_seq; //A seq
int sender_ack; //A ack
int next_seq
int b_seq: // what b expects
int window_size;
int base; 

struct msg buffer[1000];
int bufferwriteindex;

int calc_checksum(struct pkt *packet){
  int sum = 0;
  sum += packet->seqnum;
  sum += packet->acknum;
  for (int i =0; i < 20; i++){
      sum += packet->payload[i];
  }
  return sum;
}

void send(){
 //send in a loop 
 while((next_seq < (base + window_size)) && (next_seq < bufferwriteindex)){
  
    struct msg message = buffer[next_seq];
    struct pkt packet;
    memcpy(packet.payload, message.data, 20);
    packet.seqnum = next_seq;
    packet.acknum = sender_ack;
    packet.checksum = calc_checksum(&packet);
  
    tolayer3(0, packet);
    if (base == next_seq){
       starttimer(0, RTT);
    }
    next_seq++;
 }
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
  //add message to queue
  struct msg buff;
  memcpy(buff.data, message.data, 20);
  buffer[bufferwriteindex] = buff;
  bufferwriteindex++;
 
 //send data
 send();
 
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
 
   //check if corrupt
   bool corrupt = false;
   if(packet.checksum == calc_checksum(&packet)){
      corrupt = false;
   }else{
      corrupt = true;
   }
 
   if(!corrupt){
      base = packet.acknum + 1;
      if (base == next_seq){
        stoptimer(0);
      }else{
        stoptimer(0);
        starttimer(0, RTT);
      }
   }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  next_seq = base;
  send();
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
 sender_seq = 0;
 sender_ack = 0
 window_size = getwinsize(); //software interface
 next_seq = 0;
 base = 0;
 bufferwriteindex = 0;


}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
   //check if corrupt
   bool corrupt = false;
   if(packet.checksum == calc_checksum(&packet)){
      corrupt = false;
   }else{
      corrupt = true;
   }
 
   if(!corrupt && (b_seq == packet.seqnum)){
      tolayer5(1, packet.payload);
      //rack = receiving ack
      struct pkt rack;
      rack.acknum = b_seq;
      rack.checksum = calc_checksum(&rack);
      
      tolayer3(1, rack);
      b_seq++;
    
   }
 
 
 
 

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
 b_seq = 0; //expected seq

}
