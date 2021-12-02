#include "../include/simulator.h"
#include <string.h>
#include <stdio.h>

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

int ackflag;
int a_seq;
int b_seq;
int check = 0;
int seq = 0;
int last_suc = 0;
int numr = 0;
int last_seq = 0;

struct pkt buffer[1000];
int bufferwriteindex = 0;

float times[1000];
int timeswriteindex = 0;

float timeout = 0.0;
float RTT = 25.0;

struct pkt last;

int calc_checksum(struct pkt *packet){
  int sum = 0;
  sum += packet->seqnum;
  sum += packet->acknum;
  for (int i =0; i < 20; i++){
      sum += packet->payload[i];
  }
  return sum;
}



/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
    struct pkt npacket;
    npacket.seqnum = seq;
    npacket.acknum = seq;
    memcpy(npacket.payload, message.data, 20);
    npacket.checksum =  calc_checksum(&npacket);
  
    buffer[bufferwriteindex] = npacket;
    bufferwriteindex++;
     
    if (numr == 0){
      last = buffer[seq];
      tolayer3(0, last);
      times[timeswriteindex] = get_sim_time();
      timeswriteindex++;
      seq++;
      starttimer(0, RTT);
      numr++;
    }else if(numr < getwinsize()){
      last = buffer[seq];
      tolayer3(0, last);
      seq++;
      numr++;
    }

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
   ackflag = 1;
   if (packet.acknum == (last_seq + 1)){
      last_seq++;
   }else if(packet.acknum == (last_suc + getwinsize())){
      last_suc += getwinsize();
      stoptimer(0);
   }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
 
  //implemnted storing each sim time for each packet, to compare to our rtt to resend packets
   for (int i = 0; i <timeswriteindex; i++){
     timeout = get_sim_time() - times[i];
     if (timeout >= RTT){
        tolayer3(0, last);
        starttimer(0, RTT);
     }
     tolayer3(0, last);
     starttimer(0, RTT);
   }
 

}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  ackflag = 1;
  a_seq = 0;
  
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
   if ((b_seq == packet.seqnum) && (calc_checksum(&packet) == packet.checksum)){
      tolayer5(1, packet.payload);
      struct pkt ack;
      ack.acknum = b_seq;
      ack.checksum = packet.seqnum;
      tolayer3(1, ack);
      b_seq++;
   }else if ((b_seq != packet.seqnum) && (calc_checksum(&packet) == packet.checksum)){
      struct pkt ack2;
      ack2.acknum = b_seq - 1;
      ack2.checksum = packet.seqnum;
      tolayer3(1, ack2);
   }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  b_seq = 0;
}
