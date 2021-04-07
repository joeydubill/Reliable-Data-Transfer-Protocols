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
struct msg buffer[1000];
int bufferwriteindex = 0;
int bufferreadindex = 0;


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
     printf("A_Output: waiting on ACK, buffering message: %s", message.data);
     struct msg buff;
     memmove(buff.data, message.data, 20);
     buffer[bufferwriteindex] = buff;
     bufferwriteindex++;
     return;
 }
 printf("A_Output: waiting on Layer 5, sending message: %s", message.data);
 struct pkt packet;
 memmove(packet.payload, message.data, 20);
 packet.seqnum = sender_seq;
 packet.checksum = calc_checksum(&packet);
 timeout_pkt = packet;
 wait_5 = 0;
 tolayer3(0, packet);
 starttimer(0, sender_inc);

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
 //first check for checksum, state, and ack #
 
 if (wait_5 != 0){
    printf("A_Input: not waiting for ACK");
    return;
 }
 if (packet.checksum != calc_checksum(&packet)){
     printf("A_Input: checksum error");
     
     
     return;
 }
 if (packet.acknum != sender_seq){
    printf("A_Input: acknum not equal to sender_seq");
    return;
 }
 //we made it through
 printf("A_Input: receiving ACK");
 stoptimer(0);
 if (sender_seq == 0){
    sender_seq = 1;
 }else if (sender_seq == 1){
    sender_seq = 0;
 }else{
    printf("impossible");
 }
 wait_5 = 1;
 if (bufferreadindex < bufferwriteindex){
   printf("A_Input: sending buffered message: %s", buffer[bufferreadindex]);
   struct msg call = buffer[bufferreadindex];
   A_output(call);
   bufferreadindex++;
 }else if (bufferreadindex == bufferwriteindex){
   bufferreadindex = 0;
   bufferwriteindex = 0;
 }
 

}

/* called when A's timer goes off */
void A_timerinterrupt()
{
 if (wait_5 != 0){
     printf("A Timer interrupt, not waiting for ack, ignore");
    return;
 }
 starttimer(0, sender_inc);
 //timeout_pkt.acknum = 1 - timeout_pkt.acknum;
 printf("A Timer interrupt, resending timeout_packet: %s", timeout_pkt.payload);
 tolayer3(0, timeout_pkt);

}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
 wait_5 = 1;
 sender_seq = 0;
 //change??
 sender_inc = 15.0f;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
  struct pkt pack;
 //check checksum and seq
 if (packet.checksum != calc_checksum(&packet)){
     printf("B_input: checksum error");
     pack.acknum = 1 - rec_seq;
     memmove(pack.payload, packet.payload, 20);
     pack.checksum = calc_checksum(&packet);
     tolayer3(1, pack);
     return;    
 }
 if (packet.seqnum != rec_seq) {
     printf("B_input: seqnum not rec_seq");
     pack.acknum = 1 -rec_seq;
     memmove(pack.payload, packet.payload, 20);
     pack.checksum = calc_checksum(&packet);
     tolayer3(1, pack);
     return;
 }
 //send to 5 
 printf("B_input: sending ack and to layer 5: %s", packet.payload);
 pack.acknum = rec_seq;
 memmove(pack.payload, packet.payload, 20);
 pack.checksum = calc_checksum(&packet);
 tolayer3(1, pack);
 tolayer5(1, packet.payload);
 printf("rec: %s", packet.payload);
 if (rec_seq == 0){
    rec_seq = 1;
 }else if (rec_seq == 1){
    rec_seq = 0;
 }else{
    printf("impossible");
 }
 //could be packet

}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
 rec_seq = 0;
}
