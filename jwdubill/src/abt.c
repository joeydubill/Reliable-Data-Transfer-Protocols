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

/********* STUDENTS WRITE THE NEXT SIX ROUTINES *********/


// sender info
int sender_seq;
// 0 if waiting on ACK, 1 if waiting on layer 5
int wait_5;
// incremenet value for timer
float sender_inc;

// receiver info
int rec_seq;

// packet for timeouts
struct pkt timeout_pkt;
struct msg buffered_message;
int call_buffered_packet;

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
 if (wait_5 != 1){
     buffered_message = message;
     call_buffered_packet = 1;
     return;
 }
 struct pkt packet;
 memcpy(packet.payload, message.data, 20);
 packet.seqnum = sender_seq;
 packet.checksum = calc_checksum(&packet);
 timeout_pkt = packet;
 wait_5 = 0;
 starttimer(0, sender_inc);
 tolayer3(0, packet);
 printf("send: %s", packet.payload);

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
 //first check for checksum, state, and ack #
 if (packet.checksum != calc_checksum(&packet)){
     return;
 }
 if (wait_5 != 0){
    return;
 }
 if (packet.acknum != sender_seq){
  return;
 }
 //we made it through
 stoptimer(0);
 if (sender_seq == 0){
    sender_seq = 1;
 }else if (sender_seq == 1){
    sender_seq = 0;
 }else{
    printf("impossible");
 }
 wait_5 = 1;
 if (call_buffered_packet){
     A_output(buffered_message);
 }

}

/* called when A's timer goes off */
void A_timerinterrupt()
{
 if (wait_5 != 0){
    starttimer(0, sender_inc);
    return;
 }
 tolayer3(0, timeout_pkt);
 starttimer(0, sender_inc);

}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
 wait_5 = 1;
 sender_seq = 0;
 //change??
 sender_inc = 20.0f;
 call_buffered_packet = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
  struct pkt pack;
 //check checksum and seq
 if (packet.checksum != calc_checksum(&packet)){
     pack.acknum = 1 - rec_seq;
     pack.checksum = calc_checksum(&packet);
     memcpy(pack.payload, packet.payload, 20);
     tolayer3(1, pack);
     return;    
 }
 if (packet.seqnum != rec_seq) {
     pack.acknum = 1 - rec_seq;
     pack.checksum = calc_checksum(&packet);
    memcpy(pack.payload, packet.payload, 20);
     tolayer3(1, pack);
     return;
 }
 //send to 5 
 pack.acknum = rec_seq;
 pack.checksum = calc_checksum(&packet);
 memcpy(pack.payload, packet.payload, 20);
 tolayer5(1, packet.payload);
 printf("rec: %s", packet.payload);
 if (rec_seq == 0){
    rec_seq = 1;
 }else if (rec_seq == 1){
    rec_seq = 0;
 }else{
    printf("impossible");
 }
 tolayer3(1, pack);

}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
 rec_seq = 0;
}
